#pragma once

extern bool bEnableLNB;
extern bool bEnableLO;
extern bool bEnableDiseq;
extern uint8_t uLOid;

#ifdef __cplusplus
extern "C" {
#endif

void enableLNB();
void enable22kHz(bool en);
void enableLO(bool en, uint8_t id);

uint32_t prepareODU(char* ub, char* freq, char* p);
void sendODUchannel(uint32_t _v);
#ifdef __cplusplus
}
#endif

void lnbStatus();
