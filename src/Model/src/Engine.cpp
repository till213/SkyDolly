/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#include <algorithm>
#include <cstdint>

#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/SkyMath.h>
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "EngineData.h"
#include "Engine.h"

// PUBLIC

Engine::Engine(const AircraftInfo &aircraftInfo) noexcept
    : AbstractComponent(aircraftInfo)
{
#ifdef DEBUG
    qDebug() << "Engine::Engine: CREATED";
#endif
}

Engine::~Engine() noexcept
{
#ifdef DEBUG
    qDebug() << "Engine::Engine: DELETED";
#endif
}

const EngineData Engine::interpolate(std::int64_t timestamp, TimeVariableData::Access access) noexcept
{
    EngineData engineData;
    const EngineData *p1 {nullptr}, *p2 {nullptr};
    const std::int64_t timeOffset = access != TimeVariableData::Access::Export ? getAircraftInfo().timeOffset : 0;
    const std::int64_t adjustedTimestamp = std::max(timestamp + timeOffset, std::int64_t(0));

    if (getCurrentTimestamp() != adjustedTimestamp || getCurrentAccess() != access) {

        int currentIndex = getCurrentIndex();
        double tn {0.0};
        switch (access) {
        case TimeVariableData::Access::Linear:
            [[fallthrough]];
        case TimeVariableData::Access::Export:
            if (SkySearch::getLinearInterpolationSupportData(getData(), adjustedTimestamp, SkySearch::DefaultInterpolationWindow, currentIndex, &p1, &p2)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            currentIndex = SkySearch::updateStartIndex(getData(), currentIndex, adjustedTimestamp);
            if (currentIndex != SkySearch::InvalidIndex) {
                p1 = &getData().at(currentIndex);
                p2 = p1;
                tn = 0.0;
            } else {
                p1 = p2 = nullptr;
            }
            break;
        }

        if (p1 != nullptr) {
            engineData.throttleLeverPosition1 = SkyMath::interpolateLinear(p1->throttleLeverPosition1, p2->throttleLeverPosition1, tn);
            engineData.throttleLeverPosition2 = SkyMath::interpolateLinear(p1->throttleLeverPosition2, p2->throttleLeverPosition2, tn);
            engineData.throttleLeverPosition3 = SkyMath::interpolateLinear(p1->throttleLeverPosition3, p2->throttleLeverPosition3, tn);
            engineData.throttleLeverPosition4 = SkyMath::interpolateLinear(p1->throttleLeverPosition4, p2->throttleLeverPosition4, tn);
            engineData.propellerLeverPosition1 = SkyMath::interpolateLinear(p1->propellerLeverPosition1, p2->propellerLeverPosition1, tn);
            engineData.propellerLeverPosition2 = SkyMath::interpolateLinear(p1->propellerLeverPosition2, p2->propellerLeverPosition2, tn);
            engineData.propellerLeverPosition3 = SkyMath::interpolateLinear(p1->propellerLeverPosition3, p2->propellerLeverPosition3, tn);
            engineData.propellerLeverPosition4 = SkyMath::interpolateLinear(p1->propellerLeverPosition4, p2->propellerLeverPosition4, tn);
            engineData.mixtureLeverPosition1 = SkyMath::interpolateLinear(p1->mixtureLeverPosition1, p2->mixtureLeverPosition1, tn);
            engineData.mixtureLeverPosition2 = SkyMath::interpolateLinear(p1->mixtureLeverPosition2, p2->mixtureLeverPosition2, tn);
            engineData.mixtureLeverPosition3 = SkyMath::interpolateLinear(p1->mixtureLeverPosition3, p2->mixtureLeverPosition3, tn);
            engineData.mixtureLeverPosition4 = SkyMath::interpolateLinear(p1->mixtureLeverPosition4, p2->mixtureLeverPosition4, tn);
            engineData.cowlFlapPosition1 = SkyMath::interpolateLinear(p1->cowlFlapPosition1, p2->cowlFlapPosition1, tn);
            engineData.cowlFlapPosition2 = SkyMath::interpolateLinear(p1->cowlFlapPosition2, p2->cowlFlapPosition2, tn);
            engineData.cowlFlapPosition3 = SkyMath::interpolateLinear(p1->cowlFlapPosition3, p2->cowlFlapPosition3, tn);
            engineData.cowlFlapPosition4 = SkyMath::interpolateLinear(p1->cowlFlapPosition4, p2->cowlFlapPosition4, tn);

            // No interpolation for battery and starter/combustion states (boolean)
            engineData.electricalMasterBattery1 = p1->electricalMasterBattery1;
            engineData.electricalMasterBattery2 = p1->electricalMasterBattery2;
            engineData.electricalMasterBattery3 = p1->electricalMasterBattery3;
            engineData.electricalMasterBattery4 = p1->electricalMasterBattery4;
            engineData.generalEngineStarter1 = p1->generalEngineStarter1;
            engineData.generalEngineStarter2 = p1->generalEngineStarter2;
            engineData.generalEngineStarter3 = p1->generalEngineStarter3;
            engineData.generalEngineStarter4 = p1->generalEngineStarter4;
            engineData.generalEngineCombustion1 = p1->generalEngineCombustion1;
            engineData.generalEngineCombustion2 = p1->generalEngineCombustion2;
            engineData.generalEngineCombustion3 = p1->generalEngineCombustion3;
            engineData.generalEngineCombustion4 = p1->generalEngineCombustion4;

            engineData.timestamp = adjustedTimestamp;
        }

        setCurrentIndex(currentIndex);
        setCurrentTimestamp(adjustedTimestamp);
        setCurrentAccess(access);
    }
    return engineData;
}

template class AbstractComponent<EngineData>;
