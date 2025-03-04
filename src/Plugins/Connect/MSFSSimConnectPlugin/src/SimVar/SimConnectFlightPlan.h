/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

#include <SimConnect.h>

#include <Model/Waypoint.h>

// Include this last, as other Qt related headers will generate errors (unless STRSAFE_NO_DEPRECATE is defined)
#include <strsafe.h>

/*!
 * Simulation variables that represent aircraft lights, e.g. navigation light
 * and taxi light.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectFlightPlan
{
    char gpsWpNextId[8] {'\0'};
    char gpsWpPrevId[8] {'\0'};
    float gpsWpNextLat {0.0f};
    float gpsWpNextLon {0.0f};
    float gpsWpNextAlt {0.0f};
    float gpsWpPrevLat {0.0f};
    float gpsWpPrevLon {0.0f};
    float gpsWpPrevAlt {0.0f};

    inline Waypoint toNextWaypoint() const noexcept
    {
        Waypoint waypoint;

        // Length check
        if (SUCCEEDED(StringCbLengthA(&gpsWpNextId[0], sizeof(gpsWpNextId), nullptr))) {
            waypoint.identifier = QString::fromLatin1(gpsWpNextId);
        }
        waypoint.latitude = gpsWpNextLat;
        waypoint.longitude = gpsWpNextLon;
        waypoint.altitude = gpsWpNextAlt;

        return waypoint;
    }

    inline Waypoint toPreviousWaypoint() const noexcept
    {
        Waypoint waypoint;

        // Length check
        if (SUCCEEDED(StringCbLengthA(&gpsWpPrevId[0], sizeof(gpsWpPrevId), nullptr))) {
            waypoint.identifier = QString::fromLatin1(gpsWpPrevId);
        }
        waypoint.latitude = gpsWpPrevLat;
        waypoint.longitude = gpsWpPrevLon;
        waypoint.altitude = gpsWpPrevAlt;

        return waypoint;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTFLIGHTPLAN_H
