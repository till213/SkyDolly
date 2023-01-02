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
#ifndef SIMCONNECTENGINECORE_H
#define SIMCONNECTENGINECORE_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Kernel/Enum.h>
#include <Model/SimVar.h>
#include "SimConnectType.h"
#include <Model/EngineData.h>

/*!
 * Engine simulation variables that represent the core.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineCore
{
    float mixtureLeverPosition1 {0.0f};
    float mixtureLeverPosition2 {0.0f};
    float mixtureLeverPosition3 {0.0f};
    float mixtureLeverPosition4 {0.0f};
    std::int32_t electricalMasterBattery1 {0};
    std::int32_t electricalMasterBattery2 {0};
    std::int32_t electricalMasterBattery3 {0};
    std::int32_t electricalMasterBattery4 {0};
    std::int32_t generalEngineStarter1 {0};
    std::int32_t generalEngineStarter2 {0};
    std::int32_t generalEngineStarter3 {0};
    std::int32_t generalEngineStarter4 {0};

    SimConnectEngineCore(const EngineData &engineData) noexcept
        : SimConnectEngineCore()
    {
        fromEngineData(engineData);
    }
    SimConnectEngineCore() = default;

    inline EngineData toEngineData() const noexcept
    {
        EngineData engineData;
        toEngineData(engineData);
        return engineData;
    }

    inline void toEngineData(EngineData &engineData) const noexcept
    {
        // Note: the throttle can also yield negative thrust, hence the Sky Dolly internal type
        //       position (std::int16_t) which supports negative values as well
        engineData.mixtureLeverPosition1 = SkyMath::fromPercent(mixtureLeverPosition1);
        engineData.mixtureLeverPosition2 = SkyMath::fromPercent(mixtureLeverPosition2);
        engineData.mixtureLeverPosition3 = SkyMath::fromPercent(mixtureLeverPosition3);
        engineData.mixtureLeverPosition4 = SkyMath::fromPercent(mixtureLeverPosition4);
        engineData.electricalMasterBattery1 = (electricalMasterBattery1 != 0);
        engineData.electricalMasterBattery2 = (electricalMasterBattery2 != 0);
        engineData.electricalMasterBattery3 = (electricalMasterBattery3 != 0);
        engineData.electricalMasterBattery4 = (electricalMasterBattery4 != 0);
        engineData.generalEngineStarter1 = (generalEngineStarter1 != 0);
        engineData.generalEngineStarter2 = (generalEngineStarter2 != 0);
        engineData.generalEngineStarter3 = (generalEngineStarter3 != 0);
        engineData.generalEngineStarter4 = (generalEngineStarter4 != 0);
    }

    inline void fromEngineData(const EngineData &engineData) noexcept
    {
        mixtureLeverPosition1 = static_cast<float>(SkyMath::toPercent(engineData.mixtureLeverPosition1));
        mixtureLeverPosition2 = static_cast<float>(SkyMath::toPercent(engineData.mixtureLeverPosition2));
        mixtureLeverPosition3 = static_cast<float>(SkyMath::toPercent(engineData.mixtureLeverPosition3));
        mixtureLeverPosition4 = static_cast<float>(SkyMath::toPercent(engineData.mixtureLeverPosition4));
        electricalMasterBattery1 = engineData.electricalMasterBattery1 ? 1 : 0;
        electricalMasterBattery2 = engineData.electricalMasterBattery2 ? 1 : 0;
        electricalMasterBattery3 = engineData.electricalMasterBattery3 ? 1 : 0;
        electricalMasterBattery4 = engineData.electricalMasterBattery4 ? 1 : 0;
        generalEngineStarter1 = engineData.generalEngineStarter1 ? 1 : 0;
        generalEngineStarter2 = engineData.generalEngineStarter2 ? 1 : 0;
        generalEngineStarter3 = engineData.generalEngineStarter3 ? 1 : 0;
        generalEngineStarter4 = engineData.generalEngineStarter4 ? 1 : 0;
    }

    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::MixtureLeverPosition1, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::MixtureLeverPosition2, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::MixtureLeverPosition3, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::MixtureLeverPosition4, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ElectricalMasterBattery1, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ElectricalMasterBattery2, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ElectricalMasterBattery3, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ElectricalMasterBattery4, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineStarter1, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineStarter2, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineStarter3, "Bool", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::GeneralEngineStarter4, "Bool", ::SIMCONNECT_DATATYPE_INT32);
    }

    inline bool hasEngineStarterEnabled() const noexcept
    {
        return (generalEngineStarter1 || generalEngineStarter2 || generalEngineStarter3 || generalEngineStarter4);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTENGINECORE_H
