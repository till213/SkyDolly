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
#include <windows.h>

#include <SimConnect.h>

#include "../../Kernel/src/Enum.h"
#include "../../Model/src/SimVar.h"
#include "SimConnectType.h"
#include "SimConnectAircraftInfo.h"

void SimConnectAircraftInfo::addToDataDefinition(HANDLE simConnectHandle) noexcept
{
    // TODO Maybe support GPS WP PREV/NEXT ID
    // ZULU TIME, LOCAL TIME


//    api.AddToDataDefinition(DATA_DEFINITION_USER_AIRCRAFT, "Local Time",
//                              "seconds", SIMCONNECT_DATATYPE_INT32);
//      api.AddToDataDefinition(DATA_DEFINITION_USER_AIRCRAFT, "Local Year",
//                              "number", SIMCONNECT_DATATYPE_INT32);
//      api.AddToDataDefinition(DATA_DEFINITION_USER_AIRCRAFT, "Local Month of Year",
//                              "number", SIMCONNECT_DATATYPE_INT32);
//      api.AddToDataDefinition(DATA_DEFINITION_USER_AIRCRAFT, "Local Day of Month",
//                              "number", SIMCONNECT_DATATYPE_INT32);

//      api.AddToDataDefinition(DATA_DEFINITION_USER_AIRCRAFT, "Zulu Time",
//                              "seconds", SIMCONNECT_DATATYPE_INT32);
//      api.AddToDataDefinition(DATA_DEFINITION_USER_AIRCRAFT, "Zulu Year",
//                              "number", SIMCONNECT_DATATYPE_INT32);
//      api.AddToDataDefinition(DATA_DEFINITION_USER_AIRCRAFT, "Zulu Month of Year",
//                              "number", SIMCONNECT_DATATYPE_INT32);
//      api.AddToDataDefinition(DATA_DEFINITION_USER_AIRCRAFT, "Zulu Day of Month",
//                              "number", SIMCONNECT_DATATYPE_INT32);

      // Aircraft info
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::Title, nullptr, ::SIMCONNECT_DATATYPE_STRING256);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::ATCId, NULL, SIMCONNECT_DATATYPE_STRING32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::ATCAirline, NULL, SIMCONNECT_DATATYPE_STRING64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::ATCFlightNumber, NULL, SIMCONNECT_DATATYPE_STRING8);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::Category, NULL, SIMCONNECT_DATATYPE_STRING256);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::PlaneAltAboveGround, "Feet", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::SimOnGround, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::AirspeedTrue, "knots", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::WingSpan, "Feet", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::EngineType, "Number", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::NumberOfEngines, "Number", SIMCONNECT_DATATYPE_INT32);

    // Flight conditions
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::SurfaceType, "Number", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::GroundAltitude, "Feet", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::AmbientTemperature, "Celsius", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::TotalAirTemperature, "Celsius", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::AmbientWindVelocity, "Knots", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::AmbientWindDirection, "Degrees", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::AmbientVisibility, "Meters", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::SeaLevelPressure, "Millibars", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::PitotIcePct, "Percent", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::StructuralIcePct, "Percent", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::AmbientPrecipState, "Mask", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInfoDefinition), SimVar::AmbientInCloud, "Bool", SIMCONNECT_DATATYPE_INT32);
}

