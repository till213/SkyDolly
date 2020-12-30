#ifndef SIMCONNECTAIRCRAFTINFO_H
#define SIMCONNECTAIRCRAFTINFO_H

struct SimConnectAircraftInfo
{
    char title[256];

    static void addToDataDefinition(HANDLE simConnectHandle);
};

#endif // SIMCONNECTAIRCRAFTINFO_H
