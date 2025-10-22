/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#ifndef SIMCONNECTAIRCRAFTHANDLECORE_H
#define SIMCONNECTAIRCRAFTHANDLECORE_H

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Model/SimVar.h>
#include "SimConnectType.h"
#include <Model/AircraftHandleData.h>

/*!
 * Core aircraft handle simulation variables that are only sent to the user aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftHandleCore
{
    float brakeLeftPosition {0.0f};
    float brakeRightPosition {0.0f};

    SimConnectAircraftHandleCore(const AircraftHandleData &aircraftHandleData) noexcept
        : SimConnectAircraftHandleCore()
    {
        fromAircraftHandleData(aircraftHandleData);
    }
    SimConnectAircraftHandleCore() = default;

    inline void fromAircraftHandleData(const AircraftHandleData &aircraftHandleData) noexcept
    {
        brakeLeftPosition = static_cast<float>(SkyMath::toNormalisedPosition(aircraftHandleData.brakeLeftPosition));
        brakeRightPosition = static_cast<float>(SkyMath::toNormalisedPosition(aircraftHandleData.brakeRightPosition));
    }

    inline AircraftHandleData toAircraftHandleData() const noexcept
    {
        AircraftHandleData aircraftHandleData;
        toAircraftHandleData(aircraftHandleData);
        return aircraftHandleData;
    }

    inline void toAircraftHandleData(AircraftHandleData &aircraftHandleData) const noexcept
    {
        aircraftHandleData.brakeLeftPosition= SkyMath::fromNormalisedPosition(brakeLeftPosition);
        aircraftHandleData.brakeRightPosition = SkyMath::fromNormalisedPosition(brakeRightPosition);
    }

    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::BrakeLeftPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::BrakeRightPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTHANDLECORE_H
