/**
 * Sky Dolly - The black sheep for your fposition recordings
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
#include "PositionData.h"
#include "Position.h"

namespace
{
    constexpr double Tension = 0.0;
}

// PUBLIC

Position::Position(const AircraftInfo &aircraftInfo) noexcept
    : AbstractComponent(aircraftInfo)
{}

const PositionData &Position::interpolate(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    const PositionData *p0 {nullptr}, *p1 {nullptr}, *p2 {nullptr}, *p3 {nullptr};
    const auto timeOffset = access != TimeVariableData::Access::NoTimeOffset ? getAircraftInfo().timeOffset : 0;
    const auto adjustedTimestamp = std::max(timestamp + timeOffset, std::int64_t(0));

    if (getCurrentTimestamp() != adjustedTimestamp || getCurrentAccess() != access) {
        int currentIndex = getCurrentIndex();
        double tn {0.0};
        // Position data is always interpolated within an "infinite" interpolation window, in order to
        // take imported "sparse flight plans" into account
        if (SkySearch::getCubicInterpolationSupportData(getData(), adjustedTimestamp, SkySearch::InfinitetInterpolationWindow, currentIndex, &p0, &p1, &p2, &p3)) {
            tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
        }
        if (p1 != nullptr) {
            // Aircraft position

            // Latitude: [-90, 90] - no discontinuity at +/- 90
            m_currentData.latitude  = SkyMath::interpolateHermite(p0->latitude, p1->latitude, p2->latitude, p3->latitude, tn);
            // Longitude: [-180, 180] - discontinuity at the +/- 180 meridian
            m_currentData.longitude = SkyMath::interpolateHermite180(p0->longitude, p1->longitude, p2->longitude, p3->longitude, tn);
            // Altitude [open range]
            m_currentData.altitude  = SkyMath::interpolateHermite(p0->altitude, p1->altitude, p2->altitude, p3->altitude, tn);
            // The indicated altitude is not used for replay - only for display and analytical purposes,
            // so linear interpolation is sufficient
            m_currentData.indicatedAltitude  = SkyMath::interpolateLinear(p1->indicatedAltitude, p2->indicatedAltitude, tn);

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

template class AbstractComponent<PositionData>;
