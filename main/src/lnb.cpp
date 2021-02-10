#include "Arduino.h"
#include "Wire.h"
#include "lnb.h"
#include "customize.h"


float voltage = 0.0f;
int detectedLNB = 0;

/**
 * Read LNB chip status to get voltage, or LNB being connected
 */
void lnbStatus()
{
  uint8_t _v = 0;
  Wire.beginTransmission(LNB_ADDRESS);
  if (Wire.endTransmission() == 0) // Receive 0 = success (ACK response)
  {
    // Serial.println("LNB chip detected");
    Wire.beginTransmission((uint8_t)LNB_ADDRESS);
    Wire.write((uint8_t)0x00);
    Wire.endTransmission();
    Wire.requestFrom(LNB_ADDRESS, 1);
    _v = Wire.read();

    Wire.beginTransmission((uint8_t)LNB_ADDRESS);
    Wire.write((uint8_t)0x02);
    Wire.endTransmission();
    Wire.requestFrom(LNB_ADDRESS, 1);
    detectedLNB = Wire.read(); // bit2 - connected, bit5 - LDO_ON, bit1 - in range
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
    log_d("reg: %d, value: %d\n", 0x02, detectedLNB);
  }
}

/**
 * Enable or disable VLNB voltage
 */
void enableLNB(bool enable)
{
  Wire.beginTransmission(LNB_ADDRESS);
  if (Wire.endTransmission() == 0) // Receive 0 = success (ACK response)
  {
    log_i("LNB chip detected");
    Wire.beginTransmission((uint8_t)LNB_ADDRESS);
    Wire.write((uint8_t)0x0);
    if (enable)
    {
      Wire.write((uint8_t)138);
    }
    else
    {
      Wire.write((uint8_t)0x0);
    }
    Wire.endTransmission();
    Wire.beginTransmission((uint8_t)LNB_ADDRESS);
    Wire.write((uint8_t)0x1);
    if (enable)
    {
      Wire.write((uint8_t)138);
    }
    else
    {
      Wire.write((uint8_t)136);
    }
    Wire.endTransmission();
  }
}
