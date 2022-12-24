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
#ifndef SIMCONNECTPRIMARYFLIGHTCONTROL_H
#define SIMCONNECTPRIMARYFLIGHTCONTROL_H

#include <windows.h>

#include <Kernel/SkyMath.h>
#include <Model/PrimaryFlightControlData.h>

/*!
 * Simulation variables which represent the primary flight controls: rudder, elevators and ailerons.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPrimaryFlightControl
{
    float rudderPosition {0.0f};
    float elevatorPosition {0.0f};
    float aileronPosition {0.0f};

    inline PrimaryFlightControlData toPrimaryFlightControlData() const noexcept
    {
        PrimaryFlightControlData primaryFlightControlData;

        primaryFlightControlData.rudderPosition = SkyMath::fromNormalisedPosition(rudderPosition);
        primaryFlightControlData.elevatorPosition = SkyMath::fromNormalisedPosition(elevatorPosition);
        primaryFlightControlData.aileronPosition = SkyMath::fromNormalisedPosition(aileronPosition);

        return primaryFlightControlData;
    }

    inline void fromPrimaryFlightControlData(const PrimaryFlightControlData &primaryFlightControlData) noexcept
    {
        rudderPosition = static_cast<float>(SkyMath::toNormalisedPosition(primaryFlightControlData.rudderPosition));
        elevatorPosition = static_cast<float>(SkyMath::toNormalisedPosition(primaryFlightControlData.elevatorPosition));
        aileronPosition = static_cast<float>(SkyMath::toNormalisedPosition(primaryFlightControlData.aileronPosition));
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTPRIMARYFLIGHTCONTROL_H
