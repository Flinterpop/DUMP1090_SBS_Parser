#pragma once

#include "Aircraft_SBS.h"

int BuildCAT21FromADSB(Aircraft_SBS* ADSB_AC);
ByteBuffer BuildCAT21FromAicraft_ADSB(Aircraft_SBS* ADSB_AC);

int CreateCAT21Modern(Aircraft_SBS* ADSB_AC); //this is compliant to V2.X
int CreateCAT21SITAWARE(Aircraft_SBS* ADSB_AC);

void SetSIC_SAC(int sic, int sac);
int GetSIC();
int GetSAC();
bool isModernCAT21();
void SetModernCAT21(bool b);

