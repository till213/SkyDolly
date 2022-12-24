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
#ifndef SIMCONNECTENGINEREQUEST_H
#define SIMCONNECTENGINEREQUEST_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Model/EngineData.h>

/*!
 * Simulation variables which represent the engine (request sent to the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineRequest
{
    float throttleLeverPosition1 {0.0f};
    float throttleLeverPosition2 {0.0f};
    float throttleLeverPosition3 {0.0f};
    float throttleLeverPosition4 {0.0f};
    float propellerLeverPosition1 {0.0f};
    float propellerLeverPosition2 {0.0f};
    float propellerLeverPosition3 {0.0f};
    float propellerLeverPosition4 {0.0f};
    float mixtureLeverPosition1 {0.0f};
    float mixtureLeverPosition2 {0.0f};
    float mixtureLeverPosition3 {0.0f};
    float mixtureLeverPosition4 {0.0f};
    float recipEngineCowlFlapPosition1 {0.0f};
    float recipEngineCowlFlapPosition2 {0.0f};
    float recipEngineCowlFlapPosition3 {0.0f};
    float recipEngineCowlFlapPosition4 {0.0f};
    std::int32_t electricalMasterBattery1 {0};
    std::int32_t electricalMasterBattery2 {0};
    std::int32_t electricalMasterBattery3 {0};
    std::int32_t electricalMasterBattery4 {0};
    std::int32_t generalEngineStarter1 {0};
    std::int32_t generalEngineStarter2 {0};
    std::int32_t generalEngineStarter3 {0};
    std::int32_t generalEngineStarter4 {0};

    inline void fromEngineData(const EngineData &engineData) noexcept
    {
        throttleLeverPosition1 = static_cast<float>(SkyMath::toNormalisedPosition(engineData.throttleLeverPosition1));
        throttleLeverPosition2 = static_cast<float>(SkyMath::toNormalisedPosition(engineData.throttleLeverPosition2));
        throttleLeverPosition3 = static_cast<float>(SkyMath::toNormalisedPosition(engineData.throttleLeverPosition3));
        throttleLeverPosition4 = static_cast<float>(SkyMath::toNormalisedPosition(engineData.throttleLeverPosition4));
        propellerLeverPosition1 = static_cast<float>(SkyMath::toNormalisedPosition(engineData.propellerLeverPosition1));
        propellerLeverPosition2 = static_cast<float>(SkyMath::toNormalisedPosition(engineData.propellerLeverPosition2));
        propellerLeverPosition3 = static_cast<float>(SkyMath::toNormalisedPosition(engineData.propellerLeverPosition3));
        propellerLeverPosition4 = static_cast<float>(SkyMath::toNormalisedPosition(engineData.propellerLeverPosition4));
        mixtureLeverPosition1 = static_cast<float>(SkyMath::toPercent(engineData.mixtureLeverPosition1));
        mixtureLeverPosition2 = static_cast<float>(SkyMath::toPercent(engineData.mixtureLeverPosition2));
        mixtureLeverPosition3 = static_cast<float>(SkyMath::toPercent(engineData.mixtureLeverPosition3));
        mixtureLeverPosition4 = static_cast<float>(SkyMath::toPercent(engineData.mixtureLeverPosition4));
        recipEngineCowlFlapPosition1 = static_cast<float>(SkyMath::toPercent(engineData.cowlFlapPosition1));
        recipEngineCowlFlapPosition2 = static_cast<float>(SkyMath::toPercent(engineData.cowlFlapPosition2));
        recipEngineCowlFlapPosition3 = static_cast<float>(SkyMath::toPercent(engineData.cowlFlapPosition3));
        recipEngineCowlFlapPosition4 = static_cast<float>(SkyMath::toPercent(engineData.cowlFlapPosition4));
        electricalMasterBattery1 = engineData.electricalMasterBattery1 ? 1 : 0;
        electricalMasterBattery2 = engineData.electricalMasterBattery2 ? 1 : 0;
        electricalMasterBattery3 = engineData.electricalMasterBattery3 ? 1 : 0;
        electricalMasterBattery4 = engineData.electricalMasterBattery4 ? 1 : 0;
        generalEngineStarter1 = engineData.generalEngineStarter1 ? 1 : 0;
        generalEngineStarter2 = engineData.generalEngineStarter2 ? 1 : 0;
        generalEngineStarter3 = engineData.generalEngineStarter3 ? 1 : 0;
        generalEngineStarter4 = engineData.generalEngineStarter4 ? 1 : 0;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;

protected:
    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTENGINEREQUEST_H
