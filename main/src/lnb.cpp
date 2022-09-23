#include "Arduino.h"
#include "Wire.h"
#include "lnb.h"
#include "customize.h"
#include "settings.h"
#include "HopeDuino_CMT211xA.h"
#include "lo.h"

cmt211xaClass radio;
uint8_t uLOid;

bool bEnableLNB;
float voltage = 0.0f;
int detectedLNB = 0;
bool bEnableLO;
bool bEnableDiseq;

byte lnbAddr = 0x08;

void searchLnbAddr(){
  int lnbAddrs[] = {0x08, 0x09, 0x10, 0x60};
  
  Serial.println("Detecting LNB Chip...");
  for (int addr : lnbAddrs){
    // try to connect to LNB Chip (TPS65xx)
    Wire.beginTransmission((uint8_t)addr);
    if (Wire.endTransmission() == 0) // Receive 0 = success (ACK response)
    {
      Serial.print("LNB Chip found at Addr: ");
      Serial.println(addr);
      lnbAddr = addr;
      break;
    }
  }
}

/**
 * Read LNB chip status to get voltage, or LNB being connected
 */
void lnbStatus()
{
  uint8_t _v = 0;
  Wire.beginTransmission((uint8_t)lnbAddr);
  if (Wire.endTransmission() == 0) // Receive 0 = success (ACK response)
  {
    Wire.beginTransmission((uint8_t)lnbAddr);
    Wire.write((uint8_t)0x00);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)lnbAddr, (uint8_t)1);
    _v = Wire.read();

    Wire.beginTransmission((uint8_t)lnbAddr);
    Wire.write((uint8_t)0x02);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)lnbAddr, (uint8_t)1);
    detectedLNB = Wire.read(); // bit1 - connected, bit5 - LDO_ON, bit0 - in range, bit3 - OCP, bit4 - termal prot
    switch ((_v & 0x1E) >> 1)
    {
      case 0x0:
        voltage = 11.0;
        break;
      case 0x01:
        voltage = 11.6;
        break;
      case 0x02:
        voltage = 12.2;
        break;
      case 0x03:
        voltage = 12.8;
        break;
      case 0x04:
        voltage = 13.4;
        break;
      case 0x05:
        voltage = 14.0;
        break;
      case 0x06:
        voltage = 14.6;
        break;
      case 0x07:
        voltage = 15.2;
        break;
      case 0x08:
        voltage = 15.8;
        break;
      case 0x09:
        voltage = 16.4;
        break;
      case 0x0a:
        voltage = 17.0;
        break;
      case 0x0b:
        voltage = 17.6;
        break;
      case 0x0c:
        voltage = 18.2;
        break;
      case 0x0d:
        voltage = 18.8;
        break;
      case 0x0e:
        voltage = 19.4;
        break;
      case 0x0f:
        voltage = 20.0;
        break;
    }
    Serial.printf("reg: %d, value: %d\n", 0x02, detectedLNB);
    if(!detectedLNB && bEnableLNB) enableLNB();
  }
}

/**
 * Enable or disable VLNB voltage
 */
extern "C" void enableLNB()
{
  searchLnbAddr();

  Wire.beginTransmission((uint8_t)lnbAddr);
  if (Wire.endTransmission() == 0) // Receive 0 = success (ACK response)
  {
    Wire.beginTransmission((uint8_t)lnbAddr);
    Wire.write((uint8_t)0x0);
    if (bEnableLNB)
    {
      Wire.write((uint8_t)138);
    }
    else
    {
      Wire.write((uint8_t)128);
    }
    Wire.endTransmission();
    Wire.beginTransmission((uint8_t)lnbAddr);
    Wire.write((uint8_t)0x1);
    if (bEnableLNB)
    {
      Wire.write((uint8_t)10);
    }
    else
    {
      Wire.write((uint8_t)0);
    }
    Wire.endTransmission();
  }
}

extern "C" void enableLO(bool en, uint8_t id)
{
  pinMode(LO_CLK, OUTPUT);

  radio.Chipset        = CMT2119A;
  radio.SymbolTime     = 416;
  radio.vCMT2119AInit(CfgTbl[id], 21);

  vTaskDelay(50);
  gpio_set_level(LO_DATA, 0);
  vTaskDelay(50);
  if (en)
  {
    gpio_set_level(LO_DATA, 1);  
  }
  bEnableLO = en;
  log_i("LO: %d", bEnableLO);
}

extern "C" void enable22kHz(bool en)
{
  gpio_set_level(TPS_EXTM, 0);
  vTaskDelay(100);
  gpio_set_level(TPS_EXTM, en);
  bEnableDiseq = en;
  log_i("diseq: %d", bEnableDiseq);
}
