// ConsoleTemplate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#define BMG_VERSION 1.0

#include <iostream>
#include <vector>
#include <set>
#include <algorithm> //has sort

#include <conio.h>

#include "BG_Winsock.h"
#include "BG_Utility.h"
#include "BG_Curses.h"


#define CURSES
#include "curses.h"





bool g_debug = false;

#define BUFLEN 1510  
byte _MSG[1500];
int _MSGLength;
int RxPacketCount = 0;
int RxMSGCount = 0;
int TxPacketCount = 0;


char TCPListen_IP[20] = "192.168.1.133";
int TCPListen_Port = 30003;

char UDPSend_IP[20] = "239.255.1.1";
int UDPSend_Port = 5055;



bool bPeriodicList = true;

//forward declarations
void ReadIniFile();
void PrintMenu();
void mainConsoleLoop();

HANDLE ptrToTimerHandle;
void __stdcall MainTimerCallback(PVOID, BOOLEAN);

DWORD WINAPI TCPListenThread(LPVOID lpParam);


//APP Specific Code

struct Aircraft_SBS
{
    char ICAO[10];
    int TTCounts[9];

    char CS[10];
    int Altitude;
    double GS;
    double Trk;
    double Lat;
    double Lon;
    double VerticalRate;
    char Squawk[10];
    int AlertFlag;
    int EmergFlag;
    int SPIFlag;
    int GndFlag;

    int age = 0;
    int numMessages;
    std::set<int> TranTypesRx;

    int TrkNum;

};
std::vector<Aircraft_SBS*> ACList;
int dropAfterAge = 30;
int nonMSG = 0;
int TrkBlk = 200;

enum SORT_BY { TRK_NUM , AGE, ICAO, FLIGHT, SQUAWK, GS, TRK, UPDATES, LAST_ITEM_SENTINEL };
const char* sortByName[] = {"TRK_NUM" , "AGE", "ICAO/Hex", "FLIGHT/CS","ModeA", "GS", "TRK", "NumRx", "LAST_ITEM_SENTINEL"};

constexpr int numSortTypes = static_cast<int>(LAST_ITEM_SENTINEL);
SORT_BY sortBy = TRK_NUM;

void parseSBSBuffer(char* line, int lineLength);
Aircraft_SBS* FindAC(char* ICAO);
void PrintACList();
void CursesPrintACList();






int main()
{


#ifdef CURSES
    bgc_InitCurses();
#endif

    ReadIniFile();

    initialise_winsock();

    const int UPDATE_INTERVAL = 1000; //this is ms 
    CreateTimerQueueTimer(&ptrToTimerHandle, NULL, MainTimerCallback, NULL, 500, UPDATE_INTERVAL, WT_EXECUTEDEFAULT);

    if (EXIT_FAILURE == StartListenThreadOnTCPSocket(TCPListen_IP, TCPListen_Port, TCPListenThread))
    {
        printf("Failed to Start TCP Listen Thread\r\n");
        closeandclean_winsock();
        exit(1);
    }

    OpenUDPSocket(UDPSend_IP, UDPSend_Port);

    PrintMenu();
    mainConsoleLoop();
    
    closeandclean_winsock();
#ifdef CURSES
    bgc_StopCurses();
#endif
    
}


DWORD WINAPI TCPListenThread(LPVOID lpParam)
{
    char buffer[1510];
    SOCKET* TCPClientSocket = (SOCKET*)lpParam;

    while (true) {
        int recvLen = ::recv(*TCPClientSocket, buffer, 1510, 0);

        if (SOCKET_ERROR == recvLen) continue;

        if ((recvLen > 10) && (recvLen < 1510))
        {
            buffer[recvLen] = 0;
            
            if (g_debug) printf("%s\r\n", buffer); //doesn't flush without a newline
            fflush(stdout);// Force writing buffer to the stdout
            RxPacketCount++;


            //this section breaks up buffer into individual MSG lines
            char* msgs[30];//allow up to 30 MSG per buffer
            int msgLens[30];

            int index = 0;
            msgs[index] = &buffer[0]; //first of 1 or more MSG strings in buffer

            int thisLength = 0;
            for (int x = 0;x < recvLen;x++)
            {
                if (buffer[x] == 0x0a) //this is the end of MSG line character
                {
                    buffer[x] = 0; //end the line with a c-style string end marker
                    msgLens[index] = thisLength;  //equals x for the first MSG
                    if (x < recvLen) //there are more chars in the buffer
                    {
                        msgs[++index] = &buffer[x + 1];
                        msgLens[index] = recvLen - x;   //make length remainder of buffer (for now)
                        thisLength = 0;    //reset length of this next line
                    }
                }
                thisLength++;
            }


            if (g_debug) printf("Num Lines this Buffer: %d\r\n", index);

            for (int q = 0;q < index;q++) parseSBSBuffer(msgs[q], msgLens[q]);
        }
            
        //SendUDP(buffer, recvLen);
        //TxPacketCount++;
      
    }
    return 0;
}


