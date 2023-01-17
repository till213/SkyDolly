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
#ifndef SIMCONNECTLIGHTEVENT_H
#define SIMCONNECTLIGHTEVENT_H

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/Enum.h>
#include <Model/SimVar.h>
#include <Model/SimType.h>
#include <Model/LightData.h>
#include "SimConnectType.h"

/*!
 * Light simulation variables that are sent as event to the user aircraft
 * (and possibly as simulation variables to AI aircraft).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectLightEvent
{
    std::int32_t navigation;
    std::int32_t beacon;
    std::int32_t landing;
    std::int32_t taxi;
    std::int32_t strobe;
    std::int32_t panel;
    std::int32_t recognition;
    std::int32_t wing;
    std::int32_t logo;
    std::int32_t cabin;

    SimConnectLightEvent(const LightData &lightData) noexcept
        : SimConnectLightEvent()
    {
        fromLightData(lightData);
    }

    SimConnectLightEvent() = default;

    inline void fromLightData(const LightData &lightData) noexcept
    {
        navigation = lightData.lightStates.testFlag(SimType::LightState::Navigation) ? 1 : 0;
        beacon = lightData.lightStates.testFlag(SimType::LightState::Beacon) ? 1 : 0;
        landing = lightData.lightStates.testFlag(SimType::LightState::Landing) ? 1 : 0;
        taxi = lightData.lightStates.testFlag(SimType::LightState::Taxi) ? 1 : 0;
        strobe = lightData.lightStates.testFlag(SimType::LightState::Strobe) ? 1 : 0;
        panel = lightData.lightStates.testFlag(SimType::LightState::Panel) ? 1 : 0;
        recognition = lightData.lightStates.testFlag(SimType::LightState::Recognition) ? 1 : 0;
        wing = lightData.lightStates.testFlag(SimType::LightState::Wing) ? 1 : 0;
        logo = lightData.lightStates.testFlag(SimType::LightState::Logo) ? 1 : 0;
        cabin = lightData.lightStates.testFlag(SimType::LightState::Cabin) ? 1 : 0;
    }

    inline LightData toLightData() const noexcept
    {
        LightData lightData;
        lightData.lightStates.setFlag(SimType::LightState::Navigation, navigation != 0);
        lightData.lightStates.setFlag(SimType::LightState::Beacon, beacon != 0);
        lightData.lightStates.setFlag(SimType::LightState::Landing, landing != 0);
        lightData.lightStates.setFlag(SimType::LightState::Taxi, taxi != 0);
        lightData.lightStates.setFlag(SimType::LightState::Strobe, strobe != 0);
        lightData.lightStates.setFlag(SimType::LightState::Panel, panel != 0);
        lightData.lightStates.setFlag(SimType::LightState::Recognition, recognition != 0);
        lightData.lightStates.setFlag(SimType::LightState::Wing, wing != 0);
        lightData.lightStates.setFlag(SimType::LightState::Logo, logo != 0);
        lightData.lightStates.setFlag(SimType::LightState::Cabin, cabin != 0);
        return lightData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::LightEvent));
    }

    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightNav, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightBeacon, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightLanding, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightTaxi, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightStrobe, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightPanel, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightRecognition, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightWing, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightLogo, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightCabin, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    }

};
#pragma pack(pop)

#endif // SIMCONNECTLIGHTEVENT_H
