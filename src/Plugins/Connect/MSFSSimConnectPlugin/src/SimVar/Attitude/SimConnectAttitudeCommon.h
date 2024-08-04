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
#ifndef SIMCONNECTATTITUDECOMMON_H
#define SIMCONNECTATTITUDECOMMON_H

#include <windows.h>
#include <SimConnect.h>

#include <Model/SimVar.h>
#include <Model/AttitudeData.h>

/*!
 * Common aircraft attitude simulation variables that are sent both to the user- and AI aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAttitudeCommon
{
    // Attitude
    double pitch {0.0};
    double bank {0.0};
    double trueHeading {0.0};

    // Velocity
    double velocityBodyX {0.0};
    double velocityBodyY {0.0};
    double velocityBodyZ {0.0};

    // Implementation note:
    // If we would store the "rotation velocity body" (which we currently do not anymore) then
    // then the unit would be (wrongly) "FEET per second" (and not "RADIANS per second):
    // https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Aircraft_SimVars/Aircraft_Misc_Variables.htm#ROTATION_VELOCITY_BODY_X

    SimConnectAttitudeCommon(const AttitudeData &data) noexcept
        : SimConnectAttitudeCommon()
    {
        fromAttitudeData(data);
    }

    SimConnectAttitudeCommon() = default;

    inline void fromAttitudeData(const AttitudeData &data) noexcept
    {
        pitch = data.pitch;
        bank = data.bank;
        trueHeading = data.trueHeading;

        velocityBodyX = data.velocityBodyX;
        velocityBodyY = data.velocityBodyY;
        velocityBodyZ = data.velocityBodyZ;
    }

    inline AttitudeData toAttitudeData() const noexcept
    {
        AttitudeData data;
        toAttitudeData(data);
        return data;
    }

    inline void toAttitudeData(AttitudeData &data) const noexcept
    {
        data.pitch = pitch;
        data.bank = bank;
        data.trueHeading = trueHeading;

        data.velocityBodyX = velocityBodyX;
        data.velocityBodyY = velocityBodyY;
        data.velocityBodyZ = velocityBodyZ;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        // Aircraft attitude
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::Pitch, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::Bank, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::TrueHeading, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);

        // Velocity
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::VelocityBodyX, "Feet per Second", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::VelocityBodyY, "Feet per Second", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::VelocityBodyZ, "Feet per Second", ::SIMCONNECT_DATATYPE_FLOAT64);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTATTITUDECOMMON_H
