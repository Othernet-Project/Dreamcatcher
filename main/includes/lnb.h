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

#ifdef __cplusplus
}
#endif

void lnbStatus();
