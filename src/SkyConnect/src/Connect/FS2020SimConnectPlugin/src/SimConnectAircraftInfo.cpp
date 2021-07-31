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
#include "SimConnectAircraftInfo.h"

// PUBLIC

void SimConnectAircraftInfo::addToDataDefinition(HANDLE simConnectHandle) noexcept
{
    // Aircraft info
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::Title, nullptr, ::SIMCONNECT_DATATYPE_STRING256);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::ATCId, NULL, SIMCONNECT_DATATYPE_STRING32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::ATCAirline, NULL, SIMCONNECT_DATATYPE_STRING64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::ATCFlightNumber, NULL, SIMCONNECT_DATATYPE_STRING8);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::Category, NULL, SIMCONNECT_DATATYPE_STRING256);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::PlaneAltAboveGround, "Feet", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::SimOnGround, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::AirspeedTrue, "knots", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::WingSpan, "Feet", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::EngineType, "Number", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::NumberOfEngines, "Number", SIMCONNECT_DATATYPE_INT32);

    // Flight conditions
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::SurfaceType, "Number", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::GroundAltitude, "Feet", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::AmbientTemperature, "Celsius", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::TotalAirTemperature, "Celsius", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::AmbientWindVelocity, "Knots", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::AmbientWindDirection, "Degrees", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::AmbientVisibility, "Meters", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::SeaLevelPressure, "Millibars", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::PitotIcePct, "Percent", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::StructuralIcePct, "Percent", SIMCONNECT_DATATYPE_FLOAT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::AmbientPrecipState, "Mask", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::AmbientInCloud, "Bool", SIMCONNECT_DATATYPE_INT32);

    // Simulation time
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::LocalTime, "seconds", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::LocalYear, "number", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::LocalMonthOfYear, "number", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::LocalDayOfMonth, "number", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::ZuluTime, "seconds", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::ZuluYear, "number", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::ZuluMonthOfYear, "number", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), SimVar::ZuluDayOfMonth, "number", SIMCONNECT_DATATYPE_INT32);
}