void __stdcall MainTimerCallback(PVOID, BOOLEAN)//(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    for (int x = 0; x < ACList.size(); x++)
    {
        ++ACList[x]->age;
        if (ACList[x]->age > dropAfterAge) ACList.erase(ACList.begin() + x);
    }

    if (bPeriodicList)
    {
        #ifdef CURSES
            CursesPrintACList();
        #else           
            bgu_clearConsole();
            PrintACList();
        #endif


    }
}

void ReadIniFile()
{
    //expects ini file like
    /*
    [Form]
        TCPListen_Port = 30154
        TCPListen_IP = "192.168.1.133"

        UDPSend_Port = 4000
        UDPSend_IP = "192.168.1.255"
     */


    printf("Attempting to read .ini file\r\n");
    LPCTSTR inipath = L".\\ADSBtoCAT21.ini";
    if (!bgu_FileExists(inipath))
    {
        std::cout << "no ini file ";
        std::wcout << inipath;
        std::cout << " found" << std::endl;
    }
    else
    {
        std::cout << "Found ini file ";
        std::wcout << inipath;
        std::cout << " found" << std::endl;
    }

    wchar_t wide_IP[20];
    GetPrivateProfileString(L"Form", L"TCPListen_IP", L"192.168.1.133", wide_IP, 20, inipath);
    size_t charsConverted = 0;
    wcstombs_s(&charsConverted, TCPListen_IP, 20, wide_IP, 20);

    GetPrivateProfileString(L"Form", L"UDPSend_IP", L"239.255.1.1", wide_IP, 20, inipath);
    wcstombs_s(&charsConverted, UDPSend_IP, 20, wide_IP, 20);

    TCPListen_Port = GetPrivateProfileInt(L"Form", L"TCPListen_PORT", 30003, inipath);
    UDPSend_Port = GetPrivateProfileInt(L"Form", L"UDPSend_PORT", 5055, inipath);
    printf("INI: TCPListen: Port: %d  IP: %s \r\n", TCPListen_Port, TCPListen_IP);
    printf("INI: UDP_Send: Port: %d  IP: %s \r\n", UDPSend_Port, UDPSend_IP);

    printf("Done reading .ini file\r\n----------------------\r\n");


}

void PrintMenu()
{
    printf("Console Template. V%f Compiled %s\r\n", BMG_VERSION, __DATE__);
    printf("Visual Studio version:%d\r\n", _MSC_VER);

    printf("Receiving TCP via Telnet on %s:%d\r\n", TCPListen_IP, TCPListen_Port);
    printf("Transmitting UDP on %s:%d\r\n", UDPSend_IP, UDPSend_Port);
    printf("Packets Rx: %d  MSG Rx: %d  Packets Tx: %d\r\n", RxPacketCount, RxMSGCount, TxPacketCount);

    printf("Commands:\t\n----------\r\nx - eXit\r\ns - Sort By\r\nt - CAT21 Type: Modern or SITAWARE\r\nl - List\r\np - Periodic List\r\nd - debug\r\n");
    printf("This app is always running even if display is not refreshing\r\n");
}

void mainConsoleLoop()
{
    bool done = false;
    while (!done)
    {
        if (_kbhit() != 0)
        {
            int ch = _getch();
            switch (ch)
            {
            case 't':
            case 'T':
                {
                static int TL = 0;
                bgc_LOG_printf("Test Log: %d", TL++);
                break;

                }
            case 'c':
            case 'C':
                bgc_StopCurses();
                bgc_InitCurses();
                break;

            case 'd':
            case 'D':
                g_debug = !g_debug;
                printf("Debug is %d\r\n", g_debug);
                break;
            case 'l':
            case 'L':
                CursesPrintACList();
                break;
            case 'p':
            case 'P':
                bPeriodicList = !bPeriodicList;
                if (bPeriodicList) printf("Periodic List On\r\n");
                else printf("Periodic List Off\r\n");
                break;
            case 's':
            case 'S':
                ++(int&)sortBy;
                if (sortBy >= numSortTypes) sortBy = TRK_NUM;
                break;
            case 'x':
            case 'X':
                done = true;
                break;

            default:
                PrintMenu();
            }
            printf("Press h for help\r\n");
        }
    }
}


