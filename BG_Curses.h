#pragma once

#include "curses.h"

void bgc_InitCurses();
void bgc_StopCurses();

void  bgc_LOG_printf(const char* fmt, ...);
int bgc_ShowLog(int r);
int gbc_GetLogSize();

bool GetConsoleSize(int* cols, int* rows);
bool GetConsoleBufferSize(int* cols, int* rows);
bool SetConsoleSize(int cols, int rows);
bool SetConsoleBufferSize(int cols, int rows);


