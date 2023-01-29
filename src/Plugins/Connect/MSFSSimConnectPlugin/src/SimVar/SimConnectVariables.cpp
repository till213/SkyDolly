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
#include "SimConnectVariables.h"

// PUBLIC

void SimConnectVariables::addToDataDefinition(HANDLE simConnectHandle) noexcept
{
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::FlapsHandleIndex), SimVar::FlapsHandleIndex, "Number", ::SIMCONNECT_DATATYPE_INT32);

    // Lights
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::NavigationLight), SimVar::LightNav, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::BeaconLight), SimVar::LightBeacon, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::LandingLight), SimVar::LightLanding, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::TaxiLight), SimVar::LightTaxi, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::StrobeLight), SimVar::LightStrobe, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PanelLight), SimVar::LightPanel, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::RecognitionLight), SimVar::LightRecognition, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::WingLight), SimVar::LightWing, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::LogoLight), SimVar::LightLogo, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::CabinLight), SimVar::LightCabin, "Boolean", ::SIMCONNECT_DATATYPE_INT32);

    // Simulation
    ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::SimulationRate), SimVar::SimulationRate, "Number", ::SIMCONNECT_DATATYPE_FLOAT32);
}
