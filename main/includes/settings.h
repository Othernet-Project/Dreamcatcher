/*******************************************************************************************************
  lora Programs for Arduino - Copyright of the author Stuart Robinson - 02/03/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//*******  Setup hardware pin definitions here ! ***************

//These are the pin definitions for one of my own boards, a ESP32 shield base with a Mikrobus shield on
//top. Be sure to change the definitions to match your own setup. Some pins such as DIO2, DIO3, BUZZER
//may not be in used by this sketch so they do not need to be connected and should be included and be 
//set to -1.
#pragma once 
#include <SX128XLT.h>                                          //include the appropriate library  
#include "customize.h"
#include "AsyncUDP.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LORA_DEVICE DEVICE_SX1280               //we need to define the device we are using

#define Program_Version "V1.0"

extern uint8_t defaultsPin;
extern unsigned long resetStart;

//*******  Setup LoRa Parameters Here ! ***************

//LoRa Modem Parameters
extern uint32_t Frequency;           //frequency of transmissions
extern int32_t Offset;                        //offset frequency for calibration purposes  
extern uint8_t Bandwidth;          //LoRa bandwidth
extern uint8_t SpreadingFactor;        //LoRa spreading factor
extern uint8_t CodeRate;            //LoRa coding rate
extern uint8_t PacketLength;

extern int8_t TXpower;                      //LoRa transmit power in dBm

#define RXBUFFER_SIZE 255                        //RX buffer size  (MAX 255)

extern AsyncUDP udp;

void readMBR();
esp_err_t initSDcard();
esp_err_t initSPIFFS();
void initSX1280();
IRAM_ATTR void rxTaskSX1280(void* p);
IRAM_ATTR uint8_t readbufferSX1280(uint8_t *rxbuffer, uint8_t size);
IRAM_ATTR void rxTxISR();

void loadSettings();
void storeLoraSettings();
void storeWifiCredsAP(char* ssid, char* pass);
void storeWifiCredsSTA(char* ssid, char* pass);
void updateLoraSettings(uint32_t freq, uint8_t bw, uint8_t sf, uint8_t cr);
uint16_t countBitrate(uint16_t update);
void vTaskGetRunTimeStats2();
void setDefaults();
extern xQueueHandle rxQueue;

#ifdef __cplusplus
}
#endif
