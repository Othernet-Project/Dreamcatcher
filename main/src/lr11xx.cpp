#include "SPI.h"
#include "lr11xx.h"
#include "ft8/ft8.h"
#include "ft8/ft8_constants.h"
#include "customize.h"
#include "settings.h"
#include "esp_task_wdt.h"
#include "libb64/cdecode.h"
#include "libb64/cencode.h"
#include "carousel.h"
#include "driver/gpio.h"
#include "wifi.h"

carousel data_carousel;				// File downloader for this stream
portMUX_TYPE sxMux;
extern bool sdCardPresent;
unsigned int filepacket, filepackets;
int tx_status = 0;            // TX Status: 0 = Ready, 1 = TX Queued, 2 = TX active
char filename[260] = "";
AsyncUDP udp;
int32_t offset;

// lr11xx struct
typedef struct lr11xx_context_t {
    SPIClass* spi;
    uint8_t nreset;
    uint8_t busy;
    uint8_t dio1;
    uint8_t nss;
};

lr11xx_context_t lrRadio;
lr11xx_system_version_t version;
SPIClass* spi1110;

uint32_t Frequency;
uint32_t txFrequency;
int32_t _Offset = 0;                        //offset frequency for calibration purposes  
uint8_t Bandwidth;          //LoRa bandwidth
uint8_t SpreadingFactor;        //LoRa spreading factor
uint8_t CodeRate;            //LoRa coding rate

uint32_t packetsRX = 0;
uint32_t IRQStatus;

uint8_t RXPacketL;                               //stores length of packet received
int8_t  PacketRSSI;                              //stores RSSI of received packet
int8_t  PacketSNR;                               //stores signal to noise ratio (SNR) of received packet
extern xTaskHandle rxTaskHandle;
static uint16_t crc, header;
bool isFormatting;

//    bitrate variables
static uint8_t packetsCount;
static uint8_t lastPacket;
static uint8_t packets[100] = {0};
static uint8_t y = 0;

//pktrate vars
static uint8_t packetsInInterval;

bool loraReady;                           // variable to display LoRa fault with led or on website

struct midi_data {
    uint8_t note;
    float   start;
    uint8_t velocity;
    float   duration;
};

struct midi_data midiarray[32];
uint8_t firstnote = 0;
char* txtarray;

static SemaphoreHandle_t blink_rx;
static SemaphoreHandle_t tx_ft8;

struct ft8_data ft8Data;
struct cw_data cwData;

uint32_t midiNoteToFreq(uint8_t note){
    return pow(2, ((note - 69) / 12)) * 440;
}

void init_gpio(void) {
  gpio_reset_pin((gpio_num_t)LED_PIN);
  gpio_set_direction((gpio_num_t)LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)LED_PIN, 0);

  //pinMode(BUZ_PIN, OUTPUT);

  /*
  ledcSetup(0, 5000, 8);
  ledcAttachPin(BUZ_PIN, 0);

  
  ledcWriteNote(0, NOTE_C, 4);
  delay(500);
  ledcWrite(0, 127);
  delay(500);
  ledcWrite(0, 64);
  delay(500);
  ledcWrite(0, 32);
  delay(500);
  ledcWrite(0, 16);
  delay(500);
  ledcWrite(0, 8);

  ledcWriteNote(0, NOTE_D, 4);
  delay(500);
  
  ledcWriteNote(0, NOTE_E, 4);
  delay(500);

  ledcWriteNote(0, NOTE_F, 4);
  delay(500);
  ledcWriteNote(0, NOTE_G, 4);
  delay(500);
  ledcWriteNote(0, NOTE_A, 4);
  delay(500);
  ledcWriteNote(0, NOTE_B, 4);
  delay(500);
  
  ledcDetachPin(BUZ_PIN);*/

};

// send data out over LR1120
void lrSendData()
{
  Serial.println("LR1120 doing a test TX!");

  uint8_t msgData[224];
  msgData[0] = 0x69;
  memcpy(msgData+4, (uint8_t*)"A test Message from the TX DC2206", 34);  

  lr11xx_regmem_write_buffer8(&lrRadio, msgData, 224);
  lr11xx_radio_set_tx(&lrRadio, 0);
}

