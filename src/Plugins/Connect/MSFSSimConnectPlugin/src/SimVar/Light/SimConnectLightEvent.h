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
 * Light simulation variables (taxi, navigation, ...) that are
 * sent as event to the user aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectLightEvent
{
    std::int32_t lightStates {0};

    SimConnectLightEvent(const LightData &lightData) noexcept
        : SimConnectLightEvent()
    {
        fromLightData(lightData);
    }

    SimConnectLightEvent() = default;

    inline void fromLightData(const LightData &lightData) noexcept
    {
        lightStates = lightData.lightStates;
    }

    inline LightData toLightData() const noexcept
    {
        LightData lightData;
        lightData.lightStates = SimType::LightStates(lightStates);
        return lightData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::LightEvent));
    }

    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LightStates, "Mask", ::SIMCONNECT_DATATYPE_INT32);
    }

};
#pragma pack(pop)

#endif // SIMCONNECTLIGHTEVENT_H
