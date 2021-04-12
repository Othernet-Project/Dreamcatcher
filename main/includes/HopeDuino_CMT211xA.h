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
 * file       HopeDuino_CMT211xA.h
 * brief      show how to use CMT211xA
 * hardware   HopeDuino
 *            
 *
 * version    1.0
 * date       Feb 18 2016
 * author     QY Ruan
 */

#ifndef HopeDuino_CMT211XA_h
	#define HopeDuino_CMT211XA_h

	#include "HopeDuino_TWI.h"
	

	enum chipsetType {CMT2110A, CMT2113A, CMT2117A, CMT2119A};
	enum encodeType  {ENRZ, E527, E201};
	
	class cmt211xaClass
	{
	 public:	
	 	chipsetType Chipset;						//Chipset part number
		word SymbolTime;							//unit: us  Range: 10 - 4000
		
		
		void vCMT211xAInit(void);
		void vCMT2119AInit(uint16_t para[], byte length);
		void vCMT211xASleep(void);
		void vCMT2119ASleep(void);
		void vEncode(byte ptr[], byte length, encodeType etype);
		void vTxPacket(void);
				
	 private:
	 	twiClass Twi;
	 	byte TxBuf[64];								//Tx Buffer
	 	byte TxBufLength;							//Tx Buffer active length
	 	
		void vOpenLdoAndOsc(void);
		void vCloseLdoAndOsc(void);
		void vActiveRegsister(void);
		void vEnableRegMode(void);
		void vWriteReg(byte adr, word wrPara);
		word wReadReg(byte adr);
		void vSoftReset(void);
		void vTwiReset(void);
		void vTwiOff(void);							
	};
#else
	#warning "HopeDuino_CMT211xA.h have been defined!"

#endif