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
char *hw_version = HW_VERSION;
char *fw_version = FW_VERSION;

#define QUEUE_LENGTH 10
xTaskHandle rxTaskHandle = NULL;
xQueueHandle rxQueue = NULL;

static uint64_t lastUpdate = 0;
uint16_t bitrate = 0;

static bool bWire;

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
  delay(10);
}
bool sdCardPresent;
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
  if(bWire) enableLNB(1); // enable VLNB

  rxQueue = xQueueCreate(QUEUE_LENGTH, RXBUFFER_SIZE);
#ifdef CONFIG_IDF_TARGET_ESP32S2
  xTaskCreate(rxTaskSX1280, "RX_T", 10 * 1024, NULL, 6, &rxTaskHandle); // stack size may be increased to receive bigger files
#else
  xTaskCreatePinnedToCore(rxTaskSX1280, "RX_T", 10 * 1024, NULL, 6, &rxTaskHandle, 1); // stack size may be increased to receive bigger files
#endif

  web_server();

  initSX1280();
}

