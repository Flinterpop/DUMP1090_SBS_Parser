

#include <winsock2.h>

#include <vector>
#include <set>
#include "BG_Utility.h"
#include "Aircraft_SBS.h"

#include "ASX_CAT21.h"

extern bool g_debug;


typedef unsigned char byte;

#define BUFLEN 1510 
unsigned char _MSG[BUFLEN];
int _MSGLength;


#define FRN1 0b10000000
#define FRN2 0b01000000
#define FRN3 0b00100000
#define FRN4 0b00010000
#define FRN5 0b00001000
#define FRN6 0b00000100
#define FRN7 0b00000010

#define FRNFX 0b00000001

#define FRN8 0b10000000
#define FRN9 0b01000000
#define FRN10 0b00100000
#define FRN11 0b00010000
#define FRN12 0b00001000
#define FRN13 0b00000100
#define FRN14 0b00000010

#define FRN15 0b10000000
#define FRN16 0b01000000
#define FRN17 0b00100000
#define FRN18 0b00010000
#define FRN19 0b00001000
#define FRN20 0b00000100
#define FRN21 0b00000010

#define FRN22 0b10000000
#define FRN23 0b01000000
#define FRN24 0b00100000
#define FRN25 0b00010000
#define FRN26 0b00001000
#define FRN27 0b00000100
#define FRN28 0b00000010

#define FRN29 0b10000000
#define FRN30 0b01000000
#define FRN31 0b00100000
#define FRN32 0b00010000
#define FRN33 0b00001000
#define FRN34 0b00000100
#define FRN35 0b00000010

#define FRN36 0b10000000
#define FRN37 0b01000000
#define FRN38 0b00100000
#define FRN39 0b00010000
#define FRN40 0b00001000
#define FRN41 0b00000100
#define FRN42 0b00000010

bool bMODERN = true;
int index = 0;
int sac = 204;
int sic = 16;

bool isModernCAT21()
{
    return bMODERN;
}

void SetModernCAT21(bool b)
{
    bMODERN = b;
}
void SetSIC_SAC(int _sic, int _sac)
{
    sac = _sac;
    sic = _sic;
}

int GetSIC()
{
    return sic;
}

int GetSAC()
{
    return sac;
}

void InsertSAC_SIC_DI010()
{
    _MSG[index++] = (byte)sac;
    _MSG[index++] = (byte)sic;
}

void InsertEmitterCategoryDI020(int EC)
{
    /*
    0 = No ADS - B Emitter Category Information
    1 = light aircraft <= 15500 lbs
    2 = 15500 lbs < small aircraft < 75000 lbs
    3 = 75000 lbs < medium a / c < 300000 lbs
    4 = High Vortex Large
    5 = 300000 lbs <= heavy aircraft
    6 = highly manoeuvrable (5g acceleration capability) and high speed (>400 knots cruise)
    7 to 9 = reserved
    10 = rotocraft
    11 = glider / sailplane
    12 = lighter-than-air
    13 = unmanned aerial vehicle
    14 = space / transatmospheric vehicle
    15 = ultralight / handglider / paraglider
    16 = parachutist / skydiver
    17 to 19 = reserved
    20 = surface emergency vehicle
    21 = surface service vehicle
    22 = fixed ground or tethered obstruction
    23 = cluster obstacle
    24 = line obstacle
    */
    _MSG[index++] = EC;
}






//V0.23/26 DI
void InsertTimeOfDayDI030()
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    int elapsed = (int)st.wHour * 3600 + (int)st.wMinute * 60 + (int)st.wSecond;

    int TOD_16 = (int)(elapsed * 128);
    _MSG[index++] = (byte)(TOD_16 / (256 * 256));
    _MSG[index++] = (byte)(TOD_16 / (256));
    _MSG[index++] = (byte)(TOD_16 - (_MSG[index - 2] * 256 * 256) - (_MSG[index - 1] * 256));
}

