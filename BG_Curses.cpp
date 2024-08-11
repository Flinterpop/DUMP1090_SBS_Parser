

#include <WTypesbase.h>

#include "BG_Curses.h"


char CursesLOGLine[100][100];
int CursesLOGIndex = 0;



void bgc_InitCurses()
{
    CursesLOGLine[0][0] = 0;
    initscr();			/* Start curses mode 		*/
    newwin(50, 200, 40, 100);

    raw();				/* Line buffering disabled	*/
    //keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
    //noecho();			/* Don't echo() while we do getch */
    start_color();			/* Start color functionality	*/
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);

}

void bgc_StopCurses()
{
    endwin();
}



void  bgc_LOG_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[200];
    vsprintf_s(buf, fmt, args);
    va_end(args);

    if (CursesLOGIndex > 10)
    {
        CursesLOGIndex = 0; //clear and reset the log
        CursesLOGLine[0][0] = 0;
    }
    strcpy_s(CursesLOGLine[CursesLOGIndex++], buf);

}

int bgc_ShowLog(int r)
{
    mvprintw(r++, 0, "Log:");
    if (CursesLOGIndex > 0)
        for (int x = 0;x < CursesLOGIndex;x++) mvprintw(r++, 0, "%02d: %s", x, CursesLOGLine[x]);
    return r;
}

int gbc_GetLogSize()
{
    return CursesLOGIndex;
}

//Console Functions from:
//  https://stackoverflow.com/questions/25912721/set-console-window-size-on-windows

bool SetConsoleSize(int cols, int rows) {
    HWND hWnd;
    HANDLE hConOut;
    CONSOLE_FONT_INFO fi;
    CONSOLE_SCREEN_BUFFER_INFO bi;
    int w, h, bw, bh;
    RECT rect = { 0, 0, 0, 0 };
    COORD coord = { 0, 0 };
    hWnd = GetConsoleWindow();
    if (hWnd) {
        hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConOut && hConOut != (HANDLE)-1) {
            if (GetCurrentConsoleFont(hConOut, FALSE, &fi)) {
                if (GetClientRect(hWnd, &rect)) {
                    w = rect.right - rect.left;
                    h = rect.bottom - rect.top;
                    if (GetWindowRect(hWnd, &rect)) {
                        bw = rect.right - rect.left - w;
                        bh = rect.bottom - rect.top - h;
                        if (GetConsoleScreenBufferInfo(hConOut, &bi)) {
                            coord.X = bi.dwSize.X;
                            coord.Y = bi.dwSize.Y;
                            if (coord.X < cols || coord.Y < rows) {
                                if (coord.X < cols) {
                                    coord.X = cols;
                                }
                                if (coord.Y < rows) {
                                    coord.Y = rows;
                                }
                                if (!SetConsoleScreenBufferSize(hConOut, coord)) {
                                    return FALSE;
                                }
                            }
                            return SetWindowPos(hWnd, NULL, rect.left, rect.top, cols * fi.dwFontSize.X + bw, rows * fi.dwFontSize.Y + bh, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
                        }
                    }
                }
            }
        }
    }
    return FALSE;
}

bool SetConsoleBufferSize(int cols, int rows) {
    HANDLE hConOut;
    CONSOLE_SCREEN_BUFFER_INFO bi;
    COORD coord = { 0, 0 };
    hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConOut && hConOut != (HANDLE)-1) {
        if (GetConsoleScreenBufferInfo(hConOut, &bi)) {
            coord.X = cols;
            coord.Y = rows;
            return SetConsoleScreenBufferSize(hConOut, coord);
        }
    }
    return FALSE;
}

bool GetConsoleSize(int* cols, int* rows) {
    HWND hWnd;
    HANDLE hConOut;
    CONSOLE_FONT_INFO fi;
    int w, h;
    RECT rect = { 0, 0, 0, 0 };
    hWnd = GetConsoleWindow();
    if (hWnd) {
        hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConOut && hConOut != (HANDLE)-1) {
            if (GetCurrentConsoleFont(hConOut, FALSE, &fi)) {
                if (GetClientRect(hWnd, &rect)) {
                    w = rect.right - rect.left;
                    h = rect.bottom - rect.top;
                    *cols = w / fi.dwFontSize.X;
                    *rows = h / fi.dwFontSize.Y;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

bool GetConsoleBufferSize(int* cols, int* rows) {
    HANDLE hConOut;
    CONSOLE_SCREEN_BUFFER_INFO bi;
    hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConOut && hConOut != (HANDLE)-1) {
        if (GetConsoleScreenBufferInfo(hConOut, &bi)) {
            *cols = bi.dwSize.X;
            *rows = bi.dwSize.Y;
            return TRUE;
        }
    }
    return FALSE;
}

