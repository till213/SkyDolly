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
#ifndef SIMCONNECTENGINEALL_H
#define SIMCONNECTENGINEALL_H

#include <cstdint>

#include <windows.h>

#include <Kernel/Enum.h>
#include <Model/EngineData.h>
#include "SimConnectType.h"
#include "SimConnectEngineCore.h"
#include "SimConnectEngineEvent.h"

/*!
 * Simulation variables which represent the engine (reply received from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineAll
{
    SimConnectEngineCore core;
    SimConnectEngineEvent event;

    SimConnectEngineAll(const EngineData &engineData) noexcept
        : SimConnectEngineAll()
    {
        fromEngineData(engineData);
    }

    SimConnectEngineAll() = default;

    inline EngineData toEngineData() const noexcept
    {
        EngineData engineData = core.toEngineData();
        event.toEngineData(engineData);
        return engineData;
    }

    inline void fromEngineData(const EngineData &engineData)
    {
        core.fromEngineData(engineData);
        event.fromEngineData(engineData);
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectEngineCore::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineAll));
        SimConnectEngineEvent::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineAll));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTENGINEALL_H
