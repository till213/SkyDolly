/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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

#include <cstdint>

#include <windows.h>
#include <strsafe.h>

#include <Kernel/Const.h>
#include <Model/SimType.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/AircraftType.h>
#include <Model/FlightCondition.h>

/*!
 * Information simulation variables about the aircraft and environment at start.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftInfo
{
    // Aircraft info
    char title[256] {'\0'};
    char atcId[32] {'\0'};
    char atcAirline[64] {'\0'};
    char category[256] {'\0'};
    // Feet
    float planeAltAboveGround {0.0f};
    std::int32_t simOnGround {0};
    // Knots
    std::int32_t airspeedTrue {0};
    // Feet
    std::int32_t wingSpan {0};
    std::int32_t engineType {0};
    std::int32_t numberOfEngines {0};

    inline AircraftInfo toAircraftInfo() const noexcept
    {
        AircraftInfo aircraftInfo(Const::InvalidId);

        // Length check
        if (SUCCEEDED(StringCbLengthA(&title[0], sizeof(title), nullptr))) {
            aircraftInfo.aircraftType.type = QString(title);
        }
        aircraftInfo.aircraftType.wingSpan = wingSpan;
        aircraftInfo.aircraftType.engineType = toEngineType(engineType);
        aircraftInfo.aircraftType.numberOfEngines = numberOfEngines;

        if (SUCCEEDED(StringCbLengthA(&atcId[0], sizeof(atcId), nullptr))) {
            aircraftInfo.tailNumber = QString(atcId);
        }
        if (SUCCEEDED(StringCbLengthA(&atcAirline[0], sizeof(atcAirline), nullptr))) {
            aircraftInfo.airline = QString(atcAirline);
        }
        if (SUCCEEDED(StringCbLengthA(&category[0], sizeof(category), nullptr))) {
            aircraftInfo.aircraftType.category = QString(category);
        }

        aircraftInfo.initialAirspeed = airspeedTrue;
        aircraftInfo.altitudeAboveGround = planeAltAboveGround;
        aircraftInfo.startOnGround = (simOnGround != 0);

        return aircraftInfo;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;

private:
    static inline SimType::EngineType toEngineType(std::int32_t type) noexcept
    {
        SimType::EngineType engineType {SimType::EngineType::None};
        switch (type) {
        case 0:
            engineType = SimType::EngineType::Piston;
            break;
        case 1:
            engineType = SimType::EngineType::Jet;
            break;
        case 2:
            engineType = SimType::EngineType::None;
            break;
        case 3:
            engineType = SimType::EngineType::HeloBellTurbine;
            break;
        case 4:
            engineType = SimType::EngineType::Unsupported;
            break;
        case 5:
            engineType = SimType::EngineType::Turboprop;
            break;
        default:
            engineType = SimType::EngineType::Unknown;
            break;
        }
        return engineType;
    }
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTINFO_H
