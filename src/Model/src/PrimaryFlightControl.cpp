/**
 * Sky Dolly - The black sheep for your fposition recordings
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
#include <algorithm>
#include <cstdint>

#include <Kernel/SkyMath.h>
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "PrimaryFlightControlData.h"
#include "PrimaryFlightControl.h"


// PUBLIC

PrimaryFlightControl::PrimaryFlightControl(const AircraftInfo &aircraftInfo) noexcept
    : AbstractComponent(aircraftInfo)
{}

const PrimaryFlightControlData &PrimaryFlightControl::interpolate(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    const PrimaryFlightControlData *p1 {nullptr}, *p2 {nullptr};
    const auto timeOffset = access != TimeVariableData::Access::NoTimeOffset ? getAircraftInfo().timeOffset : 0;
    const auto adjustedTimestamp = std::max(timestamp + timeOffset, std::int64_t(0));

    if (getCurrentTimestamp() != adjustedTimestamp || getCurrentAccess() != access) {
        int currentIndex = getCurrentIndex();
        double tn {0.0};
        switch (access) {
        case TimeVariableData::Access::Linear:
            [[fallthrough]];
        case TimeVariableData::Access::NoTimeOffset:
            if (SkySearch::getLinearInterpolationSupportData(getData(), adjustedTimestamp, SkySearch::DefaultInterpolationWindow, currentIndex, &p1, &p2)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        case TimeVariableData::Access::DiscreteSeek:
            [[fallthrough]];
        case TimeVariableData::Access::ContinuousSeek:
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
            m_currentData.leftAileronDeflection = SkyMath::interpolateLinear(p1->leftAileronDeflection, p2->leftAileronDeflection, tn);
            m_currentData.rightAileronDeflection = SkyMath::interpolateLinear(p1->rightAileronDeflection, p2->rightAileronDeflection, tn);
            m_currentData.elevatorDeflection = SkyMath::interpolateLinear(p1->elevatorDeflection, p2->elevatorDeflection, tn);
            m_currentData.rudderDeflection = SkyMath::interpolateLinear(p1->rudderDeflection, p2->rudderDeflection, tn);
            m_currentData.rudderPosition = SkyMath::interpolateLinear(p1->rudderPosition, p2->rudderPosition, tn);
            m_currentData.elevatorPosition = SkyMath::interpolateLinear(p1->elevatorPosition, p2->elevatorPosition, tn);
            m_currentData.aileronPosition = SkyMath::interpolateLinear(p1->aileronPosition, p2->aileronPosition, tn);
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

template class AbstractComponent<PrimaryFlightControlData>;
