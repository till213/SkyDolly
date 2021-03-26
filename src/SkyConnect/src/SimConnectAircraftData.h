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
#ifndef SIMCONNECTAIRCRAFTDATA_H
#define SIMCONNECTAIRCRAFTDATA_H

#include <windows.h>

#include "../../Kernel/src/SkyMath.h"
#include "../../Model/src/SimType.h"
#include "../../Model/src/AircraftData.h"

/*!
 * Simulation variables which represent the aircraft's position, attitude and velocities.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(1)
struct SimConnectAircraftData
{
    // Aircraft position
    double latitude;
    double longitude;
    double altitude;
    double pitch;
    double bank;
    double heading;

    // Velocity
    double velocityBodyX;
    double velocityBodyY;
    double velocityBodyZ;
    double rotationVelocityBodyX;
    double rotationVelocityBodyY;
    double rotationVelocityBodyZ;

    // Gear, brakes & handles
    double brakeLeftPosition;
    double brakeRightPosition;
    double waterRudderHandlePosition;
    double tailhookPosition;
    double canopyOpen;
    qint32 gearHandlePosition;

    // Lights
    qint32 lightStates;

    inline AircraftData toAircraftData() const noexcept
    {
        AircraftData aircraftData;

        aircraftData.latitude = latitude;
        aircraftData.longitude = longitude;
        aircraftData.altitude = altitude;
        aircraftData.pitch = pitch;
        aircraftData.bank = bank;
        aircraftData.heading = heading;

        aircraftData.velocityBodyX = velocityBodyX;
        aircraftData.velocityBodyY = velocityBodyY;
        aircraftData.velocityBodyZ = velocityBodyZ;
        aircraftData.rotationVelocityBodyX = rotationVelocityBodyX;
        aircraftData.rotationVelocityBodyY = rotationVelocityBodyY;
        aircraftData.rotationVelocityBodyZ = rotationVelocityBodyZ;

        aircraftData.brakeLeftPosition= SkyMath::fromPosition(brakeLeftPosition);
        aircraftData.brakeRightPosition = SkyMath::fromPosition(brakeRightPosition);
        aircraftData.waterRudderHandlePosition = SkyMath::fromPosition(waterRudderHandlePosition);
        aircraftData.tailhookPosition = SkyMath::fromPercent(tailhookPosition);
        aircraftData.canopyOpen = SkyMath::fromPercent(canopyOpen);
        aircraftData.gearHandlePosition = gearHandlePosition != 0;

        aircraftData.lightStates = SimType::LightStates(lightStates);

        return aircraftData;
    }

    inline void fromAircraftData(const AircraftData &aircraftData) noexcept
    {
        latitude = aircraftData.latitude;
        longitude = aircraftData.longitude;
        altitude = aircraftData.altitude;
        pitch = aircraftData.pitch;
        bank = aircraftData.bank;
        heading = aircraftData.heading;

        velocityBodyX = aircraftData.velocityBodyX;
        velocityBodyY = aircraftData.velocityBodyY;
        velocityBodyZ = aircraftData.velocityBodyZ;
        rotationVelocityBodyX = aircraftData.rotationVelocityBodyX;
        rotationVelocityBodyY = aircraftData.rotationVelocityBodyY;
        rotationVelocityBodyZ = aircraftData.rotationVelocityBodyZ;

        brakeLeftPosition = SkyMath::toPosition(aircraftData.brakeLeftPosition);
        brakeRightPosition = SkyMath::toPosition(aircraftData.brakeRightPosition);
        waterRudderHandlePosition = SkyMath::toPosition(aircraftData.waterRudderHandlePosition);
        tailhookPosition = SkyMath::toPercent(aircraftData.tailhookPosition);
        canopyOpen = SkyMath::toPercent(aircraftData.canopyOpen);
        gearHandlePosition = aircraftData.gearHandlePosition ? 1 : 0;

        lightStates = aircraftData.lightStates;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
} __attribute__ ((packed));

#endif // SIMCONNECTAIRCRAFTDATA_H
