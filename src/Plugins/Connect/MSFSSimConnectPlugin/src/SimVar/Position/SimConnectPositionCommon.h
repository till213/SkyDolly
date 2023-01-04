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
#ifndef SIMCONNECTPOSITIONCOMMON_H
#define SIMCONNECTPOSITIONCOMMON_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Kernel/Enum.h>
#include <Model/SimVar.h>
#include <Model/PositionData.h>
#include <Model/InitialPosition.h>
#include "SimConnectType.h"

/*!
 * Common SimConnect position data that is sent both to the user- and AI aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPositionCommon
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

    // Implementation note:
    // If we would store the "rotation velocity body" (which we currently do not anymore) then
    // then the unit would be (wrongly) "FEET per second" (and not "RADIANS per second):
    // https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Aircraft_SimVars/Aircraft_Misc_Variables.htm#ROTATION_VELOCITY_BODY_X

    SimConnectPositionCommon(const PositionData &positionData) noexcept
        : SimConnectPositionCommon()
    {
        fromPositionData(positionData);
    }

    SimConnectPositionCommon() = default;

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
    }

    inline PositionData toPositionData() const noexcept
    {
        PositionData positionData;
        toPositionData(positionData);
        return positionData;
    }

    inline void toPositionData(PositionData &positionData) const noexcept
    {

        positionData.latitude = latitude;
        positionData.longitude = longitude;
        positionData.altitude = altitude;
        positionData.pitch = pitch;
        positionData.bank = bank;
        positionData.trueHeading = trueHeading;

        positionData.velocityBodyX = velocityBodyX;
        positionData.velocityBodyY = velocityBodyY;
        positionData.velocityBodyZ = velocityBodyZ;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        // Aircraft position & attitude
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::Latitude, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::Longitude, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::Altitude, "Feet", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::Pitch, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::Bank, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::TrueHeading, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);

        // Velocity
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::VelocityBodyX, "Feet per Second", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::VelocityBodyY, "Feet per Second",::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::VelocityBodyZ, "Feet per Second",::SIMCONNECT_DATATYPE_FLOAT64);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTPOSITIONCOMMON_H
