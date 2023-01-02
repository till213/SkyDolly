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
#include <windows.h>

#include <SimConnect.h>

#include <Kernel/Enum.h>
#include <Model/SimVar.h>
#include "SimConnectType.h"
#include "SimConnectPositionRequest.h"

// PUBLIC

void SimConnectPositionRequest::addToDataDefinition(HANDLE simConnectHandle) noexcept
{
    addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionRequest));
}

// PROTECTED

void SimConnectPositionRequest::addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
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
