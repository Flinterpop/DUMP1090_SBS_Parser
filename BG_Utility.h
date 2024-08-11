#pragma once

//#include <time.h>
#include <WTypesbase.h>


void bgu_UnixTimeToFileTime(time_t t, LPFILETIME pft);
BOOL bgu_FileExists(LPCTSTR szPath);


void bgu_clearConsole();





