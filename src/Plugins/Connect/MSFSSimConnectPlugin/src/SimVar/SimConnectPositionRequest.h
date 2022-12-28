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
#ifndef SIMCONNECTPOSITIONREQUEST_H
#define SIMCONNECTPOSITIONREQUEST_H

#include <windows.h>
#include <SimConnect.h>

#include <Model/PositionData.h>
#include <Model/InitialPosition.h>

/*!
 * Simulation variables which represent the aircraft's position, attitude and velocities
 * (request sent to the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPositionRequest
{
    // Aircraft position
    double latitude {0.0};
    double longitude {0.0};
    double altitude {0.0};
    double pitch {0.0};
    double bank {0.0};
    double trueHeading {0.0};

    // Velocity
    double velocityBodyX {0.0};
    double velocityBodyY {0.0};
    double velocityBodyZ {0.0};
    double rotationVelocityBodyX {0.0};
    double rotationVelocityBodyY {0.0};
    double rotationVelocityBodyZ {0.0};

    inline void fromPositionData(const PositionData &positionData) noexcept
    {
        latitude = positionData.latitude;
        longitude = positionData.longitude;
        altitude = positionData.altitude;
        pitch = positionData.pitch;
        bank = positionData.bank;
        trueHeading = positionData.trueHeading;

        velocityBodyX = positionData.velocityBodyX;
        velocityBodyY = positionData.velocityBodyY;
        velocityBodyZ = positionData.velocityBodyZ;
        rotationVelocityBodyX = positionData.rotationVelocityBodyX;
        rotationVelocityBodyY = positionData.rotationVelocityBodyY;
        rotationVelocityBodyZ = positionData.rotationVelocityBodyZ;
    }

    static inline SIMCONNECT_DATA_INITPOSITION toInitialPosition(const PositionData &positionData, bool onGround, int initialAirspeed)
    {
        SIMCONNECT_DATA_INITPOSITION initialPosition;

        initialPosition.Latitude = positionData.latitude;
        initialPosition.Longitude = positionData.longitude;
        initialPosition.Altitude = positionData.altitude;
        initialPosition.Pitch = positionData.pitch;
        initialPosition.Bank = positionData.bank;
        initialPosition.Heading = positionData.trueHeading;
        initialPosition.OnGround = onGround ? 1 : 0;
        initialPosition.Airspeed = initialAirspeed;

        return initialPosition;
    }

    static inline SIMCONNECT_DATA_INITPOSITION toInitialPosition(const InitialPosition &initialPosition)
    {
        SIMCONNECT_DATA_INITPOSITION initialSimConnnectPosition;

        initialSimConnnectPosition.Latitude = initialPosition.latitude;
        initialSimConnnectPosition.Longitude = initialPosition.longitude;
        initialSimConnnectPosition.Altitude = initialPosition.altitude;
        initialSimConnnectPosition.Pitch = initialPosition.pitch;
        initialSimConnnectPosition.Bank = initialPosition.bank;
        initialSimConnnectPosition.Heading = initialPosition.trueHeading;
        initialSimConnnectPosition.OnGround = initialPosition.onGround ? 1 : 0;
        initialSimConnnectPosition.Airspeed = initialPosition.indicatedAirspeed;

        return initialSimConnnectPosition;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;

protected:
    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTPOSITIONREQUEST_H
