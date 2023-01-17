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
#ifndef SIMCONNECTAIRCRAFTHANDLEANIMATION_H
#define SIMCONNECTAIRCRAFTHANDLEANIMATION_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Model/SimVar.h>
#include "SimConnectType.h"
#include <Model/AircraftHandleData.h>

/*!
 * Aircraft handle simulation variables that are sent exclusively to AI aircraft for animation.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftHandleAnimation
{
    float foldingWingLeftPercent {0.0f};
    float foldingWingRightPercent {0.0f};

    SimConnectAircraftHandleAnimation(const AircraftHandleData &aircraftHandleData) noexcept
        : SimConnectAircraftHandleAnimation()
    {
        fromAircraftHandleData(aircraftHandleData);
    }

    SimConnectAircraftHandleAnimation() = default;

    inline void fromAircraftHandleData(const AircraftHandleData &aircraftHandleData) noexcept
    {
        foldingWingLeftPercent = static_cast<float>(SkyMath::toPercent(aircraftHandleData.leftWingFolding));
        foldingWingRightPercent = static_cast<float>(SkyMath::toPercent(aircraftHandleData.rightWingFolding));
    }

    inline AircraftHandleData toAircraftHandleData() const noexcept
    {
        AircraftHandleData aircraftHandleData;
        toAircraftHandleData(aircraftHandleData);
        return aircraftHandleData;
    }

    inline void toAircraftHandleData(AircraftHandleData &aircraftHandleData) const noexcept
    {
        aircraftHandleData.leftWingFolding = SkyMath::fromPercent(foldingWingLeftPercent);
        aircraftHandleData.rightWingFolding = SkyMath::fromPercent(foldingWingRightPercent);
    }

    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::FoldingWingLeftPercent, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::FoldingWingRightPercent, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTHANDLEANIMATION_H
