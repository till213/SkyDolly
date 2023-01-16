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
#ifndef SIMCONNECTAIRCRAFTHANDLEEVENT_H
#define SIMCONNECTAIRCRAFTHANDLEEVENT_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Model/SimVar.h>
#include "SimConnectType.h"
#include <Model/AircraftHandleData.h>

/*!
 * Aircraft handle simulation variables that are only sent to the user aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftHandleEvent
{
    float tailhookPosition {0.0f};
    // TODO Move to HandleAnimation
    float foldingWingLeftPercent {0.0f};
    float foldingWingRightPercent {0.0f};
    std::int32_t gearHandlePosition {0};
    std::int32_t smokeEnable {0};

    SimConnectAircraftHandleEvent(const AircraftHandleData &aircraftHandleData) noexcept
        : SimConnectAircraftHandleEvent()
    {
        fromAircraftHandleData(aircraftHandleData);
    }
    SimConnectAircraftHandleEvent() = default;

    inline void fromAircraftHandleData(const AircraftHandleData &aircraftHandleData) noexcept
    {
        tailhookPosition = static_cast<float>(SkyMath::toPercent(aircraftHandleData.tailhookPosition));
        foldingWingLeftPercent = static_cast<float>(SkyMath::toPercent(aircraftHandleData.leftWingFolding));
        foldingWingRightPercent = static_cast<float>(SkyMath::toPercent(aircraftHandleData.rightWingFolding));
        gearHandlePosition = aircraftHandleData.gearHandlePosition ? 1 : 0;
        smokeEnable = aircraftHandleData.smokeEnabled ? 1 : 0;
    }

    inline AircraftHandleData toAircraftHandleData() const noexcept
    {
        AircraftHandleData aircraftHandleData;
        toAircraftHandleData(aircraftHandleData);
        return aircraftHandleData;
    }

    inline void toAircraftHandleData(AircraftHandleData &aircraftHandleData) const noexcept
    {
        aircraftHandleData.tailhookPosition = SkyMath::fromPercent(tailhookPosition);
        aircraftHandleData.leftWingFolding = SkyMath::fromPercent(foldingWingLeftPercent);
        aircraftHandleData.rightWingFolding = SkyMath::fromPercent(foldingWingRightPercent);
        aircraftHandleData.gearHandlePosition = gearHandlePosition != 0;
        aircraftHandleData.smokeEnabled = smokeEnable != 0;
    }

    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::TailhookPosition, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::FoldingWingLeftPercent, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::FoldingWingRightPercent, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GearHandlePosition, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::SmokeEnable, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTHANDLEEVENT_H