//THIS is HARD CODED and matches messages recorded from Medium Range Radar (CA Army)
void InsertTargetReportDescriptorDI040()
{
    _MSG[index++] = 0x01;
    _MSG[index++] = 0x00;
}

void InsertMode3ADI070(int Mode3)//optional field for modern and old CAT 21
{
    _MSG[index++] = (byte)(Mode3 / 256);
    _MSG[index++] = (byte)(Mode3 - _MSG[index - 1] * 256);
}

void InsertTimeOfMsgReceptionDI073()
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    int elapsed = (int)st.wHour * 3600 + (int)st.wMinute * 60 + (int)st.wSecond;

    int TOD_16 = (int)(elapsed * 128);
    _MSG[index++] = (byte)(TOD_16 / (256 * 256));
    _MSG[index++] = (byte)(TOD_16 / (256));
    _MSG[index++] = (byte)(TOD_16 - (_MSG[index - 2] * 256 * 256) - (_MSG[index - 1] * 256));
}

void InsertTimeOfMsgReceptionDI075()
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    int elapsed = (int)st.wHour * 3600 + (int)st.wMinute * 60 + (int)st.wSecond;

    int TOD_16 = (int)(elapsed * 128);
    _MSG[index++] = (byte)(TOD_16 / (256 * 256));
    _MSG[index++] = (byte)(TOD_16 / (256));
    _MSG[index++] = (byte)(TOD_16 - (_MSG[index - 2] * 256 * 256) - (_MSG[index - 1] * 256));
}

void InsertAC_ICAO_Address_DI80(int ICAO)
{
    //Tgt Address  24 bit ICAO
    _MSG[index++] = (byte)((ICAO & 0xFF0000) >> 16);
    _MSG[index++] = (byte)((ICAO & 0x00FF00) >> 8);
    _MSG[index++] = (byte)((ICAO & 0x0000FF));
}

//V0.23/26 DI
void InsertFOM_DI090()
//quality indicators
//Definition : ADS - B quality indicators transmitted by a / c according to MOPS version.
// Format : Variable Length Data Item, comprising a primary subfield of oneoctet, followed by one - octet extensions as necessary.

{
    _MSG[index++] = 0b10100000;
    _MSG[index++] = 0b00001000;

}

void InsertPositionDI130_3BytePos(double latitude, double longitude)
//Position in WGS-84 Co-ordinates
{
    int Lat = (int)(latitude / 0.0000214576721191406);  //this number is 180/2^23
    _MSG[index++] = (byte)((Lat & 0xFF0000) >> 16);
    _MSG[index++] = (byte)((Lat & 0x00FF00) >> 8);
    _MSG[index++] = (byte)((Lat & 0x0000FF));

    int Lon = (int)(longitude / 0.0000214576721191406);
    _MSG[index++] = (byte)((Lon & 0xFF0000) >> 16);
    _MSG[index++] = (byte)((Lon & 0x00FF00) >> 8);
    _MSG[index++] = (byte)((Lon & 0x0000FF));
}

void InsertPositionDI130_4BytePos(double latitude, double longitude)
//High-Resolution Position in WGS-84 Co-ordinates
{
    int Lat = (int)(latitude / 0.00000536441802978515625); //this number is 180/2^25
    _MSG[index++] = (byte)((Lat & 0xFF000000) >> 24);
    _MSG[index++] = (byte)((Lat & 0x00FF0000) >> 16);
    _MSG[index++] = (byte)((Lat & 0x0000FF00) >> 8);
    _MSG[index++] = (byte)((Lat & 0x000000FF));



    int Lon = (int)(longitude / 0.00000536441802978515625);
    //byte val = (byte)((Lon & 0xFF000000) >> 24);
    //if (longitude > 0) val |= 0x10000000;
    //_MSG[index++] = val;

    _MSG[index++] = (byte)((Lon & 0xFF000000) >> 24);
    _MSG[index++] = (byte)((Lon & 0x00FF0000) >> 16);
    _MSG[index++] = (byte)((Lon & 0x0000FF00) >> 8);
    _MSG[index++] = (byte)((Lon & 0x000000FF));
}

