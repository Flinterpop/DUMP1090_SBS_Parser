#pragma once

#include <set>

struct Aircraft_SBS
{
    char ICAO[10];
    int ICAO_i;
    int TTCounts[9];

    char CS[10];
    int Altitude;
    double GS;
    double Trk;
    double Lat;
    double Lon;
    double VerticalRate;
    char Squawk[10];
    int Squawk_i;
    int AlertFlag;
    int EmergFlag;
    int SPIFlag;
    int GndFlag;

    int age = 0;
    int numMessages;
    std::set<int> TranTypesRx;
    int NewTrack = 5;

    int TrkNum;

};
