#pragma once

extern bool bEnableLNB;
extern bool bEnableLO;
extern bool bEnableDiseq;

extern "C" void enableLNB();
extern "C" void enable22kHz(bool en);
extern "C" void enableLO(bool en);

void lnbStatus();
