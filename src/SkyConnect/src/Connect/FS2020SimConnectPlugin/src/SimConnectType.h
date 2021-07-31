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
#ifndef SIMCONNECTTYPE_H
#define SIMCONNECTTYPE_H
#include <windows.h>

#include <SimConnect.h>

namespace SimConnectType
{
    enum class DataDefinition: ::SIMCONNECT_DATA_DEFINITION_ID {
        FlightInformationDefinition,
        FlightPlanDefinition,
        SimulationTimeDefinition,
        AircraftPositionDefinition,
        AircraftEngineReplyDefinition,
        AircraftEngineRequestDefinition,
        AircraftPrimaryFlightControlDefinition,
        AircraftSecondaryFlightControlDefinition,
        AircraftHandleDefinition,
        AircraftLightDefinition,
        AircraftInitialPosition
    };

    enum class DataRequest: ::SIMCONNECT_DATA_REQUEST_ID {
        AircraftInfo,
        FlightPlan,
        SimulationTime,
        AircraftPosition,
        Engine,
        PrimaryFlightControl,
        SecondaryFlightControl,
        AircraftHandle,
        Light,
        // AI objects
        AICreateObject,
        AIRemoveObject,
        AIReleaseControl,
        // Must come last
        AIObjectBase
    };
}

#endif // SIMCONNECTTYPE_H