Aircraft_SBS* FindAC(char* ICAO)
{
    for (auto a : ACList)
    {
        if (g_debug) printf("Comparing %s  %s\r\n", a->ICAO, ICAO);
        if (!strcmp(a->ICAO, ICAO)) return a;
    }
    return NULL;
}


void parseSBSBuffer(char* line, int lineLength)
{
    if (g_debug) printf(">>Parsing: %s\r\n", line);

    char tokenList[24][24];

    int tokenNumber = 1;  //use 1 indexing to allow code to match ICD
    int charPosInToken = 0;
    int numCommas = 0;

    //break up the MSG into tokens
    for (int x = 0;x < lineLength;x++)
    {
        char p = line[x];
        if (p == ',')
        {
            numCommas++;
            tokenList[tokenNumber][charPosInToken] = 0; //terminate the c-style string
            charPosInToken = 0; //reset for next token
            tokenNumber++;
        }
        else //assume its a letter or number (isprint() == true)
        {
            tokenList[tokenNumber][charPosInToken++] = p;
        }
    }

    if (g_debug)
    {
        printf("Parsed: %s. It has %d commas\r\n", line, numCommas);
        for (int x = 1;x <= tokenNumber;x++)
        {
            printf("%02d: %s\r\n", x, tokenList[x]);
        }
    }


    //decode the MSG line
    if (!strcmp(tokenList[1], "MSG"))
    {
        RxMSGCount++;

        Aircraft_SBS* ac = FindAC(tokenList[5]); //tokenList[5] is ICAO
        if (ac == NULL) //new track
        {
            if (g_debug) printf("First Rx of %s\r\n", tokenList[5]); //ICAO
            ac = new Aircraft_SBS;
            strcpy_s(ac->ICAO, 10, tokenList[5]); //ICAO
            ac->Squawk[0] = 0;
            ac->age = 0;
            ac->Lat = 0;
            ac->Lon = 0;
            ac->Altitude = 0;
            ac->GS = 0;
            ac->Trk = 0;
            ac->numMessages = 1;
            ac->CS[0] = 0;
            ac->TrkNum = TrkBlk++;
            for (int i = 0;i < 9;i++) ac->TTCounts[i] = 0;
            ACList.push_back(ac);
        }
        else //old track sp update some metadata
        {
            ac->age = 0;
            ++ac->numMessages;
        }

        int TransmissionType = strtol(tokenList[2], NULL, 10);
        if (g_debug) printf("Trans Type: %d  for ICAO %s >", TransmissionType, tokenList[5]);
        ac->TranTypesRx.insert(TransmissionType);
        ++ac->TTCounts[TransmissionType];

        switch (TransmissionType)
        {
        case 1:
        {
            strcpy_s(ac->CS, 10, tokenList[11]);
            if (g_debug) printf("Trans Type 1: CS: %s", ac->CS);
            break;
        }
        case 2:
        {
            ac->Altitude = strtol(tokenList[12], NULL, 10);
            ac->GS = strtod(tokenList[13], NULL);
            ac->Trk = strtod(tokenList[14], NULL);
            ac->Lat = strtod(tokenList[15], NULL);
            ac->Lon = strtod(tokenList[16], NULL);
            ac->GndFlag = strtol(tokenList[22], NULL, 10);
            if (g_debug) printf("Msg Type 2: Alt: %d GS: %f  Trk: %f Lat %f  Lon %f GF:%d\r\n", ac->Altitude, ac->GS, ac->Trk, ac->Lat, ac->Lon, ac->GndFlag);
            break;
        }
        case 3:
        {
            ac->Altitude = strtol(tokenList[12], NULL, 10);
            ac->Lat = strtod(tokenList[15], NULL);
            ac->Lon = strtod(tokenList[16], NULL);
            ac->AlertFlag = strtol(tokenList[19], NULL, 10);
            ac->EmergFlag = strtol(tokenList[20], NULL, 10);
            ac->SPIFlag = strtol(tokenList[21], NULL, 10);
            ac->GndFlag = strtol(tokenList[22], NULL, 10);
            if (g_debug) printf("Msg Type 3: Alt: %d Lat %f  Lon %f AF: %d  EF:%d  SF:%d  GF:%d\r\n", ac->Altitude, ac->Lat, ac->Lon, ac->AlertFlag, ac->EmergFlag, ac->SPIFlag, ac->GndFlag);
            break;
        }
        case 4:
        {
            ac->GS = strtod(tokenList[13], NULL);
            ac->Trk = strtod(tokenList[14], NULL);
            ac->VerticalRate = strtod(tokenList[17], NULL);

            if (g_debug) printf("Msg Type 4: GS %f  Trk %f VF: %f\r\n", ac->GS, ac->Trk, ac->VerticalRate);
            break;
        }
        case 5:
        {
            ac->Altitude = strtol(tokenList[12], NULL, 10);
            ac->AlertFlag = strtol(tokenList[19], NULL, 10);
            ac->SPIFlag = strtol(tokenList[21], NULL, 10);
            ac->GndFlag = strtol(tokenList[22], NULL, 10);
            if (g_debug) printf("Msg Type 5: Alt: %d AF: %d  SF:%d  GF:%d\r\n", ac->Altitude, ac->AlertFlag, ac->SPIFlag, ac->GndFlag);
            break;
        }
        case 6:
        {
            ac->Altitude = strtol(tokenList[12], NULL, 10);
            strcpy_s(ac->Squawk, 10, tokenList[18]);
            //strcpy_s(LOGLine, 10, tokenList[18]);
            printf(tokenList[18]);

            ac->AlertFlag = strtol(tokenList[19], NULL, 10);
            ac->EmergFlag = strtol(tokenList[20], NULL, 10);
            ac->SPIFlag = strtol(tokenList[21], NULL, 10);
            ac->GndFlag = strtol(tokenList[22], NULL, 10);
            if (g_debug) printf("Msg Type 6: Alt: %d Squawk:%s  AF: %d  EF:%d  SF:%d  GF:%d\r\n", ac->Altitude, ac->Squawk, ac->AlertFlag, ac->EmergFlag, ac->SPIFlag, ac->GndFlag);
            break;
        }
        case 7:
        {
            ac->Altitude = strtol(tokenList[12], NULL, 10);
            ac->GndFlag = strtol(tokenList[22], NULL, 10);
            if (g_debug) printf("Msg Type 7: Alt: %d GF:%d\r\n", ac->Altitude, ac->GndFlag);
        }
        case 8:
        {
            ac->GndFlag = strtol(tokenList[22], NULL, 10);
            if (g_debug) printf("Msg Type 8: GF:%d\r\n", ac->GndFlag);
            break;
        }

        default:
            break;
        }
    }
    else if (!strcmp(tokenList[1], "STA"))
    {
        puts("Rx STA");
        nonMSG++;
    }
    else if (!strcmp(tokenList[1], "ID"))
    {
        puts("Rx ID");
        nonMSG++;
    }
    else if (!strcmp(tokenList[1], "AIR"))
    {
        puts("Rx AIR");
        nonMSG++;
    }
    else if (!strcmp(tokenList[1], "SEL"))
    {
        puts("Rx SEL");
        nonMSG++;
    }
    else if (!strcmp(tokenList[1], "CLK"))
    {
        puts("Rx CLK");
        nonMSG++;
    }
    else if (!strcmp(tokenList[1], "STA"))
    {
        puts("Rx STA");
        nonMSG++;
    }
    else
    {
        puts("Rx Unknown!!!!!!!!!!");
        nonMSG++;
        bgc_LOG_printf("Unknown: %d %s",lineLength, line);
    }

}

