/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

    SimConnectPrimaryFlightControlAnimation(const PrimaryFlightControlData &data) noexcept
        : SimConnectPrimaryFlightControlAnimation()
    {
        fromPrimaryFlightControlData(data);
    }

    SimConnectPrimaryFlightControlAnimation() = default;

    inline void fromPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept
    {
        rudderDeflection = data.rudderDeflection;
        elevatorDeflection = data.elevatorDeflection;
        aileronLeftDeflection = data.leftAileronDeflection;
        aileronRightDeflection = data.rightAileronDeflection;
    }

    inline PrimaryFlightControlData toPrimaryFlightControlData() const noexcept
    {
        PrimaryFlightControlData primaryFlightControlData;
        toPrimaryFlightControlData(primaryFlightControlData);
        return primaryFlightControlData;
    }

    inline void toPrimaryFlightControlData(PrimaryFlightControlData &data) const noexcept
    {
        data.rudderDeflection = rudderDeflection;
        data.elevatorDeflection = elevatorDeflection;
        data.leftAileronDeflection = aileronLeftDeflection;
        data.rightAileronDeflection = aileronRightDeflection;
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