void blinky(void *pvParameter)
{
  while(1)
    {
        if( xSemaphoreTake( blink_rx, portMAX_DELAY ) == pdTRUE )
        {
          gpio_set_level((gpio_num_t)LED_PIN, 1);
          gpio_set_level((gpio_num_t)BUZ_PIN, 1);
          vTaskDelay(10 / portTICK_RATE_MS); // sleep 100ms
          gpio_set_level((gpio_num_t)LED_PIN, 0);
          gpio_set_level((gpio_num_t)BUZ_PIN, 0);
        }
    }
}

// transmits a CW tone
void txCW()
{
  Serial.println("txCW function");
  Serial.println(txFrequency);
  if (cwData.enabled && cwData.type == 2)
  {
    Serial.println("txCW function: ON");
    lr11xx_radio_set_rf_freq(&lrRadio, txFrequency);
    lr11xx_radio_set_tx_cw(&lrRadio);               // Enable TX in CW mode
  } else if (!cwData.enabled && cwData.type == 2) {
    Serial.println("txCW function: OFF");
    lr11xx_radio_set_rx(&lrRadio, 0);
    tx_status = 0;
  }

  // do a dit
  if (cwData.enabled && cwData.type == 0)
  {
    Serial.println("txCW function: DIT");
    lr11xx_radio_set_rf_freq(&lrRadio, txFrequency);
    lr11xx_radio_set_tx_cw(&lrRadio);
    vTaskDelay(100 / portTICK_RATE_MS); // sleep 100ms
    lr11xx_radio_set_rx(&lrRadio, 0);
    tx_status = 0;
  }

  // do a dah
  if (cwData.enabled && cwData.type == 1)
  {
    Serial.println("txCW function: DAH");
    lr11xx_radio_set_rf_freq(&lrRadio, txFrequency);
    lr11xx_radio_set_tx_cw(&lrRadio);
    vTaskDelay(300 / portTICK_RATE_MS); // sleep 100ms
    lr11xx_radio_set_rx(&lrRadio, 0);
    tx_status = 0;
  }
  
}

void txData(void *pvParameter)
{
  while(1)
    {
        if( xSemaphoreTake( tx_ft8, portMAX_DELAY ) == pdTRUE )
        {
          // if FT8 data is avaliable for TX, start it
          if (ft8Data.readyToTx && tx_status == 0)
          {
            Serial.println("FT8 Data ready to TX");
            tx_status = 1;
            txFT8(ft8Data.message, ft8Data.isFreeMessage, ft8Data.useOddSlot);
            ft8Data.readyToTx = false;
          }
          // CW should be enabled
          Serial.println(tx_status);
          Serial.println(cwData.enabled);
          if ((cwData.enabled && tx_status == 0) || (cwData.enabled == false && tx_status == 2))
          //if(tx_status == 0)
          {
            Serial.println("CW TX Toggle");
            tx_status = 2;
            txCW();
          }
              
        }
    }
}

/**
 * Helper function to feed website with stats
 */
extern "C" void getStats(uint16_t* _crc, uint16_t* _header)
{
  *_crc = crc;
  *_header = header;
}

extern "C" void getMidi(char* _txtarray)
{
  const char* txt_pre = "{\"type\":\"midi\",\"data\":[";
  const char* txt_suf = "]}";
  const char* txt_template = "[%d,%f,%d,%f],";
  int strlength = 0;
  strlength += sprintf(_txtarray+strlength, txt_pre);
  for (midi_data x : midiarray)
  {
    strlength += sprintf(_txtarray+strlength, txt_template, x.note, x.start, x.velocity, x.duration);
  }
  strlength += sprintf(_txtarray+strlength-1, txt_suf);
}

/**
 * Helper function to feed website with stats
 */
extern "C" void getPacketStats(int8_t* rssi, int8_t* snr)
{
  *rssi = PacketRSSI;
  *snr = PacketSNR;
}