void InsertGeoHeightDI140(int alt)
//Minimum height from a plane tangent to the earth’s ellipsoid, defined by WGS - 84, in two’s complement form.
//lsb is 6.25 feet
{
    int gcount = (int)(alt / 6.25);
    byte msb = (byte)(gcount / 256);
    byte lsb = gcount % 256;
    _MSG[index++] = msb;
    _MSG[index++] = lsb;
}

void InsertFLDI145(double alt_baro)
//Flight Level from barometric measurements, not QNH corrected,in two’s complement form.
//lsb is 1/4 FL (25 feet)
{
    int gcount = alt_baro / 25;
    byte msb = (byte)(gcount / 256);
    byte lsb = gcount % 256;
    _MSG[index++] = msb;
    _MSG[index++] = lsb;
}


void InsertTASDI151(int tas)
//Definition : True Air Speed.
//Format : Two - Octet fixed length data item
//lsb is 1 knot
{
    byte msb = (byte)(tas / 256);
    byte lsb = tas % 256;
    _MSG[index++] = msb;
    _MSG[index++] = lsb;
}

void InsertMagHdgDI152(double mag_heading)
//Definition : Magnetic Heading(Element of Air Vector).
//Format : Two - Octet fixed length data item.
//lsb = 360° / 216 (approx. 0.0055°)
{//VERIFY THIS
    byte msb = (byte)(mag_heading / 256);
    byte lsb = (int)(mag_heading) % 256;
    _MSG[index++] = msb;
    _MSG[index++] = lsb;
}





void InsertBarometricVerticalRateDI155(double VR)
{
    double vr = VR / 6.25;
    byte msb = (byte)(vr / 256);
    byte lsb = (int)(vr) % 256;
    _MSG[index++] = msb;
    _MSG[index++] = lsb;
}

void InsertAirborneGroundVectorDI160(int gndSpdInKnots, int heading)
{
    int AGVSpeed = (int)(gndSpdInKnots / 0.22);
    _MSG[index++] = (byte)((AGVSpeed & 0xFF00) >> 8);
    _MSG[index++] = (byte)((AGVSpeed & 0x00FF));

    int angle = (int)(heading / 0.0055);
    _MSG[index++] = (byte)((angle & 0xFF00) >> 8); // / 256);
    _MSG[index++] = (byte)((angle & 0x00FF)); //  % 256);
}

void InsertTrackNumberDI161(int CAT21TrackNumber)
{
    _MSG[index++] = (byte)((CAT21TrackNumber & 0xFF00) >> 8);  //Track Number MSB
    _MSG[index++] = (byte)(CAT21TrackNumber & 0x00FF);         //Track Number LSB
}

byte ICAOencode(char c)
{
    if ((c >= 'A') && (c <= 'Z'))
    {
        return (byte)((c - 'A') + 1);
    }
    else if (c == ' ')
    {
        return 0x20;
    }
    else //number 0 thru 9
    {
        return (byte)c;
    }
}

void Insert_ACIdent_DI170(char* ACIdent)
{
    byte c[8];

    for (int x = 0; x < 8; x++) c[x] = ICAOencode(ACIdent[x]);

    _MSG[index] = (byte)((c[0] & 0x3F) << 2);
    _MSG[index] = (byte)(_MSG[index] | (byte)((c[1] & 0x30) >> 4));
    index++;

    _MSG[index] = (byte)((c[1] & 0x0F) << 4);
    _MSG[index] = (byte)(_MSG[index] | (byte)((c[2] & 0x3C) >> 2));
    index++;

    _MSG[index] = (byte)((c[2] & 0x03) << 6);
    _MSG[index] = (byte)(_MSG[index] | c[3]);
    index++;

    _MSG[index] = (byte)((c[4] & 0x3F) << 2);
    _MSG[index] = (byte)(_MSG[index] | (c[5] & 0x30) >> 4);
    index++;

    _MSG[index] = (byte)((c[5] & 0x0F) << 4);
    _MSG[index] = (byte)(_MSG[index] | (c[6] & 0x3C) >> 2);
    index++;

    _MSG[index] = (byte)((c[6] & 0x03) << 6);
    _MSG[index] = (byte)(_MSG[index] | (c[7] & 0x3F));
    index++;

}

