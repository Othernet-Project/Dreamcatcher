/*******************************************************************************************************
   DreamCatcher app for esp32 and esp32-S2
   author chegewara
*******************************************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "customize.h"
#include "settings.h"
#include "SPI.h"
#include "lnb.h"
#include "Wire.h"
#include "wifi.h"

#if LORA_USE_LR1110
  #include "lr11xx.h"
#else
  #include "sx1280.h"
#endif


char myIP[20] = "192.168.4.1";

#define QUEUE_LENGTH 10
xTaskHandle rxTaskHandle = NULL;
xQueueHandle rxQueue = NULL;

static uint64_t lastUpdate = 0;
uint32_t bitrate = 0;

static bool bWire;
bool sdCardPresent;

uint8_t defaultsPin = 1;
unsigned long resetStart = 0;

void loop()
{
  uint16_t update = millis() - lastUpdate;
  if (update >= 1000)
  {
    lastUpdate = millis();
    bitrate = countBitrate(update);
    if(bWire) lnbStatus();
    vTaskGetRunTimeStats2();
    //Serial.println("Alive...");
    Serial.printf("Alive... DIO1: %i \n", digitalRead(DIO1));
    //heap_caps_print_heap_info(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT | MALLOC_CAP_8BIT);

    //lrSendData();
  }
  /*
  if (resetStart + 5000 < millis() && !defaultsPin)
  {
    defaultsPin = 1;
    Serial.print("Reset to defaults ");
    setDefaults();
  }
  */
  
  delay(10);
}

IRAM_ATTR void tpsFault()
{
  ets_printf("LNB fault detected");
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting Setup...");

  // enable RF section
  gpio_reset_pin((gpio_num_t)RF_PWR);
  gpio_set_direction((gpio_num_t)RF_PWR, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)RF_PWR, 1);

  /*// switch over subghz
  gpio_reset_pin((gpio_num_t)RF_SW_SMA1);
  gpio_set_direction((gpio_num_t)RF_SW_SMA1, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)RF_SW_SMA1, 1);

  // switch over subghz
  gpio_reset_pin((gpio_num_t)RF_SW_SMA2);
  gpio_set_direction((gpio_num_t)RF_SW_SMA2, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)RF_SW_SMA2, 0);

  // switch over SMA/F
  gpio_reset_pin((gpio_num_t)RF_SW_SUBG1);
  gpio_set_direction((gpio_num_t)RF_SW_SUBG1, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)RF_SW_SUBG1, 1);

  // switch over SMA/F
  gpio_reset_pin((gpio_num_t)RF_SW_SUBG2);
  gpio_set_direction((gpio_num_t)RF_SW_SUBG2, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)RF_SW_SUBG2, 0);
*/
  bWire = Wire.begin(I2C_SDA, I2C_SCL);
  vTaskPrioritySet(NULL, 5);
  loadSettings();

  Serial.println("SD Cards next");
  sdCardPresent = true;
  if(initSDcard() != ESP_OK) {
    sdCardPresent = false;

    if(initSPIFFS() != ESP_OK) {
      log_e("error to init sd card and spiffs storage");
    }
  }
  
  if(bWire) enableLNB(); // enable VLNB
  enable22kHz(bEnableDiseq);   // switch LNB to high band if needed
  //enableLO(bEnableLO, uLOid);  // if neccessary, enable local oscillator
  
  rxQueue = xQueueCreate(QUEUE_LENGTH, RXBUFFER_SIZE);
  if (LORA_USE_LR1110)
  {
    Serial.println("LR1110 is used");
    xTaskCreate(rxTaskLR11xx, "RX_T", 16 * 1024, NULL, 6, &rxTaskHandle);
    //xTaskCreate(dio1IrqTask, "RX_T", 10 * 1024, NULL, 6, &rxTaskHandle);
  } else {
    Serial.println("SX1280 is used");
    xTaskCreate(rxTaskSX1280, "RX_T", 16 * 1024, NULL, 6, &rxTaskHandle); // stack size may be increased to receive bigger files
  }

  gpio_set_direction(LO_DATA, GPIO_MODE_OUTPUT);
  gpio_set_direction(TPS_EXTM, GPIO_MODE_OUTPUT);

  gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
  gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLUP_ONLY);
  attachInterrupt(1, tpsFault, FALLING);

  web_server();

  if (LORA_USE_LR1110)
  {
    Serial.println("init LR1110");
    initLR11xx();
    getLR11xxInfo();
  } else {
    Serial.println("init SX1280");
    initSX1280();
  }
  
}