void PrintACList()
{
    printf("ICAO    CS      Squawk  Age   #Msg  Lat       Lon        Alt     GS    TRK     1   2   3   4   5   6   7   8\r\n");
    printf("------------------------------------------------------------------------------------------------------------\r\n");

    for (auto a : ACList)
    {
        printf("%6s %8s %4s    %03d   %04d  %7.4f  %8.4f  %5d   %6.2f  %5.1f ", a->ICAO, a->CS, a->Squawk, a->age, a->numMessages, a->Lat, a->Lon, a->Altitude, a->GS, a->Trk);
        for (int i = 1;i <= 8;i++)
        {
            printf("% 3d ", a->TTCounts[i]);
        }
        printf("\r\n");
    }
    printf("Num Non MSG Rx: %d\r\n", nonMSG);
}

bool bgu_strcmp(char* s1, char* s2)
{
    //if ((s1[0] == '\0') && (s2[0] == '\0')) return false;
    //if ((s2[0] == '\0') && (s1[0] != '\0')) return true;
    for (int i = 0; s1[i] != '\0' || s2[i] != '\0'; i++)
    {
        if (s1[i] != s2[i])
        {
            return ((unsigned char)s1[i] < (unsigned char)s2[i]) ? false: true;
        }
    }
    return false;
}

