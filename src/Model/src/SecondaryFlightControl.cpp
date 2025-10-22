/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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

#include <Kernel/Settings.h>
#include <Kernel/SkyMath.h>
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "SecondaryFlightControlData.h"
#include "SecondaryFlightControl.h"

// PUBLIC

SecondaryFlightControl::SecondaryFlightControl(const AircraftInfo &aircraftInfo) noexcept
    : AbstractComponent(aircraftInfo)
{}

const SecondaryFlightControlData &SecondaryFlightControl::interpolate(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    const SecondaryFlightControlData *p1 {nullptr}, *p2 {nullptr};
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
            m_currentData.leftLeadingEdgeFlapsPosition = SkyMath::interpolateLinear(p1->leftLeadingEdgeFlapsPosition, p2->leftLeadingEdgeFlapsPosition, tn);
            m_currentData.rightLeadingEdgeFlapsPosition = SkyMath::interpolateLinear(p1->rightLeadingEdgeFlapsPosition, p2->rightLeadingEdgeFlapsPosition, tn);
            m_currentData.leftTrailingEdgeFlapsPosition = SkyMath::interpolateLinear(p1->leftTrailingEdgeFlapsPosition, p2->leftTrailingEdgeFlapsPosition, tn);
            m_currentData.rightTrailingEdgeFlapsPosition = SkyMath::interpolateLinear(p1->rightTrailingEdgeFlapsPosition, p2->rightTrailingEdgeFlapsPosition, tn);
            m_currentData.leftSpoilersPosition = SkyMath::interpolateLinear(p1->leftSpoilersPosition, p2->leftSpoilersPosition, tn);
            m_currentData.rightSpoilersPosition = SkyMath::interpolateLinear(p1->rightSpoilersPosition, p2->rightSpoilersPosition, tn);
            m_currentData.spoilersHandlePercent = SkyMath::interpolateLinear(p1->spoilersHandlePercent, p2->spoilersHandlePercent, tn);
            // No interpolation for boolean values
            m_currentData.flapsHandleIndex = p1->flapsHandleIndex;
            m_currentData.spoilersArmed = p1->spoilersArmed;
            m_currentData.timestamp = adjustedTimestamp;
        } else {
            // No recorded data (and no repeat), or the timestamp exceeds the timestamp of the last recorded data
            m_currentData.reset();
        }

        setCurrentIndex(currentIndex);
        setCurrentTimestamp(adjustedTimestamp);
        setCurrentAccess(access);
    }
    return m_currentData;
}

template class AbstractComponent<SecondaryFlightControlData>;