class mycallback : public carousel::callback {
  void fileComplete( const std::string &path ) {
    Serial.println("--- File Complete ---");
    Serial.printf("new file path: %s\n", path.c_str());
    strcpy(filename, path.c_str());

    char *newLogEntry = (char*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
    sprintf(newLogEntry,"File Complete:  %s", path.c_str());
    logToFile(newLogEntry);
  }
	void processFile(unsigned int index, unsigned int count) {
    Serial.printf("file progress: %d of %d bytes\n", index, count);
    filepacket = index;
    filepackets = count;

    char *newLogEntry = (char*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
    sprintf(newLogEntry,"file progress: %d of %d bytes", index, count);
    logToFile(newLogEntry);    
  }
};

/**
 * ISR function from lr11xx
 */
IRAM_ATTR void rx1110ISR()
{
  //Serial.println("DIO1");
  xTaskNotify(rxTaskHandle, 0x0, eSetBits);
}

IRAM_ATTR void busyIRQ()
{
  //Serial.print("irq on busy, level: ");
  //Serial.println(digitalRead(RFBUSY));
}

// set Frequency including RF Switches
void setFrequency(uint32_t freq)
{
  Serial.println("Setting Freq");
  lr11xx_radio_set_rf_freq(&lrRadio, freq);

  // set RF Switches to subGHZ SMA if frequency is under 1.9ghz
  if (freq < 1900000000)
  {
    Serial.println("Setting Freq: RF Switch to sub-GHz Port");
    gpio_set_level((gpio_num_t)RF_SW_SUBG1, 0);
    gpio_set_level((gpio_num_t)RF_SW_SUBG2, 1);
  } else {
    Serial.println("Setting Freq: RF Switch to 2.4GHz Port");
    gpio_set_level((gpio_num_t)RF_SW_SUBG1, 1);
    gpio_set_level((gpio_num_t)RF_SW_SUBG2, 0);
  }
  
}

//async trigger tx to not block webserver TODO: make it universal useable with different TX Protocls
extern "C" void queueTX(const char *message, bool isFreeMessage = false, bool useOddSlot = false)
{
  Serial.println("queueTX started");
  xSemaphoreGive(tx_ft8);
}
 


// transmits FT8 over the LR11xx using CW mode and the FT8_lib
extern "C" void txFT8(const char *message, bool isFreeMessage = false, bool useOddSlot = false)
{
  uint16_t tone_spacing = 6;       // Tone Spacing is 6.25 Hz, but SX Chips only support full Hz, so we use 6
  uint16_t tone_delay = 159;       // Tone Delay in ms

  uint8_t* tones = getFT8SymbolsFromText(message, isFreeMessage);

  // get time to find next FT8 window to TX
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  Serial.printf("Current Devicetime: %02d:%02d:%02d - %02d.%02d.%d \n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900);

  bool waitingForTimeSlot = true;

  //while (timeinfo.tm_sec % 30 != 00)
  while (waitingForTimeSlot)
  {
    Serial.print("waiting for TX window, secs: ");
    Serial.println(timeinfo.tm_sec);
    if (!useOddSlot)
    {
      if (timeinfo.tm_sec % 30 == 0) {
        waitingForTimeSlot = false;
      }
    } else {
      if (timeinfo.tm_sec == 15 || timeinfo.tm_sec == 45) {
        waitingForTimeSlot = false;
      }
    }
    
    vTaskDelay((500) / portTICK_PERIOD_MS);
    time(&now);
    localtime_r(&now, &timeinfo);
  }

  tx_status = 2;

  Serial.printf("TX Devicetime: %02d:%02d:%02d - %02d.%02d.%d \n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900);

  setFrequency(txFrequency);
  lr11xx_radio_set_tx_cw(&lrRadio);               // Enable TX in CW mode

  uint8_t i;
  for (i = 0; i < FT8_NN; i++)
  {
    lr11xx_radio_set_rf_freq(&lrRadio, (txFrequency) + (tones[i] * tone_spacing));
    vTaskDelay(tone_delay / portTICK_PERIOD_MS);   //just use delay, not perfect timing but good enough for now
  }

  // Set Radio back to normal Frequency and stop TX by setting it to RX
  setFrequency(Frequency);
  lr11xx_radio_set_rx(&lrRadio, 0);
  tx_status = 0;
}

/**
 * Init lr11xx with default params, order is important:
 * 1) SetPacketType to LORA
 * 2) SetModulationParams
 * 3) SetPacketParams
 * 3a) Set radio frequency - optional here
 * 4) SetPAConfig
 * 5) SetTxParams
 * 6) Set dio1 irq
 */
void initLR11xx()
{
  char *newLogEntry = (char*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
  sprintf(newLogEntry,"initialization of Radio - LR11xx, Uptime: %lus", millis()/1000);
  logToFile(newLogEntry);

  init_gpio();

  lr11xx_system_stat1_t lrStat1;
  lr11xx_system_stat2_t lrStat2;
  lr11xx_system_irq_mask_t lrIrq_status; 

  pinMode(DIO1, INPUT);
  pinMode(RFBUSY, INPUT);
  pinMode(NRESET, OUTPUT);

  memset(&lrRadio, 0, sizeof(lr11xx_context_t));
  lrRadio.busy = RFBUSY;
  lrRadio.dio1 = DIO1;
  lrRadio.nreset = NRESET;
  lrRadio.nss = LORA_NSS;

  //attachInterrupt(DIO1, rx1110ISR, RISING);

  Serial.println("Init SPI for LR11xx");
  spi1110 = new SPIClass(HSPI);
  spi1110->begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  lrRadio.spi = spi1110;

  pinMode(LORA_NSS, OUTPUT);
  digitalWrite(LORA_NSS, HIGH);

  //------------------------------------------------------

  Serial.println("Reset and reboot LR11xx");

  lr11xx_system_reset(&lrRadio);
  delay(220);
  lr11xx_system_reboot(&lrRadio, false);

  delay(100);
  
  Serial.print("Set regs LR11xx");
  lr11xx_system_set_reg_mode(&lrRadio, LR11XX_SYSTEM_REG_MODE_DCDC);

  lr11xx_system_get_status(&lrRadio, &lrStat1, &lrStat2, &lrIrq_status);
  Serial.print("DCDC Stat1: ");
  Serial.println(lrStat1.command_status);

  // SET RF Switches
  const lr11xx_system_rfswitch_cfg_t rfsw_cfg = {
    .enable = LR11XX_SYSTEM_RFSW0_HIGH,
    .standby = LR11XX_SYSTEM_RFSW0_HIGH,
    .rx = LR11XX_SYSTEM_RFSW0_HIGH,
    .tx = LR11XX_SYSTEM_RFSW0_HIGH,
    .tx_hp = LR11XX_SYSTEM_RFSW0_HIGH,
  };
  lr11xx_system_set_dio_as_rf_switch(&lrRadio, &rfsw_cfg);

  //lr11xx_system_set_tcxo_mode(&lrRadio, lr11xx_SYSTEM_TCXO_CTRL_3_0V, 0x70);
  lr11xx_system_set_tcxo_mode(&lrRadio, LR11XX_SYSTEM_TCXO_CTRL_1_8V, 0x70);
  
  lr11xx_system_get_status(&lrRadio, &lrStat1, &lrStat2, &lrIrq_status);
  Serial.print("TCXO Stat1: ");
  Serial.println(lrStat1.command_status);

  delay(100);
  lr11xx_system_cfg_lfclk(&lrRadio, LR11XX_SYSTEM_LFCLK_RC, true);
  lr11xx_system_clear_errors(&lrRadio);

  Serial.println("system calibrate");
  lr11xx_system_calibrate(&lrRadio, 0x3F);

  lr11xx_system_get_status(&lrRadio, &lrStat1, &lrStat2, &lrIrq_status);
  Serial.print("calibrate Stat1: ");
  Serial.println(lrStat1.command_status);
  //lr11xx_system_calibrate(&lrRadio, lr11xx_SYSTEM_CALIB_ADC_MASK);
  lr11xx_system_clear_irq_status(&lrRadio, LR11XX_SYSTEM_IRQ_ALL_MASK | 0x14 | 0x15);

  // SetPacketType (1)
  lr11xx_radio_set_pkt_type(&lrRadio, LR11XX_RADIO_PKT_TYPE_LORA);

  // SetModulationParams (2)
  lr11xx_radio_mod_params_lora_t mod_params;
  mod_params.sf = (lr11xx_radio_lora_sf_t)SpreadingFactor;
  mod_params.bw = (lr11xx_radio_lora_bw_t)Bandwidth;
  mod_params.cr = (lr11xx_radio_lora_cr_t)CodeRate;

  //mod_params.sf = (lr11xx_radio_lora_sf_t)LR11XX_RADIO_LORA_SF9;
  //mod_params.bw = (lr11xx_radio_lora_bw_t)LR11XX_RADIO_LORA_BW_200;
  //mod_params.cr = (lr11xx_radio_lora_cr_t)LR11XX_RADIO_LORA_CR_4_5;

  lr11xx_radio_set_lora_mod_params(&lrRadio, &mod_params);

  // SetPacketParams (3)
  const lr11xx_radio_pkt_params_lora_t pkt_params = {
      .preamble_len_in_symb = 0x23,                  //!< LoRa Preamble length [symbols]
      .header_type = LR11XX_RADIO_LORA_PKT_EXPLICIT, //!< LoRa Header type configuration
      .pld_len_in_bytes = 255,                        //!< LoRa Payload length [bytes]
      .crc = LR11XX_RADIO_LORA_CRC_ON,               //!< LoRa CRC configuration
      .iq = LR11XX_RADIO_LORA_IQ_STANDARD,           //!< LoRa IQ configuration
  };
  lr11xx_radio_set_lora_pkt_params(&lrRadio, &pkt_params);

  // Set radio freq (3a)
  //lr11xx_radio_set_rf_freq(&lrRadio, Frequency);
  setFrequency(Frequency);

  // SetPAConfig (4)
  const lr11xx_radio_pa_cfg_t pa_cfg = {
      .pa_sel = LR11XX_RADIO_PA_SEL_LP,                 //!< Power Amplifier selection
      .pa_reg_supply = LR11XX_RADIO_PA_REG_SUPPLY_VBAT, //!< Power Amplifier regulator
      .pa_duty_cycle = 0x04,                            //!< Power Amplifier duty cycle (Default 0x04)
      .pa_hp_sel = 0x07                                 //!< Number of slices for HPA (Default 0x07)
  };
  lr11xx_radio_set_pa_cfg(&lrRadio, &pa_cfg);

  // SetTxParams (5)
  lr11xx_radio_set_tx_params(&lrRadio, -10, LR11XX_RADIO_RAMP_48_US);

  // Set dio1 irq(6)
  lr11xx_system_set_dio_irq_params(&lrRadio, LR11XX_SYSTEM_IRQ_ALL_MASK | 0x14 | 0x15, 0);
  lr11xx_system_clear_irq_status(&lrRadio, LR11XX_SYSTEM_IRQ_ALL_MASK | 0x14 | 0x15);
  delay(100);

  attachInterrupt(DIO1, rx1110ISR, RISING);
  attachInterrupt(RFBUSY, busyIRQ, RISING);

  lr11xx_radio_set_rx(&lrRadio, 0); //start Receiving
  //lr11xx_radio_set_tx_infinite_preamble(&lrRadio);
  //Serial.println("TX enabled for testing");
  //lr11xx_radio_set_tx_cw(&lrRadio);

  loraReady = true;

  lr11xx_system_errors_t lrErrors;
  lr11xx_system_get_errors( &lrRadio, &lrErrors );
  Serial.print("lr11xx Errors: ");
  Serial.println(lrErrors);

  blink_rx = xSemaphoreCreateBinary();
  xTaskCreate(&blinky, "blinky", 1024,NULL,5,NULL);

  tx_ft8 = xSemaphoreCreateBinary();
  xTaskCreate(&txData, "txdata", 8 * 1024,NULL,5,NULL);
}

/**
 * Get lr11xx version info to confirm lr11xx is working
 */
void getLR11xxInfo()
{
  lr11xx_system_get_version(&lrRadio, &version);
  Serial.printf("HW: 0x%02x, FW: 0x%04x\n", version.hw, version.fw);
  Serial.printf("Mode: 0x%02x\n", version.type);
  Serial.printf("\n");
}

extern "C" void updateLoraSettings(uint32_t freq, uint8_t bw, uint8_t sf, uint8_t cr)
{
  char *newLogEntry = (char*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
  sprintf(newLogEntry,"Updating Radio Settings and restarting it, Uptime: %lus", millis()/1000);
  logToFile(newLogEntry);
  
  Frequency = freq;
  SpreadingFactor = sf;
  Bandwidth = bw;
  CodeRate = cr;
  lr11xx_radio_mod_params_lora_t mod_params;
  mod_params.sf = (lr11xx_radio_lora_sf_t)SpreadingFactor;
  mod_params.bw = (lr11xx_radio_lora_bw_t)Bandwidth;
  mod_params.cr = (lr11xx_radio_lora_cr_t)CodeRate;

  unsigned long startMillis = micros();
  lr11xx_radio_set_lora_mod_params(&lrRadio, &mod_params);
  setFrequency(Frequency);          //set Frequency with RF switches in mind
  lr11xx_radio_set_rx(&lrRadio, 0); //start Receiving

  storeLoraSettings();
}

/**
 * Read data from lr11xx
 */
uint8_t readbufferlr11xx(uint8_t *rxbuffer, uint8_t size)
{
  uint8_t RXstart;
  uint8_t RXPacketL = 0;
  uint32_t regdata;

  /*
  lr11xx_system_get_and_clear_irq_status(&lrRadio, &regdata);

  if ( (regdata & lr11XX_SYSTEM_IRQ_HEADER_ERROR) | (regdata & lr11XX_SYSTEM_IRQ_CRC_ERROR) ) //check if any of the preceding IRQs is set
  {
    return 0;
  }
  */

  //get lor header info
  lr11xx_radio_lora_cr_t pktCrInfo;
  bool CRCInfo;
  lr11xx_radio_get_lora_rx_info(&lrRadio, &CRCInfo, &pktCrInfo);
  //Serial.printf("Pkt header info - crc: %02X , codingrate: %02X \n", CRCInfo, pktCrInfo);

  //get lora stats
  lr11xx_radio_stats_lora_t loraStats;
  lr11xx_radio_get_lora_stats(&lrRadio, &loraStats);
  //Serial.printf("Lorastats: RX %i, CRC %i, HEADER %i \n", loraStats.nb_pkt_received, loraStats.nb_pkt_crc_error, loraStats.nb_pkt_header_error);

  // get rx buffer size to read
  lr11xx_radio_rx_buffer_status_t bufferStatus;
  lr11xx_radio_get_rx_buffer_status(&lrRadio, &bufferStatus);
  
  //Serial.printf("RX Buffer (len: %i, offset: %i): \n", bufferStatus.pld_len_in_bytes, bufferStatus.buffer_start_pointer);

  RXPacketL = bufferStatus.pld_len_in_bytes;
  RXstart = bufferStatus.buffer_start_pointer;

  //uint8_t buffer[256];
  // read rxbuffer over SPI afap
  lr11xx_regmem_read_buffer8(&lrRadio, rxbuffer, RXstart, RXPacketL);
  
  //Serial.println((char*)rxbuffer);

  return RXPacketL;
}

/**
 * Read packet from lr11xx, when IRQ is triggered
 */
void rxTaskLR11xx(void* p)
{
  Serial.print("RXTask on Core: ");
  Serial.println (xPortGetCoreID());
  static uint32_t mask = 0;
  data_carousel.init("/files/TMP", new mycallback());
  float lastMidiStart = 0;
  while(1) {
    if (xTaskNotifyWait(0, 0, &mask, portMAX_DELAY))
    {
      //Serial.println("rxTask triggered");

      lr11xx_system_get_and_clear_irq_status(&lrRadio, &IRQStatus);

      if(IRQStatus == 0x0) continue;
      if(IRQStatus & LR11XX_SYSTEM_IRQ_TX_DONE){
        Serial.println("TX was done on LR!");
        gpio_set_level((gpio_num_t)LED_PIN, 1);
        vTaskDelay(50 / portTICK_RATE_MS); // sleep 100ms
        gpio_set_level((gpio_num_t)LED_PIN, 0);
        vTaskDelay(100 / portTICK_RATE_MS); // sleep 100ms
        gpio_set_level((gpio_num_t)LED_PIN, 1);
        vTaskDelay(50 / portTICK_RATE_MS); // sleep 100ms
        gpio_set_level((gpio_num_t)LED_PIN, 0);
        vTaskDelay(100 / portTICK_RATE_MS); // sleep 100ms
        gpio_set_level((gpio_num_t)LED_PIN, 1);
        vTaskDelay(50 / portTICK_RATE_MS); // sleep 100ms
        gpio_set_level((gpio_num_t)LED_PIN, 0);
      }
      if(IRQStatus & LR11XX_SYSTEM_IRQ_RX_DONE) {
        //Serial.println("GOT A PACKET WOOHOO!");

        lr11xx_radio_pkt_status_lora_t pkt_status;        
        lr11xx_radio_get_lora_pkt_status( &lrRadio, &pkt_status );        

        PacketRSSI = pkt_status.rssi_pkt_in_dbm;              //read the recived RSSI value
        PacketSNR = pkt_status.snr_pkt_in_db;                //read the received SNR value
        //offset = LT.getFrequencyErrorRegValue();
        /*
        Serial.println("Packet stats------");
        Serial.printf("RSSI: %d, SNR: %d \n", PacketRSSI, PacketSNR);
        Serial.println("---------------------");
        */
        uint8_t data[256];

        RXPacketL = readbufferlr11xx(data, RXBUFFER_SIZE);

        // to avoid missing packets set radi back to RX before processing
        lr11xx_radio_set_rx( &lrRadio, 0);

        //Serial.printf("RX Buffer (len: %i): \n", RXPacketL);
        //Serial.println((char*)data);

        //udp.writeTo(data, RXPacketL, IPAddress(192,168,0,174), 8280);

        //xTaskCreate(&blinky, "blinky", 1024,NULL,5,NULL);
        xSemaphoreGive(blink_rx);
        
        packetsRX++;
        lastPacket = y%100;
        packets[lastPacket] = RXPacketL;
        packetsCount++;
        y++;
        packetsInInterval++;
        
        // Check if we got a special Realtime Packet
        //0x73 = Midi Stream
        if(data[2] == 0x73){
          Serial.println("--- Got a MIDI Packet ---");
          //udp.writeTo(data+4, RXPacketL-8, IPAddress(239,1,2,3), 8281);
          uint8_t midibuf[10] = {0};
          int chunks = floor((RXPacketL-8)/10);
          //loop at data in 10byte chunks to parse Data
          memset(midiarray, 0, sizeof(midiarray));
          for (int i = 0; i < chunks; i++)
          {  
            memcpy(midibuf, data+4+i*10, 10);
          
            struct midi_data tmpmidi;

            memcpy(&tmpmidi.note, midibuf, 1);
            memcpy(&tmpmidi.start, midibuf+1, 4);
            memcpy(&tmpmidi.velocity, midibuf+5, 1);
            memcpy(&tmpmidi.duration, midibuf+6, 4);
            midiarray[i] = tmpmidi;

            // play midi on speaker
            //ledcWriteTone(0, midiNoteToFreq(tmpmidi.note));
            //delay(tmpmidi.duration*1000+tmpmidi.start);
          }
        }
        //0x69 = Test/Chat Packet for Factory Testing
        if(data[2] == 0x69){
          Serial.println("--- Got a Test Packet ---");
          Serial.println((char*)data+4);
        }
      
        if(RXPacketL > 0) {
          //Serial.printf("RX Payload Size: %02x %02x %02x %02x %02x %02x %02x %02x\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
          //Serial.println("Consume data check");
          if (!isFormatting) {            // stop consuming data during sd card formatting to not access card
            if (!sdCardPresent)
            {
              //portENTER_CRITICAL(&sxMux);
                //data_carousel.consume(data, RXPacketL);
              //portEXIT_CRITICAL(&sxMux);
            } else {
              //Serial.println("consume Data");
              data_carousel.consume(data, RXPacketL);
            }
          }
          data[0] = RXPacketL;
        }
        
      }

      if (IRQStatus & LR11XX_SYSTEM_IRQ_CRC_ERROR) {
        //Serial.println("lr11xx_SYSTEM_IRQ_CRC_ERROR");
        crc++;
      } 
      if (IRQStatus & LR11XX_SYSTEM_IRQ_HEADER_ERROR) {
        //Serial.println("lr11xx_SYSTEM_IRQ_HEADER_ERROR");
        header++;
      }
      
      /*if(IRQStatus & LR11XX_SYSTEM_IRQ_PREAMBLE_DETECTED)
      {
        Serial.println("LR11XX_SYSTEM_IRQ_PREAMBLE_DETECTED");
      }*/

      //lr11xx_radio_set_rx( &lrRadio, 0);
      //Serial.println("Set Radio back to RX");
    }
  }
}

/**
 * Simple function to count LoRa bitrate
 * @param update - time in millis from last update
 */
uint16_t countBitrate(uint16_t update)
{
  uint16_t bitrate = 0;
  int n = 0;

  do
  {
    n = lastPacket - packetsCount;
    if (n < 0)
    {
      n += 100;
    }
    
    bitrate += packets[n];
    packetsCount--;
  } while (packetsCount > 0);

  Serial.printf("bitrate: %d bits/s\n", bitrate * 8 * 1000/update);

  return bitrate * 8 * 1000/update; // bitrate in bits/s
}

/**
 * Simple function to count LoRa Paketrate
 * @param update - time in millis from last update
 */
float countPktrate(uint16_t update)
{
  float pktrate = 0;
  pktrate = (float)packetsInInterval / (update/1000);
  packetsInInterval = 0;  
  Serial.printf("pktrate: %.2f pkts/s\n", pktrate);
  return pktrate; // bitrate in bits/s
}