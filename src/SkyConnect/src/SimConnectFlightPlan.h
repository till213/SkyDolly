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
#ifndef SIMCONNECTFLIGHTPLAN_H
#define SIMCONNECTFLIGHTPLAN_H

#include <windows.h>
#include <strsafe.h>

#include <SimConnect.h>

#include "../../Model/src/FlightPlanData.h"

/*!
 * Simulation variables which represent aircraft lights, e.g. navigation light
 * and taxi light.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectFlightPlan
{
    char gpsWpNextId[8];
    char gpsWpPrevId[8];
    float gpsWpNextLat;
    float gpsWpNextLon;
    float gpsWpNextAlt;
    float gpsWpPrevLat;
    float gpsWpPrevLon;
    float gpsWpPrevAlt;

    inline FlightPlanData toNextFlightPlanData() const noexcept
    {
        FlightPlanData flightPlanData;

        // Length check
        if (SUCCEEDED(StringCbLengthA(&gpsWpNextId[0], sizeof(gpsWpNextId), nullptr))) {
            flightPlanData.waypointIdentifier = QString(gpsWpNextId);
        }
        flightPlanData.latitude = gpsWpNextLat;
        flightPlanData.longitude = gpsWpNextLon;
        flightPlanData.altitude = gpsWpNextAlt;

        return flightPlanData;
    }

    inline FlightPlanData toPreviousFlightPlanData() const noexcept
    {
        FlightPlanData flightPlanData;

        // Length check
        if (SUCCEEDED(StringCbLengthA(&gpsWpPrevId[0], sizeof(gpsWpPrevId), nullptr))) {
            flightPlanData.waypointIdentifier = QString(gpsWpPrevId);
        }
        flightPlanData.latitude = gpsWpPrevLat;
        flightPlanData.longitude = gpsWpPrevLon;
        flightPlanData.altitude = gpsWpPrevAlt;

        return flightPlanData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTFLIGHTPLAN_H
