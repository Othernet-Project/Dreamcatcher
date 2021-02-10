
#include "Arduino.h"
#pragma once

#include <SPI.h>                                               //the lora device is SPI based so load the SPI library                                         
#include <SX128XLT.h>                                          //include the appropriate library  
#include "settings.h"                                          //include the setiings file, frequencies, LoRa settings etc   

extern uint8_t TXPacketL;
extern uint32_t TXPacketCount, startmS, endmS;
extern uint32_t RXpacketCount;
extern uint32_t errors;
extern uint16_t IRQStatus;

extern uint8_t RXPacketL;                               //stores length of packet received
extern int8_t  PacketRSSI;                              //stores RSSI of received packet
extern int8_t  PacketSNR;                               //stores signal to noise ratio (SNR) of received packet

extern uint8_t messageLength;
extern SX128XLT LT;
