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

#include <Kernel/SkyMath.h>
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "EngineData.h"
#include "Engine.h"

// PUBLIC

Engine::Engine(const AircraftInfo &aircraftInfo) noexcept
    : AbstractComponent(aircraftInfo)
{}

EngineData Engine::interpolate(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
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
            m_currentData.throttleLeverPosition1 = SkyMath::interpolateLinear(p1->throttleLeverPosition1, p2->throttleLeverPosition1, tn);
            m_currentData.throttleLeverPosition2 = SkyMath::interpolateLinear(p1->throttleLeverPosition2, p2->throttleLeverPosition2, tn);
            m_currentData.throttleLeverPosition3 = SkyMath::interpolateLinear(p1->throttleLeverPosition3, p2->throttleLeverPosition3, tn);
            m_currentData.throttleLeverPosition4 = SkyMath::interpolateLinear(p1->throttleLeverPosition4, p2->throttleLeverPosition4, tn);
            m_currentData.propellerLeverPosition1 = SkyMath::interpolateLinear(p1->propellerLeverPosition1, p2->propellerLeverPosition1, tn);
            m_currentData.propellerLeverPosition2 = SkyMath::interpolateLinear(p1->propellerLeverPosition2, p2->propellerLeverPosition2, tn);
            m_currentData.propellerLeverPosition3 = SkyMath::interpolateLinear(p1->propellerLeverPosition3, p2->propellerLeverPosition3, tn);
            m_currentData.propellerLeverPosition4 = SkyMath::interpolateLinear(p1->propellerLeverPosition4, p2->propellerLeverPosition4, tn);
            m_currentData.mixtureLeverPosition1 = SkyMath::interpolateLinear(p1->mixtureLeverPosition1, p2->mixtureLeverPosition1, tn);
            m_currentData.mixtureLeverPosition2 = SkyMath::interpolateLinear(p1->mixtureLeverPosition2, p2->mixtureLeverPosition2, tn);
            m_currentData.mixtureLeverPosition3 = SkyMath::interpolateLinear(p1->mixtureLeverPosition3, p2->mixtureLeverPosition3, tn);
            m_currentData.mixtureLeverPosition4 = SkyMath::interpolateLinear(p1->mixtureLeverPosition4, p2->mixtureLeverPosition4, tn);
            m_currentData.cowlFlapPosition1 = SkyMath::interpolateLinear(p1->cowlFlapPosition1, p2->cowlFlapPosition1, tn);
            m_currentData.cowlFlapPosition2 = SkyMath::interpolateLinear(p1->cowlFlapPosition2, p2->cowlFlapPosition2, tn);
            m_currentData.cowlFlapPosition3 = SkyMath::interpolateLinear(p1->cowlFlapPosition3, p2->cowlFlapPosition3, tn);
            m_currentData.cowlFlapPosition4 = SkyMath::interpolateLinear(p1->cowlFlapPosition4, p2->cowlFlapPosition4, tn);

            // No interpolation for battery and starter/combustion states (boolean)
            m_currentData.electricalMasterBattery1 = p1->electricalMasterBattery1;
            m_currentData.electricalMasterBattery2 = p1->electricalMasterBattery2;
            m_currentData.electricalMasterBattery3 = p1->electricalMasterBattery3;
            m_currentData.electricalMasterBattery4 = p1->electricalMasterBattery4;
            m_currentData.generalEngineStarter1 = p1->generalEngineStarter1;
            m_currentData.generalEngineStarter2 = p1->generalEngineStarter2;
            m_currentData.generalEngineStarter3 = p1->generalEngineStarter3;
            m_currentData.generalEngineStarter4 = p1->generalEngineStarter4;
            m_currentData.generalEngineCombustion1 = p1->generalEngineCombustion1;
            m_currentData.generalEngineCombustion2 = p1->generalEngineCombustion2;
            m_currentData.generalEngineCombustion3 = p1->generalEngineCombustion3;
            m_currentData.generalEngineCombustion4 = p1->generalEngineCombustion4;
            m_currentData.timestamp = adjustedTimestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded data
            m_currentData.reset();
        }

        setCurrentIndex(currentIndex);
        setCurrentTimestamp(adjustedTimestamp);
        setCurrentAccess(access);
    }
    return m_currentData;
}

template class AbstractComponent<EngineData>;
