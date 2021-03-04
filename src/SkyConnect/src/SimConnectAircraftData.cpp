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

#include "../../Kernel/src/Const.h"
#include "SimConnectDataDefinition.h"
#include "SimConnectAircraftData.h"

void SimConnectAircraftData::addToDataDefinition(HANDLE simConnectHandle)
{
    // Aircraft position
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::Latitude, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::Longitude, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::Altitude, "feet", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::Pitch, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::Bank, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::Heading, "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);

    // Aircraft controls
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::YokeXPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::YokeYPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::RudderPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::ElevatorPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::AileronPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);

    // Engine
    // TODO: GENERAL ENG PROPELLER LEVER POSITION, PROP RPM
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::ThrottleLeverPosition1, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::ThrottleLeverPosition2, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::ThrottleLeverPosition3, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::ThrottleLeverPosition4, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);

    // Flaps & speed brake
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::LeadingEdgeFlapsLeftPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::LeadingEdgeFlapsRightPercent, "Position",::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::TrailingEdgeFlapsLeftPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::TrailingEdgeFlapsRightPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    // Spoilers, also known as "speed brake"
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::SpoilersHandlePosition, "Percent", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::FlapsHandleIndex, "Number", ::SIMCONNECT_DATATYPE_INT32);

    // Gear & Brakes
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::GearHandlePosition, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::WaterRudderHandlePosition, "Percent Over 100", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::BrakeLeftPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, Const::BrakeRightPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
}
