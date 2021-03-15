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
    char atcId[256];
    char atcAirline[256];
    char atcFlightNumber[256];
    char category[256];

    qint32 simOnGround;
    qint32 airspeedTrue;
    qint32 surfaceType;
    qint32 wingSpan;
    qint32 numberOfEngines;
    qint32 engineType;

    SimConnectAircraftInfo()
        : simOnGround(false),
          airspeedTrue(0),
          surfaceType(0),
          wingSpan(0),
          numberOfEngines(0),
          engineType(0)
    {}

    inline AircraftInfo toAircraftInfo() const {
        AircraftInfo aircraftInfo;

        // Length check
        if (SUCCEEDED(StringCbLengthA(&title[0], sizeof(title), nullptr))) {
            aircraftInfo.name = title;
        }
        if (SUCCEEDED(StringCbLengthA(&atcId[0], sizeof(atcId), nullptr))) {
            aircraftInfo.atcId = atcId;
        }
        if (SUCCEEDED(StringCbLengthA(&atcAirline[0], sizeof(atcAirline), nullptr))) {
            aircraftInfo.atcAirline = atcAirline;
        }
        if (SUCCEEDED(StringCbLengthA(&atcFlightNumber[0], sizeof(atcFlightNumber), nullptr))) {
            aircraftInfo.atcFlightNumber = atcFlightNumber;
        }
        if (SUCCEEDED(StringCbLengthA(&category[0], sizeof(category), nullptr))) {
            aircraftInfo.category = category;
        }

        aircraftInfo.startOnGround = (simOnGround != 0);
        aircraftInfo.initialAirspeed = airspeedTrue;
        aircraftInfo.surfaceType = toSurfaceType(surfaceType);
        aircraftInfo.wingSpan = wingSpan;
        aircraftInfo.numberOfEngines = numberOfEngines;
        aircraftInfo.engineType = toEngineType(engineType);

        return aircraftInfo;
    }

    static void addToDataDefinition(HANDLE simConnectHandle);

private:
    static inline SimTypes::SurfaceType toSurfaceType(qint32 surfaceType)
    {
        // TODO IMPLEMENT ME
        return SimTypes::SurfaceType::Asphalt;
    }
    static inline SimTypes::EngineType toEngineType(qint32 engineType)
    {
        // TODO IMPLEMENT ME
        return SimTypes::EngineType::Jet;
    }
};

#endif // SIMCONNECTAIRCRAFTINFO_H
