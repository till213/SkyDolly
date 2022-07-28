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
#ifndef SIMCONNECTPOSITIONREPLY_H
#define SIMCONNECTPOSITIONREPLY_H

#include <windows.h>
#include <SimConnect.h>

#include <Model/PositionData.h>
#include "SimConnectPositionRequest.h"

/*!
 * Simulation variables which represent the aircraft's position, attitude and velocities
 * (reply received from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPositionReply : public SimConnectPositionRequest
{
    // Extended aircraft position
    double indicatedAltitude {0.0};

    inline PositionData toPositionData() const noexcept
    {
        PositionData positionData;
        positionData.latitude = latitude;
        positionData.longitude = longitude;
        positionData.altitude = altitude;
        positionData.indicatedAltitude = indicatedAltitude;
        positionData.pitch = pitch;
        positionData.bank = bank;
        positionData.trueHeading = trueHeading;

        positionData.velocityBodyX = velocityBodyX;
        positionData.velocityBodyY = velocityBodyY;
        positionData.velocityBodyZ = velocityBodyZ;
        positionData.rotationVelocityBodyX = rotationVelocityBodyX;
        positionData.rotationVelocityBodyY = rotationVelocityBodyY;
        positionData.rotationVelocityBodyZ = rotationVelocityBodyZ;

        return positionData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;

};
#pragma pack(pop)

#endif // SIMCONNECTPOSITIONREPLY_H