//HARD CODED
void InsertTargetStatusDI200(unsigned char status)
{
    _MSG[index++] = status;
}

//HARD CODED
void InsertLinkTechnologyDI210()
{
    _MSG[index++] = 0b00000000;
}

void UpdateMsgLength(int _Length)
{
    int msb = _Length / 256;
    int lsb = _Length - (msb * 256);
    _MSG[1] = (byte)msb;
    _MSG[2] = (byte)lsb;
}





ByteBuffer BuildCAT21FromAicraft_ADSB(Aircraft_SBS* ADSB_AC)
{
    ByteBuffer bb;
    char buf[300];
    SYSTEMTIME st;
    int MsgLength = 0;

    if (bMODERN)
    {
        MsgLength = CreateCAT21Modern(ADSB_AC);
        if (g_debug) {
            GetSystemTime(&st);
            sprintf_s(buf, "Sending CAT21 V2+ at time %02d:%02d:%02d.%d\r\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
            puts(buf);
        }
    }
    else //SITAWARE
    {
        MsgLength = CreateCAT21SITAWARE(ADSB_AC);
        if (g_debug) {
            GetSystemTime(&st);
            sprintf_s(buf, "Sending CAT21 V0.26 at time %02d:%02d:%02d.%d\r\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
            puts(buf);
        }
    }
    bb.buffer = (char*)_MSG;
    bb.bufLength = MsgLength;

    return bb;
}


int BuildCAT21FromADSB(Aircraft_SBS* ADSB_AC)
{
    char buf[300];
    SYSTEMTIME st;
    int MsgLength = 0;

    if (bMODERN)
    {
        MsgLength = CreateCAT21Modern(ADSB_AC);
        if (g_debug) {
            GetSystemTime(&st);
            sprintf_s(buf, "Sending CAT21 V2+ at time %02d:%02d:%02d.%d\r\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
            puts(buf);
        }
    }
    else //SITAWARE
    {
        MsgLength = CreateCAT21SITAWARE(ADSB_AC);
        if (g_debug) {
            GetSystemTime(&st);
            sprintf_s(buf, "Sending CAT21 V0.26 at time %02d:%02d:%02d.%d\r\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
            puts(buf);
        }
    }

    return MsgLength;
}

int CreateCAT21SITAWARE(Aircraft_SBS* ADSB_AC)
{
    std::vector<int> FRNsToSend;      //this is empty unlesss populated outside of this file by settings in the UI or config file
    FRNsToSend.clear();

    //this CAT21 is compliant to Sitaware
    FRNsToSend.push_back(1);  //DI010 Data Src ID
    FRNsToSend.push_back(2);  //DI040  Tgt Report Descriptor
    FRNsToSend.push_back(3);  //DI030 TOD
    
    if (ADSB_AC->TTCounts[2] > 0) FRNsToSend.push_back(4);  //DI 130  Pos WGS84
    if (ADSB_AC->TTCounts[3] > 0) FRNsToSend.push_back(4);  //DI 130  Pos WGS84

    FRNsToSend.push_back(5);  //DI080 ICAO/Hex/ Tgt Address
    if (ADSB_AC->TTCounts[3] > 0) FRNsToSend.push_back(6);  //DI 140 Geom Alt
    
    FRNsToSend.push_back(7);  //DI090 FOM
    FRNsToSend.push_back(8);  //DI210 Link Technology
    if (ADSB_AC->TTCounts[3] > 0) FRNsToSend.push_back(10); //DI 145 FL

    if (ADSB_AC->TTCounts[2] > 0) FRNsToSend.push_back(16);  //DI 160 Ground Vector
    if (ADSB_AC->TTCounts[4] > 0) FRNsToSend.push_back(16);  //DI 160 Ground Vector
    
    if (ADSB_AC->TTCounts[1] > 0) FRNsToSend.push_back(18); //DI170 Target ID/CS/Flight
    FRNsToSend.push_back(21); //DI200 Tgt Status
    FRNsToSend.push_back(22); //DO020 Emitter Category
    if (ADSB_AC->TTCounts[6] > 0) FRNsToSend.push_back(27); //{"FRN27: I021/070 Mode 3/A",MSG_STATE::OPT},


    //build the FSPEC (next ~75 lines)
    int FSPEC[6] = { 0,0,0,0,0,0 };

    unsigned char m_FRN[50];
    //builds a bit map lookup table for each of the FRNs. ASX CAT 21 supports 49 FRNs: 1 thru 49
    for (int x = 1; x <= 49; x++) m_FRN[x] = 0b10000000 >> ((x - 1) % 7);

    for (int frn : FRNsToSend)
    {
        int _index = (frn - 1) / 7;
        FSPEC[_index] |= m_FRN[frn];
    }

    //add the extension bit if required
    if (FSPEC[5] != 0)
    {
        FSPEC[4] |= FRNFX;
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[4] != 0)
    {
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[3] != 0)
    {
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[2] != 0)
    {
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[1] != 0)
    {
        FSPEC[0] |= FRNFX;
    }


    //Start building the ASTERIX Message
    index = 0; //reset the index
    _MSG[index++] = 21;     //ASTERIX CAT 21
    _MSG[index++] = 0;      //LEN MSB    these are placeholders that will be filled in later
    _MSG[index++] = 0;      //LEN LSB


    _MSG[index++] = FSPEC[0];
    if (FSPEC[0] & FRNFX) _MSG[index++] = FSPEC[1];
    if (FSPEC[1] & FRNFX) _MSG[index++] = FSPEC[2];
    if (FSPEC[2] & FRNFX) _MSG[index++] = FSPEC[3];
    if (FSPEC[3] & FRNFX) _MSG[index++] = FSPEC[4]; //this will never happen with this implementation. FSPEC[4] only contains RE and SP

    InsertSAC_SIC_DI010(); //FRN 1 Mandatory
    InsertTargetReportDescriptorDI040(); //FRN 2 M
    InsertTimeOfDayDI030(); //FRN 3 M
    
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 4) != FRNsToSend.end())
        InsertPositionDI130_4BytePos(ADSB_AC->Lat, ADSB_AC->Lon);   //FRN 4 M
    
    InsertAC_ICAO_Address_DI80(ADSB_AC->ICAO_i);  //FRN 5 M

    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 6) != FRNsToSend.end())
        InsertGeoHeightDI140(ADSB_AC->Altitude); // FRN 6 Optional

    InsertFOM_DI090();   //FRN 7 M
    InsertLinkTechnologyDI210();    // FRN 8 M


    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 10) != FRNsToSend.end())
        InsertFLDI145(ADSB_AC->Altitude);// FRN 10 Optional
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 16) != FRNsToSend.end())
        InsertAirborneGroundVectorDI160(ADSB_AC->GS, ADSB_AC->Trk);// FRN 16 Optional  ias and true heading may be wrong

    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 18) != FRNsToSend.end()) 
        Insert_ACIdent_DI170(ADSB_AC->CS); //Ident/flight/CS

    InsertTargetStatusDI200(0); // FRN 21 M
    
    InsertEmitterCategoryDI020(0); //FRN 22
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 27) != FRNsToSend.end())
        InsertMode3ADI070(ADSB_AC->Squawk_i);
    
    UpdateMsgLength(index);
    return index;
}






