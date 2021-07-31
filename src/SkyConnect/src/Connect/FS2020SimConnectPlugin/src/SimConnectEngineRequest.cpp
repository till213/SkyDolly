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

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Model/src/SimVar.h"
#include "SimConnectType.h"
#include "SimConnectEngineRequest.h"

// PUBLIC

void SimConnectEngineRequest::addToDataDefinition(HANDLE simConnectHandle) noexcept
{
    addToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftEngineRequestDefinition));
}

// PROTECTED

void SimConnectEngineRequest::addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
{
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ThrottleLeverPosition1, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ThrottleLeverPosition2, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ThrottleLeverPosition3, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ThrottleLeverPosition4, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PropellerLeverPosition1, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PropellerLeverPosition2, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PropellerLeverPosition3, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PropellerLeverPosition4, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::MixtureLeverPosition1, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::MixtureLeverPosition2, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::MixtureLeverPosition3, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::MixtureLeverPosition4, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition1, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition2, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition3, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition4, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ElectricalMasterBattery1, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ElectricalMasterBattery2, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ElectricalMasterBattery3, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ElectricalMasterBattery4, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineStarter1, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineStarter2, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineStarter3, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineStarter4, "Bool", ::SIMCONNECT_DATATYPE_INT32);
}
