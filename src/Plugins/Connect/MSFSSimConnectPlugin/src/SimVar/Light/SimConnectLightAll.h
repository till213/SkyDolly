/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#ifndef SIMCONNECTLIGHTEVENTALL_H
#define SIMCONNECTLIGHTEVENTALL_H

#include <windows.h>

#include <Kernel/Enum.h>
#include <Model/LightData.h>
#include "SimConnectType.h"
#include "SimConnectLightEvent.h"
#include "SimConnectLightAi.h"

/*!
 * All light simulation variables (reply from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectLightAll
{
    SimConnectLightEvent event;

    SimConnectLightAll(const LightData &data) noexcept
        : SimConnectLightAll()
    {
        fromLightData(data);
    }

    SimConnectLightAll() = default;

    inline void fromLightData(const LightData &data) noexcept
    {
        event.fromLightData(data);
    }

    inline LightData toLightData() const noexcept
    {
        LightData data = event.toLightData();
        return data;
    }

    inline SimConnectLightAi ai() const noexcept
    {
        SimConnectLightAi ai {};
        ai.event = event;
        return ai;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectLightEvent::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::LightAll));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTLIGHTEVENTALL_H
