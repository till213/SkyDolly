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
#ifndef SIMCONNECTENGINEREPLY_H
#define SIMCONNECTENGINEREPLY_H

#include <cstdint>

#include <windows.h>

#include <Kernel/Enum.h>
#include <Model/EngineData.h>
#include "SimConnectType.h"
#include "SimConnectEngine.h"
#include "SimConnectEngineAnimation.h"

/*!
 * Simulation variables which represent the engine (reply received from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineReply
{
    SimConnectEngine engine;
    SimConnectEngineAnimation engineAnimation;
    std::int32_t generalEngineCombustion1 {0};
    std::int32_t generalEngineCombustion2 {0};
    std::int32_t generalEngineCombustion3 {0};
    std::int32_t generalEngineCombustion4 {0};

    inline EngineData toEngineData() const noexcept
    {
        EngineData engineData = engine.toEngineData();

        engineAnimation.toEngineData(engineData);
        engineData.generalEngineCombustion1 = (generalEngineCombustion1 != 0);
        engineData.generalEngineCombustion2 = (generalEngineCombustion2 != 0);
        engineData.generalEngineCombustion3 = (generalEngineCombustion3 != 0);
        engineData.generalEngineCombustion4 = (generalEngineCombustion4 != 0);

        return engineData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectEngine::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineReply));
        SimConnectEngineAnimation::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineReply));

        ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineReply), SimVar::GeneralEngineCombustion1, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineReply), SimVar::GeneralEngineCombustion2, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineReply), SimVar::GeneralEngineCombustion3, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineReply), SimVar::GeneralEngineCombustion4, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTENGINERESPONSE_H
