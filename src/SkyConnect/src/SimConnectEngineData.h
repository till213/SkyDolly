/**
 * Sky Dolly - The black sheep for your flight recordings
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
#ifndef SIMCONNECTENGINEDATA_H
#define SIMCONNECTENGINEDATA_H

#include <windows.h>

#include "../../Kernel/src/SkyMath.h"
#include "../../Model/src/SimType.h"
#include "../../Model/src/EngineData.h"

/*!
 * Simulation variables which represent the engine.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineData
{
    double throttleLeverPosition1;
    double throttleLeverPosition2;
    double throttleLeverPosition3;
    double throttleLeverPosition4;
    double propellerLeverPosition1;
    double propellerLeverPosition2;
    double propellerLeverPosition3;
    double propellerLeverPosition4;
    double mixtureLeverPosition1;
    double mixtureLeverPosition2;
    double mixtureLeverPosition3;
    double mixtureLeverPosition4;
    double recipEngineCowlFlapPosition1;
    double recipEngineCowlFlapPosition2;
    double recipEngineCowlFlapPosition3;
    double recipEngineCowlFlapPosition4;
    qint32 electricalMasterBattery1;
    qint32 electricalMasterBattery2;
    qint32 electricalMasterBattery3;
    qint32 electricalMasterBattery4;
    qint32 generalEngineStarter1;
    qint32 generalEngineStarter2;
    qint32 generalEngineStarter3;
    qint32 generalEngineStarter4;


    inline EngineData toEngineData() const noexcept
    {
        EngineData engineData;

        // Note: the throttle can also yield negative thrust, hence the Sky Dolly internal type
        //       position (qint16) which supports negative values as well
        engineData.throttleLeverPosition1 = SkyMath::fromPosition(throttleLeverPosition1);
        engineData.throttleLeverPosition2 = SkyMath::fromPosition(throttleLeverPosition2);
        engineData.throttleLeverPosition3 = SkyMath::fromPosition(throttleLeverPosition3);
        engineData.throttleLeverPosition4 = SkyMath::fromPosition(throttleLeverPosition4);
        engineData.propellerLeverPosition1 = SkyMath::fromPosition(propellerLeverPosition1);
        engineData.propellerLeverPosition2 = SkyMath::fromPosition(propellerLeverPosition2);
        engineData.propellerLeverPosition3 = SkyMath::fromPosition(propellerLeverPosition3);
        engineData.propellerLeverPosition4 = SkyMath::fromPosition(propellerLeverPosition4);
        engineData.mixtureLeverPosition1 = SkyMath::fromPercent(mixtureLeverPosition1);
        engineData.mixtureLeverPosition2 = SkyMath::fromPercent(mixtureLeverPosition2);
        engineData.mixtureLeverPosition3 = SkyMath::fromPercent(mixtureLeverPosition3);
        engineData.mixtureLeverPosition4 = SkyMath::fromPercent(mixtureLeverPosition4);
        engineData.cowlFlapPosition1 = SkyMath::fromPercent(recipEngineCowlFlapPosition1);
        engineData.cowlFlapPosition2 = SkyMath::fromPercent(recipEngineCowlFlapPosition2);
        engineData.cowlFlapPosition3 = SkyMath::fromPercent(recipEngineCowlFlapPosition3);
        engineData.cowlFlapPosition4 = SkyMath::fromPercent(recipEngineCowlFlapPosition4);
        engineData.electricalMasterBattery1 = (electricalMasterBattery1 != 0);
        engineData.electricalMasterBattery2 = (electricalMasterBattery2 != 0);
        engineData.electricalMasterBattery3 = (electricalMasterBattery3 != 0);
        engineData.electricalMasterBattery4 = (electricalMasterBattery4 != 0);
        engineData.generalEngineStarter1 = (generalEngineStarter1 != 0);
        engineData.generalEngineStarter2 = (generalEngineStarter2 != 0);
        engineData.generalEngineStarter3 = (generalEngineStarter3 != 0);
        engineData.generalEngineStarter4 = (generalEngineStarter4 != 0);

        return engineData;
    }

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
};
#pragma pack(pop)

#endif // SIMCONNECTENGINEDATA_H
