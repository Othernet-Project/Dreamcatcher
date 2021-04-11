/*******************************************************************************************************
   DreamCatcher app for esp32 and esp32-S2
   author chegewara
*******************************************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "customize.h"
#include "settings.h"

#include "sx1280.h"
#include "SPI.h"
#include "lnb.h"
#include "Wire.h"
#include "wifi.h"


char myIP[20] = "192.168.4.1";

#define QUEUE_LENGTH 10
xTaskHandle rxTaskHandle = NULL;
xQueueHandle rxQueue = NULL;

static uint64_t lastUpdate = 0;
uint16_t bitrate = 0;

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
  }

  if (resetStart + 5000 < millis() && !defaultsPin)
  {
    defaultsPin = 1;
    Serial.print("Reset to defaults ");
    setDefaults();
  }
  
  delay(10);
}

IRAM_ATTR void tpsFault()
{
  ets_printf("LNB fault detected");
}

void setup()
{
  Serial.begin(115200);
  bWire = Wire.begin(I2C_SDA, I2C_SCL);
  vTaskPrioritySet(NULL, 5);
  loadSettings();

  sdCardPresent = true;
  if(initSDcard() != ESP_OK) {
    sdCardPresent = false;

    if(initSPIFFS() != ESP_OK) {
      log_e("error to init sd card and spiffs storage");
    }
  }
  if(bWire) enableLNB(); // enable VLNB

  rxQueue = xQueueCreate(QUEUE_LENGTH, RXBUFFER_SIZE);
#ifdef CONFIG_IDF_TARGET_ESP32S2
  xTaskCreate(rxTaskSX1280, "RX_T", 10 * 1024, NULL, 6, &rxTaskHandle); // stack size may be increased to receive bigger files
#else
  xTaskCreatePinnedToCore(rxTaskSX1280, "RX_T", 10 * 1024, NULL, 6, &rxTaskHandle, 1); // stack size may be increased to receive bigger files
#endif

  gpio_set_direction(LO_DATA, GPIO_MODE_OUTPUT);
  gpio_set_direction(TPS_EXTM, GPIO_MODE_OUTPUT);

  gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
  gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLUP_ONLY);
  attachInterrupt(1, tpsFault, FALLING);

  web_server();

  initSX1280();
}

