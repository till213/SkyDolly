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
{
#ifdef DEBUG
    qDebug() << "Position::Position: CREATED";
#endif
}

Position::~Position() noexcept
{
#ifdef DEBUG
    qDebug() << "Position::Position: DELETED";
#endif
}

const PositionData &Position::interpolate(std::int64_t timestamp, TimeVariableData::Access access) noexcept
{
    const PositionData *p0 {nullptr}, *p1 {nullptr}, *p2 {nullptr}, *p3 {nullptr};
    const std::int64_t timeOffset = access != TimeVariableData::Access::Export ? getAircraftInfo().timeOffset : 0;
    const std::int64_t adjustedTimestamp = std::max(timestamp + timeOffset, std::int64_t(0));

    if (getCurrentTimestamp() != adjustedTimestamp || getCurrentAccess() != access) {

        int currentIndex = getCurrentIndex();
        double tn {0.0};
        switch (access) {
        case TimeVariableData::Access::Linear:
            [[fallthrough]];
        case TimeVariableData::Access::Export:
            if (SkySearch::getCubicInterpolationSupportData(getData(), adjustedTimestamp, SkySearch::PositionInterpolationWindow, currentIndex, &p0, &p1, &p2, &p3)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            if (SkySearch::getCubicInterpolationSupportData(getData(), adjustedTimestamp, SkySearch::InfinitetInterpolationWindow, currentIndex, &p0, &p1, &p2, &p3)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        }

        if (p1 != nullptr) {
            // Aircraft position & attitude

            // Latitude: [-90, 90] - no discontinuity at +/- 90
            m_currentPositionData.latitude  = SkyMath::interpolateHermite(p0->latitude, p1->latitude, p2->latitude, p3->latitude, tn);
            // Longitude: [-180, 180] - discontinuity at the +/- 180 meridian
            m_currentPositionData.longitude = SkyMath::interpolateHermite180(p0->longitude, p1->longitude, p2->longitude, p3->longitude, tn);
            // Altitude [open range]
            m_currentPositionData.altitude  = SkyMath::interpolateHermite(p0->altitude, p1->altitude, p2->altitude, p3->altitude, tn);
            // The indicated altitude is not used for replay - only for display and analytical purposes,
            // so linear interpolation is sufficient
            m_currentPositionData.indicatedAltitude  = SkyMath::interpolateLinear(p1->indicatedAltitude, p2->indicatedAltitude, tn);
            // Pitch: [-90, 90] - no discontinuity at +/- 90
            m_currentPositionData.pitch = SkyMath::interpolateHermite(p0->pitch, p1->pitch, p2->pitch, p3->pitch, tn, ::Tension);
            // Bank: [-180, 180] - discontinuity at +/- 180
            m_currentPositionData.bank  = SkyMath::interpolateHermite180(p0->bank, p1->bank, p2->bank, p3->bank, tn, ::Tension);
            // Heading: [0, 360] - discontinuity at 0/360
            m_currentPositionData.heading = SkyMath::interpolateHermite360(p0->heading, p1->heading, p2->heading, p3->heading, tn, ::Tension);

            // Velocity
            m_currentPositionData.velocityBodyX = SkyMath::interpolateLinear(p1->velocityBodyX, p2->velocityBodyX, tn);
            m_currentPositionData.velocityBodyY = SkyMath::interpolateLinear(p1->velocityBodyY, p2->velocityBodyY, tn);
            m_currentPositionData.velocityBodyZ = SkyMath::interpolateLinear(p1->velocityBodyZ, p2->velocityBodyZ, tn);
            m_currentPositionData.rotationVelocityBodyX = SkyMath::interpolateLinear(p1->rotationVelocityBodyX, p2->rotationVelocityBodyX, tn);
            m_currentPositionData.rotationVelocityBodyY = SkyMath::interpolateLinear(p1->rotationVelocityBodyY, p2->rotationVelocityBodyY, tn);
            m_currentPositionData.rotationVelocityBodyZ = SkyMath::interpolateLinear(p1->rotationVelocityBodyZ, p2->rotationVelocityBodyZ, tn);

            m_currentPositionData.timestamp = adjustedTimestamp;

        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            m_currentPositionData = PositionData::NullData;
        }

        setCurrentIndex(currentIndex);
        setCurrentTimestamp(adjustedTimestamp);
        setCurrentAccess(access);
    }
    return m_currentPositionData;
}

template class AbstractComponent<PositionData>;