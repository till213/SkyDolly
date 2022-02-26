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

#include <windows.h>
#include <SimConnect.h>

#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../../../Model/src/EngineData.h"

/*!
 * Simulation variables which represent the engine (request sent to the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineRequest
{
    float throttleLeverPosition1;
    float throttleLeverPosition2;
    float throttleLeverPosition3;
    float throttleLeverPosition4;
    float propellerLeverPosition1;
    float propellerLeverPosition2;
    float propellerLeverPosition3;
    float propellerLeverPosition4;
    float mixtureLeverPosition1;
    float mixtureLeverPosition2;
    float mixtureLeverPosition3;
    float mixtureLeverPosition4;
    float recipEngineCowlFlapPosition1;
    float recipEngineCowlFlapPosition2;
    float recipEngineCowlFlapPosition3;
    float recipEngineCowlFlapPosition4;
    qint32 electricalMasterBattery1;
    qint32 electricalMasterBattery2;
    qint32 electricalMasterBattery3;
    qint32 electricalMasterBattery4;
    qint32 generalEngineStarter1;
    qint32 generalEngineStarter2;
    qint32 generalEngineStarter3;
    qint32 generalEngineStarter4;

    inline void fromEngineData(const EngineData &engineData) noexcept
    {
        throttleLeverPosition1 = SkyMath::toPosition(engineData.throttleLeverPosition1);
        throttleLeverPosition2 = SkyMath::toPosition(engineData.throttleLeverPosition2);
        throttleLeverPosition3 = SkyMath::toPosition(engineData.throttleLeverPosition3);
        throttleLeverPosition4 = SkyMath::toPosition(engineData.throttleLeverPosition4);
        propellerLeverPosition1 = SkyMath::toPosition(engineData.propellerLeverPosition1);
        propellerLeverPosition2 = SkyMath::toPosition(engineData.propellerLeverPosition2);
        propellerLeverPosition3 = SkyMath::toPosition(engineData.propellerLeverPosition3);
        propellerLeverPosition4 = SkyMath::toPosition(engineData.propellerLeverPosition4);
        mixtureLeverPosition1 = SkyMath::toPercent(engineData.mixtureLeverPosition1);
        mixtureLeverPosition2 = SkyMath::toPercent(engineData.mixtureLeverPosition2);
        mixtureLeverPosition3 = SkyMath::toPercent(engineData.mixtureLeverPosition3);
        mixtureLeverPosition4 = SkyMath::toPercent(engineData.mixtureLeverPosition4);
        recipEngineCowlFlapPosition1 = SkyMath::toPercent(engineData.cowlFlapPosition1);
        recipEngineCowlFlapPosition2 = SkyMath::toPercent(engineData.cowlFlapPosition2);
        recipEngineCowlFlapPosition3 = SkyMath::toPercent(engineData.cowlFlapPosition3);
        recipEngineCowlFlapPosition4 = SkyMath::toPercent(engineData.cowlFlapPosition4);
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
