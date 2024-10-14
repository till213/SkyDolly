/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include "SimConnectAircraftInfo.h"

// PUBLIC

void SimConnectAircraftInfo::addToDataDefinition(HANDLE simConnectHandle) noexcept
{
    // Aircraft info
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::Title, nullptr, ::SIMCONNECT_DATATYPE_STRING256);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::ATCId, nullptr, SIMCONNECT_DATATYPE_STRING32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::ATCAirline, nullptr, SIMCONNECT_DATATYPE_STRING64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::Category, nullptr, SIMCONNECT_DATATYPE_STRING256);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::PlaneAltAboveGround, "Feet", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::SimOnGround, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::AirspeedTrue, "Knots", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::WingSpan, "Feet", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::EngineType, "Number", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftInfo), SimVar::NumberOfEngines, "Number", SIMCONNECT_DATATYPE_INT32);
}