int CreateCAT21Modern(Aircraft_SBS* ADSB_AC) //this is compliant to V2.X
{
    std::vector<int> FRNsToSend;      //this is empty unlesss populated outside of this file by settings in the UI or config file
    FRNsToSend.clear();

    //this CAT21 is compliant to V0.26
    FRNsToSend.push_back(1);  //{"FRN01: I021/010 Data Source Identification", MSG_STATE::MANDATORY},
    FRNsToSend.push_back(2);  //{"FRN02: I021/040 Target Report Descriptor", MSG_STATE::MANDATORY},
    FRNsToSend.push_back(3);  //{"FRN03: I021/161 Track Number",MSG_STATE::MANDATORY},
    FRNsToSend.push_back(6);  //{"FRN06: I021/130 Position in WGS84 Coords", MSG_STATE::MANDATORY },  //3 byte version
    FRNsToSend.push_back(11); //{"FRN11: I021/080 Target Address",MSG_STATE::MANDATORY},
    FRNsToSend.push_back(12); //{"FRN12: I021/073 Time of Msg Reception of Position",MSG_STATE::MANDATORY},
    FRNsToSend.push_back(14); //{"FRN14: I021/075 Time of Msg Reception of Velocity",MSG_STATE::MANDATORY},
    //FRNsToSend.push_back(16); //{"FRN16: I021/140 Geometric Altitude",MSG_STATE::MANDATORY},
    if (ADSB_AC->TTCounts[4] > 0) FRNsToSend.push_back(24); //{"FRN24: I021/155 Baro Verical Rate",MSG_STATE::MANDATORY},
    FRNsToSend.push_back(26); //{"FRN26: I021/160 Ground Vector",MSG_STATE::MANDATORY},
    if (ADSB_AC->TTCounts[1] > 0) 
        FRNsToSend.push_back(29); //{"FRN29: I021/170 Target Identification",MSG_STATE::MANDATORY},

    FRNsToSend.push_back(30); //{"FRN30: I021/020 Emitter Catagory",MSG_STATE::MANDATORY},

    if (ADSB_AC->TTCounts[6] > 0) FRNsToSend.push_back(19); //{"FRN19: I021/070 Mode 3/A",MSG_STATE::OPT},
    if (ADSB_AC->TTCounts[3] > 0) FRNsToSend.push_back(21); //{"FRN21: I021/145 Flight Level",MSG_STATE::OPT},
    if (ADSB_AC->TTCounts[5] > 0) FRNsToSend.push_back(21); //{"FRN21: I021/145 Flight Level",MSG_STATE::OPT},
    if (ADSB_AC->TTCounts[6] > 0) FRNsToSend.push_back(21); //{"FRN21: I021/145 Flight Level",MSG_STATE::OPT},
    //if (ADSB_AC->TTCounts[2] > 0) FRNsToSend.push_back(22); //{"FRN22: I021/152 Magnetic Heading",MSG_STATE::OPT},
    //if (ADSB_AC->TTCounts[4] > 0) FRNsToSend.push_back(22); //{"FRN22: I021/152 Magnetic Heading",MSG_STATE::OPT},

    //build the FSPEC (next ~75 lines)
    int FSPEC[7] = { 0,0,0,0,0,0,0 };

    unsigned char m_FRN[50];
    //builds a bit map lookup table for each of the FRNs. ASX CAT 21 supports 49 FRNs: 1 thru 49
    for (int x = 1; x <= 49; x++) m_FRN[x] = 0b10000000 >> ((x - 1) % 7);

    for (int frn : FRNsToSend)
    {
        int _index = (frn - 1) / 7;

        FSPEC[_index] |= m_FRN[frn];
        //printf("Adding FRN %d  index: %d, FRN Bits:%X\r\n", frn, _index, m_FRN[frn]);
    }


    //set the extension bits if required
    if (FSPEC[6] != 0)//there is at least 1 FRN in the 7th word (index is 6 because zero indexing)
    {
        FSPEC[5] |= FRNFX;
        FSPEC[4] |= FRNFX;
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[5] != 0)
    {
        FSPEC[4] |= FRNFX;
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[4] != 0)
    {
        FSPEC[3] |= FRNFX;
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[3] != 0)
    {
        FSPEC[2] |= FRNFX;
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[2] != 0)
    {
        FSPEC[1] |= FRNFX;
        FSPEC[0] |= FRNFX;
    }
    else if (FSPEC[1] != 0)
    {
        FSPEC[0] |= FRNFX;
    }

    //Start building the ASTERIX Message
    index = 0; //reset the index
    _MSG[index++] = 21;     //ASTERIX CAT 21
    _MSG[index++] = 0;      //LEN MSB    these are placeholders that will be filled in later
    _MSG[index++] = 0;      //LEN LSB

    //Insert the FSPEC into the Message
    _MSG[index++] = FSPEC[0];
    if (FSPEC[0] & FRNFX) _MSG[index++] = FSPEC[1];
    if (FSPEC[1] & FRNFX) _MSG[index++] = FSPEC[2];
    if (FSPEC[2] & FRNFX) _MSG[index++] = FSPEC[3];
    if (FSPEC[3] & FRNFX) _MSG[index++] = FSPEC[4];
    if (FSPEC[4] & FRNFX) _MSG[index++] = FSPEC[5];
    if (FSPEC[5] & FRNFX) _MSG[index++] = FSPEC[6];

    //FRN1
    InsertSAC_SIC_DI010();

    //FRN2
    InsertTargetReportDescriptorDI040();

    //FRN3
    InsertTrackNumberDI161(ADSB_AC->TrkNum);

    //FRN6
    InsertPositionDI130_3BytePos(ADSB_AC->Lat, ADSB_AC->Lon);

    //FRN9: Air Speed (so is this IAS?)
    //FRN10: True Air Speed
    //FRN11
    InsertAC_ICAO_Address_DI80(ADSB_AC->ICAO_i);

    //FRN12
    InsertTimeOfMsgReceptionDI073();//073 Time of MSG Reception for Position

    //FRN13:

    //FRN14
    InsertTimeOfMsgReceptionDI075();//075 Time of MSG Reception for Velocity

    //FRN15:

    //FRN16
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 16) != FRNsToSend.end())
        InsertGeoHeightDI140(ADSB_AC->Altitude);

    //FRN17:
    //FRN18:

    //FRN19
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 19) != FRNsToSend.end())
        InsertMode3ADI070(ADSB_AC->Squawk_i);

    //FRN20: Roll Angle

    //FRN21
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 21) != FRNsToSend.end())
        InsertFLDI145(ADSB_AC->Altitude);

    //FRN 22
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 22) != FRNsToSend.end())
        InsertMagHdgDI152(ADSB_AC->Trk);

    //FRN23:


    //FRN24
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 24) != FRNsToSend.end())
        InsertBarometricVerticalRateDI155(ADSB_AC->VerticalRate);

    //FRN25:Geo Vertical Rate

    //FRN26
    InsertAirborneGroundVectorDI160(ADSB_AC->GS, ADSB_AC->Trk); // ias and true heading may be wrong

    //FRN27
    //FRN28


    //FRN29
    if (std::find(FRNsToSend.begin(), FRNsToSend.end(), 29) != FRNsToSend.end())
        Insert_ACIdent_DI170(ADSB_AC->CS); //was Ident

    //FRN30
    InsertEmitterCategoryDI020(0);//ADSB_AC->EmitterCatagory);

    UpdateMsgLength(index);
    return index;
}



