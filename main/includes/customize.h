#pragma once
#include "sdkconfig.h"

#define HW_VERSION "2023.01"
#define FW_VERSION "1.0.10"

#define EXAMPLE_ESP_WIFI_SSID      "Othernet"
#define EXAMPLE_ESP_WIFI_PASS      "othernet"
#define DEFAULT_FREQUENCY 2308100000
#define DEFAULT_BW 0x0D
#define DEFAULT_SF 0x09
#define DEFAULT_CR 0x01

#define LORA_USE_LR1110 true

//#define LNB_ADDRESS 0x09 // Now handled in lnb.cpp - older boards 0x08, DC4 0121 rev002 0x09, Q233 0x60


//DC 2206 (LR based)
#define SD_MISO  (gpio_num_t)10
#define SD_MOSI  (gpio_num_t)39
#define SD_SCK   (gpio_num_t)38
#define SD_CS    (gpio_num_t)40

// LoRa SPI
#define LORA_NSS 14                                   //select pin on LoRa device
#define LORA_SCK 12                                  //LORA_SCK on SPI3
#define LORA_MISO 13                                 //LORA_MISO on SPI3 
#define LORA_MOSI 11                                 //LORA_MOSI on SPI3 

#define NRESET 3                               //reset pin on LoRa device
#define RFBUSY 47                               //busy line
#define DIO1 4                                //DIO1 pin on LoRa device, used for RX and TX done 2 on SX, 9 on LR1110

// I2C
#define I2C_SDA 16
#define I2C_SCL 17

#define LO_DATA (gpio_num_t)18
#define LO_CLK 41
#define TPS_EXTM (gpio_num_t)45

// other Pins
#define LED_PIN 6
#define BUZ_PIN 48
#define RF_PWR 7
#define RF_SW_SMA1 42
#define RF_SW_SMA2 41
#define RF_SW_SUBG1 8
#define RF_SW_SUBG2 5

/*
#define SD_MISO  (gpio_num_t)37
#define SD_MOSI  (gpio_num_t)39
#define SD_SCK   (gpio_num_t)38
#define SD_CS    (gpio_num_t)40

// LoRa SPI
#define LORA_NSS 14                                   //select pin on LoRa device
#define LORA_SCK 12                                  //LORA_SCK on SPI3
#define LORA_MISO 13                                 //LORA_MISO on SPI3 
#define LORA_MOSI 11                                 //LORA_MOSI on SPI3 

#define NRESET 3                               //reset pin on LoRa device
#define RFBUSY 33                               //busy line
#define DIO1 9                                //DIO1 pin on LoRa device, used for RX and TX done 2 on SX, 9 on LR1110

// I2C
#define I2C_SDA 16
#define I2C_SCL 17

#define LO_DATA (gpio_num_t)18
#define LO_CLK 41
#define TPS_EXTM (gpio_num_t)35
*/
/*
//Older DCs

#define SD_MISO  (gpio_num_t)37
#define SD_MOSI  (gpio_num_t)39
#define SD_SCK   (gpio_num_t)38
#define SD_CS    (gpio_num_t)40

// LoRa SPI
#define LORA_NSS 14                                   //select pin on LoRa device
#define LORA_SCK 12                                  //LORA_SCK on SPI3
#define LORA_MISO 13                                 //LORA_MISO on SPI3 
#define LORA_MOSI 11                                 //LORA_MOSI on SPI3 

#define NRESET 3                               //reset pin on LoRa device
#define RFBUSY 33                               //busy line
#define DIO1 9                                //DIO1 pin on LoRa device, used for RX and TX done 2 on SX, 9 on LR1110

// I2C
#define I2C_SDA 16
#define I2C_SCL 17

#define LO_DATA (gpio_num_t)18
#define LO_CLK 41
#define TPS_EXTM (gpio_num_t)35
*/
