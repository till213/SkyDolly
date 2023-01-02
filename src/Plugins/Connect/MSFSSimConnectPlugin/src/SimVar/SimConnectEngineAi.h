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
#ifndef SIMCONNECTENGINEAI_H
#define SIMCONNECTENGINEAI_H

#include <Kernel/Enum.h>
#include <Model/EngineData.h>
#include "SimConnectType.h"
#include "SimConnectEngineCommon.h"
#include "SimConnectEngineEvent.h"

/*!
 * Engine simulation variables that are sent to AI aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineAi
{
    SimConnectEngineCommon common;
    SimConnectEngineEvent event;

    SimConnectEngineAi(const EngineData &engineData) noexcept
        : SimConnectEngineAi()
    {
        fromEngineData(engineData);
    }

    SimConnectEngineAi() = default;

    inline void fromEngineData(const EngineData &engineData)
    {
        common.fromEngineData(engineData);
        event.fromEngineData(engineData);
    }

    inline EngineData toEngineData() const noexcept
    {
        EngineData engineData = common.toEngineData();
        event.toEngineData(engineData);
        return engineData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectEngineCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineAi));
        SimConnectEngineEvent::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineAi));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTENGINEAI_H
