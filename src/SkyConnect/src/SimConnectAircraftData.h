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

    // Aircraft controls
    double yokeXPosition;
    double yokeYPosition;
    double rudderPosition;
    double elevatorPosition;
    double aileronPosition;    

    // General engine
    double throttleLeverPosition1;
    double throttleLeverPosition2;
    double throttleLeverPosition3;
    double throttleLeverPosition4;
    double propellerLeverPosition1;
    double propellerLeverPosition2;
    double propellerLeverPosition3;
    double propellerLeverPosition4;
    double mixtureLeverPosition1;
    double mixtureLeverPosition2;
    double mixtureLeverPosition3;
    double mixtureLeverPosition4;

    // Flaps & speed brakes
    double leadingEdgeFlapsLeftPercent;
    double leadingEdgeFlapsRightPercent;
    double trailingEdgeFlapsLeftPercent;
    double trailingEdgeFlapsRightPercent;
    double spoilersHandlePosition;
    qint32 flapsHandleIndex;

    // Gear, brakes & handles
    qint32 gearHandlePosition;
    double brakeLeftPosition;
    double brakeRightPosition;
    double waterRudderHandlePosition;
    double tailhookPosition;
    double canopyOpen;

    // Lights
    qint64 lightStates;

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

        aircraftData.yokeXPosition = SkyMath::fromPosition(yokeXPosition);
        aircraftData.yokeYPosition = SkyMath::fromPosition(yokeYPosition);
        aircraftData.rudderPosition = SkyMath::fromPosition(rudderPosition);
        aircraftData.elevatorPosition = SkyMath::fromPosition(elevatorPosition);
        aircraftData.aileronPosition = SkyMath::fromPosition(aileronPosition);

        aircraftData.leadingEdgeFlapsLeftPercent = SkyMath::fromPercent(leadingEdgeFlapsLeftPercent);
        aircraftData.leadingEdgeFlapsRightPercent = SkyMath::fromPercent(leadingEdgeFlapsRightPercent);
        aircraftData.trailingEdgeFlapsLeftPercent = SkyMath::fromPercent(trailingEdgeFlapsLeftPercent);
        aircraftData.trailingEdgeFlapsRightPercent = SkyMath::fromPercent(trailingEdgeFlapsRightPercent);
        aircraftData.spoilersHandlePosition = SkyMath::fromPercent(spoilersHandlePosition);
        aircraftData.flapsHandleIndex = flapsHandleIndex;

        aircraftData.gearHandlePosition = gearHandlePosition != 0;
        aircraftData.brakeLeftPosition= SkyMath::fromPosition(brakeLeftPosition);
        aircraftData.brakeRightPosition = SkyMath::fromPosition(brakeRightPosition);
        aircraftData.waterRudderHandlePosition = SkyMath::fromPosition(waterRudderHandlePosition);
        aircraftData.tailhookPosition = SkyMath::fromPercent(tailhookPosition);
        aircraftData.canopyOpen = SkyMath::fromPercent(canopyOpen);

        // Implementation note: downcast from 64 to 32 bit (QFlags only supports 'int')
        // This should be sufficient for now, unless there will be more than 32 distinct
        // lights
        aircraftData.lightStates = SimType::LightStates(static_cast<int>(lightStates));

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

        yokeXPosition = SkyMath::toPosition(aircraftData.yokeXPosition);
        yokeYPosition = SkyMath::toPosition(aircraftData.yokeYPosition);
        rudderPosition = SkyMath::toPosition(aircraftData.rudderPosition);
        elevatorPosition = SkyMath::toPosition(aircraftData.elevatorPosition);
        aileronPosition = SkyMath::toPosition(aircraftData.aileronPosition);

        leadingEdgeFlapsLeftPercent = SkyMath::toPercent(aircraftData.leadingEdgeFlapsLeftPercent);
        leadingEdgeFlapsRightPercent = SkyMath::toPercent(aircraftData.leadingEdgeFlapsRightPercent);
        trailingEdgeFlapsLeftPercent = SkyMath::toPercent(aircraftData.trailingEdgeFlapsLeftPercent);
        trailingEdgeFlapsRightPercent = SkyMath::toPercent(aircraftData.trailingEdgeFlapsRightPercent);
        spoilersHandlePosition = SkyMath::toPercent(aircraftData.spoilersHandlePosition);
        flapsHandleIndex = aircraftData.flapsHandleIndex;

        gearHandlePosition = aircraftData.gearHandlePosition ? 1 : 0;
        brakeLeftPosition = SkyMath::toPosition(aircraftData.brakeLeftPosition);
        brakeRightPosition = SkyMath::toPosition(aircraftData.brakeRightPosition);
        waterRudderHandlePosition = SkyMath::toPosition(aircraftData.waterRudderHandlePosition);
        tailhookPosition = SkyMath::toPercent(aircraftData.tailhookPosition);
        canopyOpen = SkyMath::toPercent(aircraftData.canopyOpen);

        lightStates = aircraftData.lightStates;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};

#endif // SIMCONNECTAIRCRAFTDATA_H
