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
#ifndef SIMCONNECTPRIMARYFLIGHTCONTROLANIMATION_H
#define SIMCONNECTPRIMARYFLIGHTCONTROLANIMATION_H

#include <windows.h>
#include <SimConnect.h>

#include <Model/SimVar.h>
#include <Model/PrimaryFlightControlData.h>
#include "SimConnectType.h"

/*!
 * Primary flight control simulation variables that are sent exclusively to AI aircraft for animation.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPrimaryFlightControlAnimation
{
    // Radians
    float rudderDeflection {0.0f};
    float elevatorDeflection {0.0f};
    float aileronLeftDeflection {0.0f};
    float aileronRightDeflection {0.0f};

    SimConnectPrimaryFlightControlAnimation(const PrimaryFlightControlData &primaryFlightControlData) noexcept
        : SimConnectPrimaryFlightControlAnimation()
    {
        fromPrimaryFlightControlData(primaryFlightControlData);
    }

    SimConnectPrimaryFlightControlAnimation() = default;

    inline void fromPrimaryFlightControlData(const PrimaryFlightControlData &primaryFlightControlData) noexcept
    {
        rudderDeflection = primaryFlightControlData.rudderDeflection;
        elevatorDeflection = primaryFlightControlData.elevatorDeflection;
        aileronLeftDeflection = primaryFlightControlData.leftAileronDeflection;
        aileronRightDeflection = primaryFlightControlData.rightAileronDeflection;
    }

    inline PrimaryFlightControlData toPrimaryFlightControlData() const noexcept
    {
        PrimaryFlightControlData primaryFlightControlData;
        toPrimaryFlightControlData(primaryFlightControlData);
        return primaryFlightControlData;
    }

    inline void toPrimaryFlightControlData(PrimaryFlightControlData &primaryFlightControlData) const noexcept
    {
        primaryFlightControlData.rudderDeflection = rudderDeflection;
        primaryFlightControlData.elevatorDeflection = elevatorDeflection;
        primaryFlightControlData.leftAileronDeflection = aileronLeftDeflection;
        primaryFlightControlData.rightAileronDeflection = aileronRightDeflection;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RudderDeflection, "Radians", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ElevatorDeflection, "Radians", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::AileronLeftDeflection, "Radians", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::AileronRightDeflection, "Radians", ::SIMCONNECT_DATATYPE_FLOAT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTPRIMARYFLIGHTCONTROLANIMATION_H
