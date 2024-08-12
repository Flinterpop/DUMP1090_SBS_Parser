#pragma once

#include <WTypesbase.h>


typedef struct _ByteBuffer {
	int bufLength;
	char* buffer;
} ByteBuffer;

void bgu_UnixTimeToFileTime(time_t t, LPFILETIME pft);
BOOL bgu_FileExists(LPCTSTR szPath);


void bgu_clearConsole();





