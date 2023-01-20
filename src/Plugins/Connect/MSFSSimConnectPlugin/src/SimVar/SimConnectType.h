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
    enum struct DataDefinition: ::SIMCONNECT_DATA_DEFINITION_ID {
        FlightInformation,
        FlightPlan,
        SimulationTime,
        InitialPosition,
        Location,
        PositionUser,
        PositionAi,
        PositionAll,
        EngineUser,
        EngineAi,
        EngineAll,
        PrimaryFlightControlAi,
        PrimaryFlightControlAll,
        SecondaryFlightControlAi,
        SecondaryFlightControlAll,
        AircraftHandleUser,
        AircraftHandleAi,
        AircraftHandleAll,
        LightAi,
        LightAll,
        // Simulation variables
        FlapsHandleIndex,
        NavigationLight,
        BeaconLight,
        LandingLight,
        TaxiLight,
        StrobeLight,
        PanelLight,
        RecognitionLight,
        WingLight,
        LogoLight,
        CabinLight
    };

    enum struct DataRequest: ::SIMCONNECT_DATA_REQUEST_ID {
        AircraftInfo,
        FlightPlan,
        InitialPosition,
        Location,
        SimulationTime,
        AircraftPositionAll,
        EngineAll,
        PrimaryFlightControlAll,
        SecondaryFlightControlAll,
        AircraftHandleAll,
        LightAll,
        // Simulation variables
        FlapsHandleIndex,
        NavigationLight,
        BeaconLight,
        LandingLight,
        TaxiLight,
        StrobeLight,
        PanelLight,
        RecognitionLight,
        WingLight,
        LogoLight,
        CabinLight,
        // AI objects
        AiRemoveObject,
        AiReleaseControl,
        // Must come last
        AiObjectBase
    };
}

#endif // SIMCONNECTTYPE_H
