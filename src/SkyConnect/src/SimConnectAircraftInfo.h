#ifndef SIMCONNECTAIRCRAFTINFO_H
#define SIMCONNECTAIRCRAFTINFO_H

#include <windows.h>
#include <strsafe.h>

#include <QtGlobal>

#include "../../Kernel/src/AircraftInfo.h"

struct SimConnectAircraftInfo
{
    char title[256];
    qint32 simOnGround;
    qint32 airspeedTrue;

    SimConnectAircraftInfo()
        : simOnGround(false),
          airspeedTrue(0)
    {}

    inline AircraftInfo toAircraftInfo() const {
        AircraftInfo aircraftInfo;

        // security check
        if (SUCCEEDED(StringCbLengthA(&title[0], sizeof(title), nullptr))) {
            aircraftInfo.name = title;
        }
        aircraftInfo.startOnGround = (simOnGround != 0);
        aircraftInfo.initialAirspeed = airspeedTrue;

        return aircraftInfo;
    }

    static void addToDataDefinition(HANDLE simConnectHandle);
};

#endif // SIMCONNECTAIRCRAFTINFO_H
