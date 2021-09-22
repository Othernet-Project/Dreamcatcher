#include "Arduino.h"
#include "lnb.h"

static void bit_one()
{
    enable22kHz(1);
    ets_delay_us(500);
    enable22kHz(0);
    ets_delay_us(1000);
}

static void bit_zero()
{
    enable22kHz(1);
    ets_delay_us(1000);
    enable22kHz(0);
    ets_delay_us(500);
}

void framingE0()
{
    bit_one();
    bit_one();
    bit_one();
    bit_zero();
    bit_zero();
    bit_zero();
    bit_zero();
    bit_zero();
}

void sendNimble(uint8_t val)
{
    switch (val)
    {
    case 0: // 0000
        bit_zero();
        bit_zero();
        bit_zero();
        bit_zero();
        break;
    case 1: // 0001
        bit_zero();
        bit_zero();
        bit_zero();
        bit_one();
        break;
    case 2: // 0010
        bit_zero();
        bit_zero();
        bit_one();
        bit_zero();
        break;
    case 3: // 0011
        bit_zero();
        bit_zero();
        bit_one();
        bit_one();
        break;
    case 4: // 0100
        bit_zero();
        bit_one();
        bit_zero();
        bit_zero();
        break;
    case 5: // 0101
        bit_zero();
        bit_one();
        bit_zero();
        bit_one();
        break;
    case 6: // 0110
        bit_zero();
        bit_one();
        bit_one();
        bit_zero();
        break;
    case 7: // 0111
        bit_zero();
        bit_one();
        bit_one();
        bit_one();
        break;
    case 8: // 1000
        bit_one();
        bit_zero();
        bit_zero();
        bit_zero();
        break;
    case 9: // 1001
        bit_one();
        bit_zero();
        bit_zero();
        bit_one();
        break;
    case 10: // 1010
        bit_one();
        bit_zero();
        bit_one();
        bit_zero();
        break;
    case 11: // 1011
        bit_one();
        bit_zero();
        bit_one();
        bit_one();
        break;
    case 12: // 1100
        bit_one();
        bit_one();
        bit_zero();
        bit_zero();
        break;
    case 13: // 1101
        bit_one();
        bit_one();
        bit_zero();
        bit_one();
        break;
    case 14: // 1110
        bit_one();
        bit_one();
        bit_one();
        bit_zero();
        break;
    case 15: // 1111
        bit_one();
        bit_one();
        bit_one();
        bit_one();
        break;
    }
}

void writeFreqCmd()
{
    sendNimble(5);
    sendNimble(8);
}

void setVpolarity()
{
    sendNimble(2);
    sendNimble(1);
}

void burstSatA()
{
    enable22kHz(1);
    ets_delay_us(12500);
    enable22kHz(0);
}

void _pause()
{
    // The end of each DiSEqC message is signalled by a minimum of 6ms of silence
    ets_delay_us(6100); 
}

void freqToBCD(int freq)
{
    uint8_t n = 0;
    char f[10] = {};
    itoa(freq, f, 10);

    Serial.print(freq);
    Serial.printf(" > (%d) > ", strlen(f));
    Serial.println(f);
    for (size_t i = 0; i < strlen(f); i++)
    {
        uint8_t val = f[i] - 0x30;
        // Serial.println(val);
        sendNimble(val);
    }
}

void address()
{
    freqToBCD(11);
}

void sendMasterCmd(int freq)
{
    enable22kHz(0);
    delay(16);

    // framing, pause, address, pause, command, pause, data, pause
    framingE0();
    _pause();
    address();
    _pause();
    writeFreqCmd();
    _pause();
    freqToBCD(freq);
    _pause();

    delay(16);
    burstSatA();
    delay(16);
}

/*
channel decode UB = 1<<19
Tune_Wrd = 12000 - 10600 - 100
POL = 3(H) or 1(V)
UB + Tune_Wrd + POL

Select Tune_Wrd
  Case <10700
      HRSOut Hex4 Tune_Wrd,13
      If Tune_Wrd < 10 Then 
          GoTo exit
      EndIf
      HRSOut "Error in Tune Freq l",13,10
      GoTo OUT
  Case > 12750
      HRSOut "Error in Tune Freq h",13,10
      GoTo OUT
  Case <= 11700
      Tune_Wrd = Tune_Wrd - 9750
        HRSOut "LB detected",13,10 
        Dec POL                  
  Case Else
      Tune_Wrd = Tune_Wrd - 10600

*/
uint32_t prepareODU(char* ub, char* freq, char* p)
{
    log_i("Freq: %s", freq);
    Serial.print("Channel: ");
    Serial.println(ub);
    Serial.print("Polarization: ");
    Serial.println(p);
    uint32_t ret = 0x70<<24;
    uint8_t ch = atoi(ub);
    ch--;
    uint8_t POL = 0;
    uint32_t tune = atoi(freq);

    if (strcmp(p, "V") == 0) {
        POL = 1;
    } else if (strcmp(p, "H") == 0) {
        POL = 3;
    }

    if (tune <= 11700)
    {
        tune -= 9750;
        POL--;
    } else {
        tune -= 10600;
    }

    tune -= 100;

    ret = ret + (tune<<8) + (ch<<19) + POL;
    Serial.println(ret);
    return ret;
}

extern "C" void sendODUchannel(uint32_t _v)
{
    uint8_t val[4] = {};
    memcpy(val, &_v, 4);
    // reverted order (little endian)
    Serial.println(val[3]);
    Serial.println(val[2]);
    Serial.println(val[1]);
    Serial.println(val[0]);

    for (int i = 3; i >= 0; i--) {
        sendNimble(val[i]>>4);
        sendNimble(val[i]&0xf);
    }    
}


