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

#include <windows.h>

#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../../../Model/src/AircraftHandleData.h"

/*!
 * Simulation variables which represent aircraft handles and brakes.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftHandle
{
    // Gear, brakes & handles
    float brakeLeftPosition;
    float brakeRightPosition;
    float waterRudderHandlePosition;
    float tailhookPosition;
    float canopyOpen;
    float foldingWingLeftPercent;
    float foldingWingRightPercent;
    qint32 gearHandlePosition;
    qint32 smokeEnable;

    inline AircraftHandleData toAircraftHandleData() const noexcept
    {
        AircraftHandleData aircraftHandleData;

        aircraftHandleData.brakeLeftPosition= SkyMath::fromPosition(brakeLeftPosition);
        aircraftHandleData.brakeRightPosition = SkyMath::fromPosition(brakeRightPosition);
        aircraftHandleData.waterRudderHandlePosition = SkyMath::fromPosition(waterRudderHandlePosition);
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
        brakeLeftPosition = SkyMath::toPosition(aircraftHandleData.brakeLeftPosition);
        brakeRightPosition = SkyMath::toPosition(aircraftHandleData.brakeRightPosition);
        waterRudderHandlePosition = SkyMath::toPosition(aircraftHandleData.waterRudderHandlePosition);
        tailhookPosition = SkyMath::toPercent(aircraftHandleData.tailhookPosition);
        canopyOpen = SkyMath::toPercent(aircraftHandleData.canopyOpen);
        foldingWingLeftPercent = SkyMath::toPercent(aircraftHandleData.leftWingFolding);
        foldingWingRightPercent = SkyMath::toPercent(aircraftHandleData.rightWingFolding);
        gearHandlePosition = aircraftHandleData.gearHandlePosition ? 1 : 0;
        smokeEnable = aircraftHandleData.smokeEnabled ? 1 : 0;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTHANDLE_H
