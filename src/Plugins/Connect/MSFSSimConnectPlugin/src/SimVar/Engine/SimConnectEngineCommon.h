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
#ifndef SIMCONNECTENGINECOMMON_H
#define SIMCONNECTENGINECOMMON_H

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Kernel/Enum.h>
#include <Model/SimVar.h>
#include "SimConnectType.h"
#include <Model/EngineData.h>

/*!
 * Common SimConnect engine data that is sent both to the user- and AI aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineCommon
{
    // Percent
    float recipEngineCowlFlapPosition1 {0.0f};
    float recipEngineCowlFlapPosition2 {0.0f};
    float recipEngineCowlFlapPosition3 {0.0f};
    float recipEngineCowlFlapPosition4 {0.0f};

    SimConnectEngineCommon(const EngineData &engineData) noexcept
        : SimConnectEngineCommon()
    {
        fromEngineData(engineData);
    }
    SimConnectEngineCommon() = default;

    inline EngineData toEngineData() const noexcept
    {
        EngineData engineData;

        engineData.cowlFlapPosition1 = SkyMath::fromPercent(recipEngineCowlFlapPosition1);
        engineData.cowlFlapPosition2 = SkyMath::fromPercent(recipEngineCowlFlapPosition2);
        engineData.cowlFlapPosition3 = SkyMath::fromPercent(recipEngineCowlFlapPosition3);
        engineData.cowlFlapPosition4 = SkyMath::fromPercent(recipEngineCowlFlapPosition4);

        return engineData;
    }

    inline void fromEngineData(const EngineData &engineData) noexcept
    {
        recipEngineCowlFlapPosition1 = static_cast<float>(SkyMath::toPercent(engineData.cowlFlapPosition1));
        recipEngineCowlFlapPosition2 = static_cast<float>(SkyMath::toPercent(engineData.cowlFlapPosition2));
        recipEngineCowlFlapPosition3 = static_cast<float>(SkyMath::toPercent(engineData.cowlFlapPosition3));
        recipEngineCowlFlapPosition4 = static_cast<float>(SkyMath::toPercent(engineData.cowlFlapPosition4));
    }

    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition1, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition2, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition3, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition4, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTENGINECOMMON_H
