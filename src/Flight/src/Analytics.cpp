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
#include <utility>
#include <limits>
#include <cstdint>

#include <QtGlobal>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/SkyMath.h>
#include <Kernel/Convert.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include "Analytics.h"

namespace  {
    // The first significant movement of 'DistanceThreshold' meters defines the
    // initial heading
    constexpr double DistanceThreshold = 10.0;
    constexpr double DefaultHeading = 0.0;
    constexpr std::int64_t DefaultTimestamp = 0.0;

    const auto distanceLambda = [](const PositionData &start, const PositionData &end) -> bool {
        const SkyMath::Coordinate startPos(start.latitude, start.longitude);
        const SkyMath::Coordinate endPos(end.latitude, end.longitude);
        const double distance = SkyMath::geodesicDistance(startPos, endPos);
        return std::abs(distance) > DistanceThreshold;
    };
}

struct AnalyticsPrivate
{
    AnalyticsPrivate(const Aircraft &theAircraft)
        : aircraft(theAircraft)
    {}

    const Aircraft &aircraft;
};

// PUBLIC

Analytics::Analytics(const Aircraft &aircraft)
    : d(std::make_unique<AnalyticsPrivate>(aircraft))
{}

Analytics::Analytics(Analytics &&rhs) noexcept = default;
Analytics &Analytics::operator=(Analytics &&rhs) noexcept = default;
Analytics::~Analytics() = default;

std::pair<std::int64_t, double> Analytics::firstMovementHeading() const noexcept
{
    std::pair<std::int64_t, double> result;
    Position &position = d->aircraft.getPosition();
    auto pos = std::adjacent_find(position.begin(), position.end(), ::distanceLambda);
    if (pos != position.end()) {
        auto nextPos = std::next(pos);
        if (nextPos != position.end()) {
            const SkyMath::Coordinate startPos(pos->latitude, pos->longitude);
            const SkyMath::Coordinate endPos(nextPos->latitude, nextPos->longitude);
            const double initialHeading = SkyMath::initialBearing(startPos, endPos);
            result = std::make_pair(pos->timestamp, initialHeading);
        } else {
            // Just one point
            result = std::make_pair(pos->timestamp, ::DefaultHeading);
        }

    } else {
        // Just one point
        result = std::make_pair(::DefaultTimestamp, ::DefaultHeading);
    }
    return result;
}

PositionData Analytics::closestPosition(double latitude, double longitude) const noexcept
{
    PositionData positionData;
    double minimumDistance = std::numeric_limits<double>::max();

    Position &position = d->aircraft.getPosition();
    for (const PositionData &pos : position) {
        const double distance = SkyMath::geodesicDistance(SkyMath::Coordinate(latitude, longitude),
                                                          SkyMath::Coordinate(pos.latitude, pos.longitude));
        if (minimumDistance > distance) {
            positionData = pos;
            minimumDistance = distance;
        }
    }
    return positionData;
}
