/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: 
 *  (1) "AS IS" WITH NO WARRANTY; 
 *  (2) TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, HopeRF SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) HopeRF
 *
 * website: www.HopeRF.com
 *          www.HopeRF.cn     
 */

/*! 
 * file       HopeDuino_CMT211xA.cpp
 * brief      driver for CMT211xA 
 * hardware   HopeDuino with RFM11x
 *            
 *
 * version    1.0
 * date       Feb 18 2016
 * author     QY Ruan
 */

#include "HopeDuino_CMT211xA.h"
 
/**********************************************************
**Name:     vCMT211xAInit
**Function: Init. CMT211xA
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vCMT211xAInit(void)
{
 Twi.vTWIInit();	

 Twi.vTWIReset();				//step 1
 vSoftReset();					//step 2
 vTwiOff();						//step 3

 ClrTDAT();						//Set Dat to low
 OutputTDAT();					//Read for Tx
}

/**********************************************************
**Name:     vCMT2119AInit
**Function: Init. CMT2119A with config 
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vCMT2119AInit(uint16_t para[], byte length)
{
 byte i;	
 Twi.vTWIInit();	

 Twi.vTWIReset();				//step 1
 vSoftReset();					//step 2
 delay(2);					//wait more than 1ms		
 vOpenLdoAndOsc();				//step 3
 vActiveRegsister();			//step 4
 vEnableRegMode();				//step 5
 for(i=0;i<length;i++)			//step 6
	vWriteReg(i, para[i]);		
 vTwiOff();						//step 7	

 ClrTDAT();						//Set Dat to low
 OutputTDAT();					//Read for Tx
}

/**********************************************************
**Name:     vCMT211xASleep
**Function: set CMT211xA to sleep
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vCMT211xASleep(void)
{
 Twi.vTWIReset();				//step 1
 vSoftReset();					//step 2
}

/**********************************************************
**Name:     vCMT2119ASleep
**Function: set CMT2119A to sleep
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vCMT2119ASleep(void)
{
 vCloseLdoAndOsc();				//step 1
 Twi.vTWIReset();				//step 2
 vSoftReset();					//step 3
}

/**********************************************************
**Name:     vEncode
**Function: encode 
**Input:    ptr��length��etype
            length need less than 64
**Output:   TxBuf
**********************************************************/
void cmt211xaClass::vEncode(byte ptr[], byte length, encodeType etype)
{
 byte i, j, k;	
 
 switch(etype)
 	{
	case E527:
		TxBuf[0] = 0x80;			
		TxBuf[1] = 0x00;
		TxBuf[2] = 0x00;
		TxBuf[3] = 0x00;		
		for(i=4; i<(4+(length<<2)); i++)	//1XX0
			TxBuf[i] = 0x88;
 		k = 4;
 		for(j=0; j<length; j++)
 			{
 			for(i=0x80; i!=0; )
 				{
 				if((ptr[j]&i)!=0)
 					TxBuf[k] |= 0x60;
 				i >>= 1;
 				if((ptr[j]&i)!=0)
 					TxBuf[k] |= 0x06;
 				i >>= 1;
 				k++;
 				}
			}
		TxBufLength = ((length<<2)+4);
		break;			
	case E201:
 		for(i=0; i<4; i++)			//
     		TxBuf[i] = 0x00;		//4 byte 0x00
 		TxBuf[4] = 0x0A;			//10 pulse preamble	
 		for(i=5; i<(5+2); i++)
	 		TxBuf[i] = 0xAA;		
		TxBuf[i++] = 0x00;			//Sync
		TxBuf[i++] = 0x00;
		k = i+(length*3);
		j = i;		
 		for( ; j<k ; )				
 			{
 			TxBuf[j++] = 0x92;		//1x01x01xB
 			TxBuf[j++] = 0x49;		//01x01x01B
 			TxBuf[j++] = 0x24;		//x01x01x0B
			}

 		for(j=0; j<length; j++)
 			{
			if((ptr[j]&0x80)==0x00)
				TxBuf[i] |= 0x40;
			if((ptr[j]&0x40)==0x00)
				TxBuf[i] |= 0x08;
			if((ptr[j]&0x20)==0x00)
				TxBuf[i] |= 0x01;
			i++;
			if((ptr[j]&0x10)==0x00)
				TxBuf[i] |= 0x20;
			if((ptr[j]&0x08)==0x00)
				TxBuf[i] |= 0x04;
			i++;
			if((ptr[j]&0x04)==0x00)
				TxBuf[i] |= 0x80;
			if((ptr[j]&0x02)==0x00)
				TxBuf[i] |= 0x10;
			if((ptr[j]&0x01)==0x00)
				TxBuf[i] |= 0x02;
			i++;
 			}
 		TxBuf[i++] = 0x92;
		TxBufLength = i;
		break;
	case ENRZ: 	
 	default:
 		for(i=0; i<length; i++)		//do nothing
 			TxBuf[i] = ptr[i];
 		TxBufLength = length;
 		break;
 	}
}

