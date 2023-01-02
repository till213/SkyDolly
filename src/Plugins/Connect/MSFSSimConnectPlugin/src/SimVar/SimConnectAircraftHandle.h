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
#ifndef SIMCONNECTAIRCRAFTHANDLE_H
#define SIMCONNECTAIRCRAFTHANDLE_H

#include <cstdint>

#include <windows.h>

#include <Kernel/SkyMath.h>
#include <Model/AircraftHandleData.h>

/*!
 * Simulation variables that represent aircraft handles and brakes.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftHandle
{
    // Gear, brakes & handles
    float brakeLeftPosition {0.0f};
    float brakeRightPosition {0.0f};
    float waterRudderHandlePosition {0.0f};
    float tailhookPosition {0.0f};
    float canopyOpen {0.0f};
    float foldingWingLeftPercent {0.0f};
    float foldingWingRightPercent {0.0f};
    std::int32_t gearHandlePosition {0};
    std::int32_t smokeEnable {0};

    inline AircraftHandleData toAircraftHandleData() const noexcept
    {
        AircraftHandleData aircraftHandleData;

        aircraftHandleData.brakeLeftPosition= SkyMath::fromNormalisedPosition(brakeLeftPosition);
        aircraftHandleData.brakeRightPosition = SkyMath::fromNormalisedPosition(brakeRightPosition);
        aircraftHandleData.waterRudderHandlePosition = SkyMath::fromNormalisedPosition(waterRudderHandlePosition);
        aircraftHandleData.tailhookPosition = SkyMath::fromPercent(tailhookPosition);
        aircraftHandleData.canopyOpen = SkyMath::fromPercent(canopyOpen);
        aircraftHandleData.leftWingFolding = SkyMath::fromPercent(foldingWingLeftPercent);
        aircraftHandleData.rightWingFolding = SkyMath::fromPercent(foldingWingRightPercent);
        aircraftHandleData.gearHandlePosition = gearHandlePosition != 0;
        aircraftHandleData.smokeEnabled = smokeEnable != 0;

        return aircraftHandleData;
    }

    inline void fromAircraftHandleData(const AircraftHandleData &aircraftHandleData) noexcept
    {
        brakeLeftPosition = static_cast<float>(SkyMath::toNormalisedPosition(aircraftHandleData.brakeLeftPosition));
        brakeRightPosition = static_cast<float>(SkyMath::toNormalisedPosition(aircraftHandleData.brakeRightPosition));
        waterRudderHandlePosition = static_cast<float>(SkyMath::toNormalisedPosition(aircraftHandleData.waterRudderHandlePosition));
        tailhookPosition = static_cast<float>(SkyMath::toPercent(aircraftHandleData.tailhookPosition));
        canopyOpen = static_cast<float>(SkyMath::toPercent(aircraftHandleData.canopyOpen));
        foldingWingLeftPercent = static_cast<float>(SkyMath::toPercent(aircraftHandleData.leftWingFolding));
        foldingWingRightPercent = static_cast<float>(SkyMath::toPercent(aircraftHandleData.rightWingFolding));
        gearHandlePosition = aircraftHandleData.gearHandlePosition ? 1 : 0;
        smokeEnable = aircraftHandleData.smokeEnabled ? 1 : 0;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTHANDLE_H