bool compareAC(Aircraft_SBS *left, Aircraft_SBS  *right)
{
    switch (sortBy)
    {
    case TRK_NUM:
        return (left->TrkNum < right->TrkNum);
    case AGE:
        return (left->age < right->age);
    case UPDATES:
        return (left->numMessages > right->numMessages);
    case ICAO:
        //return bgu_strcmp(left->ICAO, right->ICAO);
        return bgu_strcmp(right->ICAO, left->ICAO);
    case SQUAWK:
        return bgu_strcmp(left->Squawk, right->Squawk);
    case FLIGHT:
        //return bgu_strcmp(left->CS, right->CS);
        return bgu_strcmp(right->CS, left->CS);
    case GS:
        return (left->GS < right->GS);
    case TRK:
        return (left->Trk < right->Trk);

    default:
        return true;
    }
}

void CursesPrintACList()
{
    sort(ACList.begin(), ACList.end(), compareAC);

    int numRows = ACList.size() + 10 + gbc_GetLogSize();
    SetConsoleSize(128, numRows);

    int numCols = 19;
    int ColHdgStart[40] = {      0,     9,   17,      24,   29,    36,   46,   54,   61,  69,   76, 80, 84, 89, 94, 98, 102, 106, 109 };
    int ColStart[40] = {         0,     9,   17,      24,   29,    35,   44,   53,   60,  68,   74, 78, 82, 87, 92, 96, 100, 104, 109 };
    const char* colNames[] = { "ICAO","CS","Mode A","Age","#Msg","Lat","Lon","Alt","GS","TRK", "1","2","3","4","5","6","7",   "8","TN" };

    clear();
    int r = 0;
    //mvprintw(r++, 0, "ICAO    CS      Squawk  Age   #Msg  Lat       Lon        Alt     GS    TRK     1   2   3   4   5   6   7   8");
    for (int c = 0;c < numCols;c++)
    {
        mvprintw(r, ColHdgStart[c], colNames[c]);
    }
    ++r;

    mvchgat(0, 0, -1, A_BLINK, 1, NULL);
    mvprintw(r++, 0, "------------------------------------------------------------------------------------------------------------------");

    for (auto a : ACList)
    {
        mvprintw(r, ColStart[0], "%6s", a->ICAO);
        mvprintw(r, ColStart[1], "%s", a->CS);
        mvprintw(r, ColStart[2], "%4s", a->Squawk);
        if (a->age > (dropAfterAge - 5))
        {
            attron(COLOR_PAIR(2));
            attron(A_BOLD);
            mvprintw(r, ColStart[3], "%3d", a->age);
            attroff(A_BOLD);
            attroff(COLOR_PAIR(2));
        }
        else mvprintw(r, ColStart[3], "%3d", a->age);

        mvprintw(r, ColStart[4], "%4d", a->numMessages);
        mvprintw(r, ColStart[5], "%7.4f", a->Lat);
        mvprintw(r, ColStart[6], "%8.4f", a->Lon);

        mvprintw(r, ColStart[7], "%5d", a->Altitude);
        mvprintw(r, ColStart[8], "%5.1f", a->GS);
        mvprintw(r, ColStart[9], "%5.1f", a->Trk);
        for (int i = 1;i <= 8;i++)
        {
            mvprintw(r, ColStart[9 + i], "%3d", a->TTCounts[i]);
        }
        mvprintw(r, ColStart[9 + 9], "%d", a->TrkNum);
        ++r;
    }

    mvprintw(r++, 0, "Sort By: %s", sortByName[sortBy]);
    mvprintw(r++, 0, "LINES: %d  COLS: %d",LINES,COLS);

    mvprintw(r++, 0, "Num Non MSG Rx: %d", nonMSG);

    int cols, rows;
    GetConsoleSize(&cols, &rows);
    mvprintw(r++, 0, "Console size: cols:%d  rows:%d", cols, rows);
    GetConsoleBufferSize(&cols, &rows);
    mvprintw(r++, 0, "Console Buffer size: cols:%d  rows:%d", cols, rows);

    bgc_ShowLog(r);
    refresh();
}