/**********************************************************
**Name:     vTxPacket
**Function: send TxBuf
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vTxPacket(void)
{
//  byte i, j;
//  byte bittime;
 
//  Twi.vTWIInit();	
//  Twi.vTWIReset();				//step 1
//  vTwiOff();						//step 2
 
//  TIFR0  = 0x00;
//  TIMSK0 = 0x00;
//  OCR0A  = 0x00;
//  TCCR0A = 0x0E;		//
//  TCCR0B = 0x00; 
//  TCNT0  = 0x00;
 
//  if(SymbolTime<16)
//  	{
//  	bittime = (SymbolTime<<4);	//unit = 1/16 us	
//  	bittime -= 4;				//compensate
//  	TCCR0B = 0x01;				//CLK/1 = 16MHz/1 = 16MHz Start	
//  	}				
//  else if(SymbolTime<128)
//  	{
//  	bittime = (SymbolTime<<1);	//unit = 1/2 us
//  	bittime -= 1;				//compensate
//  	TCCR0B = 0x02;				//CLK/8 = 16MHz/8 = 2MHz Start	
//  	}
//  else if(SymbolTime<1024)
//  	{
//  	bittime = (SymbolTime>>2);	//unit = 4us
//  	TCCR0B = 0x03;				//CLK/64 = 16MHz/64 
//  	}
//  else if(SymbolTime<4096)
//  	{
//  	bittime = (SymbolTime>>4);	//unit = 16us
//  	TCCR0B = 0x04;				//CLK/256 = 16MHz/256
//  	}
//  else if(SymbolTime<16384)
//  	{
//  	bittime = (SymbolTime>>6);	//unit = 64us
//  	TCCR0B  = 0x05;				//CLK/1024 = 16MHz/1024
//  	}
//  else
//  	{
//  	bittime = 255;
//  	TCCR0B  = 0x05;
//  	}
 
//  for(i=0; i<TxBufLength; i++)
//  	{
//  	for(j=0x80; j!=0; j>>=1)
//  		{
//  		if(TxBuf[i]&j)
//  			SetTDAT();
//  		else
//  			ClrTDAT();
//  		OCR0A  = bittime;
//  		while((TIFR0&0x02)!=0x02);
//  		TIFR0 = 0x02;		//Clear flag
//  		}
//  	}
 
//  OCR0A  = 0;
//  TCCR0A = 0x00;		// 	
//  TCCR0B = 0x00;
//  ClrTDAT(); 
}

//**********************HAL Layer**************************
/**********************************************************
**Name:     vOpenLdoAndOsc
**Function: Open LDO & Osc(only for CMT2119A)
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vOpenLdoAndOsc(void)
{
 if(Chipset==CMT2119A)	
 	Twi.vTWIWrite(0x02, 0x78);
}

/**********************************************************
**Name:     vCloseLdoAndOsc
**Function: Close LDO & Osc (only for CMT2119A)
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vCloseLdoAndOsc(void)
{
 if(Chipset==CMT2119A)		
 	Twi.vTWIWrite(0x02, 0x7F);
}

/**********************************************************
**Name:     vActiveRegsister
**Function: Active Regsisiter Mode (Step-4)  (only for CMT2119A)
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vActiveRegsister(void)
{
 if(Chipset==CMT2119A)	
 	{	
 	Twi.vTWIWrite(0x2F, 0x80);
 	Twi.vTWIWrite(0x35, 0xCA);
 	Twi.vTWIWrite(0x36, 0xEB);
 	Twi.vTWIWrite(0x37, 0x37);
 	Twi.vTWIWrite(0x38, 0x82);
	}
}	
	
/**********************************************************
**Name:     EnableRegMode
**Function: Active Enable Regsisiter Mode (Step-5)  (only for CMT2119A)
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vEnableRegMode(void)
{
 if(Chipset==CMT2119A)
 	{
 	Twi.vTWIWrite(0x12, 0x10);
 	Twi.vTWIWrite(0x12, 0x00);
 	Twi.vTWIWrite(0x24, 0x07);
 	Twi.vTWIWrite(0x1D, 0x20);
	}
}

/**********************************************************
**Name:     vWriteReg
**Function: wirte something to Regsisiter  (only for CMT2119A)
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vWriteReg(byte adr, word wrPara)
{
 if(Chipset==CMT2119A)	
 	{
 	Twi.vTWIWrite(0x18, adr);
 	Twi.vTWIWrite(0x19, (byte)wrPara);
 	Twi.vTWIWrite(0x1A, (byte)(wrPara>>8));
 	Twi.vTWIWrite(0x25, 0x01);
	}
}

/**********************************************************
**Name:     vReadReg
**Function: read something from Regsisiter 	(only for CMT2119A)
**Input:    none
**Output:   none
**********************************************************/
word cmt211xaClass::wReadReg(byte adr)
{
 byte i_H, i_L;
 if(Chipset==CMT2119A)	
 	{		
 	Twi.vTWIWrite(0x18, adr);
 	i_L = Twi.bTWIRead(0x1b);
 	i_H = Twi.bTWIRead(0x1c);
 	return(((word)i_H)<<8|i_L);
	}
 else
 	return(0);
}

/**********************************************************
**Name:     vSoftReset
**Function: Software Reset Chipset
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vSoftReset(void)
{
 Twi.vTWIWrite(0x3D, 0x01);		//0xBD01
}

/**********************************************************
**Name:     vTwiReset
**Function: Twi mode on
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vTwiReset(void)
{
 Twi.vTWIReset();
}

/**********************************************************
**Name:     vTwiOff
**Function: Twi mode off
**Input:    none
**Output:   none
**********************************************************/
void cmt211xaClass::vTwiOff(void)
{
 Twi.vTWIWrite(0x0D, 0x02);		//0x8D02
}
