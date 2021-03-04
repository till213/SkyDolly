/**
 * Sky Dolly - The black sheep for your flight recordings
 *
 * Copyright (c) Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
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
