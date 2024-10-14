/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SIMCONNECTENGINEEVENT_H
#define SIMCONNECTENGINEEVENT_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <Model/SimVar.h>
#include <Model/EngineData.h>

/*!
 * Engine simulation variables that are sent as event to the user aircraft
 * (and possibly as simulation variables to AI aircraft)..
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineEvent
{
    std::int32_t generalEngineCombustion1 {0};
    std::int32_t generalEngineCombustion2 {0};
    std::int32_t generalEngineCombustion3 {0};
    std::int32_t generalEngineCombustion4 {0};

    SimConnectEngineEvent(const EngineData &data) noexcept
        : SimConnectEngineEvent()
    {
        fromEngineData(data);
    }

    SimConnectEngineEvent() = default;

    inline void fromEngineData(const EngineData &data) noexcept
    {
        generalEngineCombustion1 = data.generalEngineCombustion1 ? 1 : 0;
        generalEngineCombustion2 = data.generalEngineCombustion2 ? 1 : 0;
        generalEngineCombustion3 = data.generalEngineCombustion3 ? 1 : 0;
        generalEngineCombustion4 = data.generalEngineCombustion4 ? 1 : 0;
    }

    inline EngineData toEngineData() const noexcept
    {
        EngineData data;
        toEngineData(data);
        return data;
    }

    inline void toEngineData(EngineData &data) const noexcept
    {
        data.generalEngineCombustion1 = (generalEngineCombustion1 != 0);
        data.generalEngineCombustion2 = (generalEngineCombustion2 != 0);
        data.generalEngineCombustion3 = (generalEngineCombustion3 != 0);
        data.generalEngineCombustion4 = (generalEngineCombustion4 != 0);
    }

    inline bool hasCombustion() const noexcept
    {
        return (generalEngineCombustion1 || generalEngineCombustion2 || generalEngineCombustion3 || generalEngineCombustion4);
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineCombustion1, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineCombustion2, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineCombustion3, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineCombustion4, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTENGINEEVENT_H
