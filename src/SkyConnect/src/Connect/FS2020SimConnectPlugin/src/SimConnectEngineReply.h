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

#include <windows.h>

#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../../../Model/src/EngineData.h"
#include "SimConnectEngineRequest.h"

/*!
 * Simulation variables which represent the engine (reply received from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineReply : public SimConnectEngineRequest
{
    qint32 generalEngineCombustion1;
    qint32 generalEngineCombustion2;
    qint32 generalEngineCombustion3;
    qint32 generalEngineCombustion4;

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
        engineData.generalEngineCombustion1 = (generalEngineCombustion1 != 0);
        engineData.generalEngineCombustion2 = (generalEngineCombustion2 != 0);
        engineData.generalEngineCombustion3 = (generalEngineCombustion3 != 0);
        engineData.generalEngineCombustion4 = (generalEngineCombustion4 != 0);

        return engineData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTENGINERESPONSE_H
