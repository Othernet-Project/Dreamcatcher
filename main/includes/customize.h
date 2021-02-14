#pragma once
#include "sdkconfig.h"

#define HW_VERSION "2021.01"
#define FW_VERSION "1.0.1-beta"

#define EXAMPLE_ESP_WIFI_SSID      "esp32s3"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
#define DEFAULT_FREQUENCY 2400000000
#define DEFAULT_BW LORA_BW_0800
#define DEFAULT_SF LORA_SF9
#define DEFAULT_CR LORA_CR_4_5

#define LNB_ADDRESS 0x08 // or 0x09

#ifdef CONFIG_IDF_TARGET_ESP32S2
#define SD_MISO  (gpio_num_t)36
#define SD_MOSI  (gpio_num_t)35
#define SD_SCK   (gpio_num_t)34
#define SD_CS    (gpio_num_t)33

// LoRa SPI
#define LORA_NSS 7                                   //select pin on LoRa device
#define LORA_SCK 4                                  //LORA_SCK on SPI3
#define LORA_MISO 6                                 //LORA_MISO on SPI3 
#define LORA_MOSI 5                                 //LORA_MOSI on SPI3 

#define NRESET 8                               //reset pin on LoRa device
#define RFBUSY 9                               //busy line
#define DIO1 10                                 //DIO1 pin on LoRa device, used for RX and TX done 

// I2C
#define I2C_SDA 16
#define I2C_SCL 17

#else

#define SD_MISO  (gpio_num_t)19
#define SD_MOSI  (gpio_num_t)18
#define SD_SCK   (gpio_num_t)4
#define SD_CS    (gpio_num_t)0

// LoRa SPI
#define LORA_NSS 27                                  //select pin on LoRa device
#define LORA_SCK 26                                  //LORA_SCK on SPI3
#define LORA_MISO 23                                 //LORA_MISO on SPI3 
#define LORA_MOSI 25                                 //LORA_MOSI on SPI3 

#define NRESET 12                               //reset pin on LoRa device
#define RFBUSY 13                               //busy line
#define DIO1 14                                 //DIO1 pin on LoRa device, used for RX and TX done 

// I2C
#define I2C_SDA 21
#define I2C_SCL 22

#endif
