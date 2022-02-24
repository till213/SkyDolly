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
#include <memory>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstdint>

#include <QObject>

#include "../../Kernel/src/SkyMath.h"
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "PositionData.h"
#include "Position.h"

class PositionPrivate
{
public:
    PositionPrivate(const AircraftInfo &aircraftInfo) noexcept
        : aircraftInfo(aircraftInfo),
          currentTimestamp(TimeVariableData::InvalidTime),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    const AircraftInfo &aircraftInfo;
    std::vector<PositionData> positionData;
    std::int64_t currentTimestamp;
    TimeVariableData::Access currentAccess;
    PositionData currentPositionData;
    mutable int currentIndex;
};

// PUBLIC

Position::Position(const AircraftInfo &aircraftInfo, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<PositionPrivate>(aircraftInfo))
{}

Position::~Position() noexcept
{}

void Position::upsertLast(const PositionData &positionData) noexcept
{
    if (d->positionData.size() > 0 && d->positionData.back() == positionData)  {
        // Same timestamp -> replace
        d->positionData[d->positionData.size() - 1] = positionData;
    } else {
        d->positionData.push_back(positionData);
    }
    emit dataChanged();
}

void Position::upsert(const PositionData &data) noexcept
{
    auto result = std::find_if(d->positionData.begin(), d->positionData.end(),
                              [&data] (const TimeVariableData &d) { return d.timestamp == data.timestamp; });
    if (result != d->positionData.end()) {
        // Same timestamp -> update
        *result = data;
    } else {
        d->positionData.push_back(data);
    }
}

const PositionData &Position::getFirst() const noexcept
{
    if (!d->positionData.empty()) {
        return d->positionData.front();
    } else {
        return PositionData::NullData;
    }
}

const PositionData &Position::getLast() const noexcept
{
    if (!d->positionData.empty()) {
        return d->positionData.back();
    } else {
        return PositionData::NullData;
    }
}

std::size_t Position::count() const noexcept
{
    return d->positionData.size();
}

const PositionData &Position::interpolate(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    const PositionData *p0, *p1, *p2, *p3;
    const double Tension = 0.0;
    const std::int64_t adjustedTimestamp = qMax(timestamp + d->aircraftInfo.timeOffset, std::int64_t(0));

    if (d->currentTimestamp != adjustedTimestamp || d->currentAccess != access) {

        double tn;
        switch (access) {
        case TimeVariableData::Access::Linear:
            if (SkySearch::getCubicInterpolationSupportData(d->positionData, adjustedTimestamp, SkySearch::PositionInterpolationWindow, d->currentIndex, &p0, &p1, &p2, &p3)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            if (SkySearch::getCubicInterpolationSupportData(d->positionData, adjustedTimestamp, SkySearch::InfinitetInterpolationWindow, d->currentIndex, &p0, &p1, &p2, &p3)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        default:
            p0 = p1 = p2 = p3 = nullptr;
            break;
        }

        if (p1 != nullptr) {
            // Aircraft position & attitude

            // Latitude: [-90, 90] - no discontinuity at +/- 90
            d->currentPositionData.latitude  = SkyMath::interpolateHermite(p0->latitude, p1->latitude, p2->latitude, p3->latitude, tn);
            // Longitude: [-180, 180] - discontinuity at the +/- 180 meridian
            d->currentPositionData.longitude = SkyMath::interpolateHermite180(p0->longitude, p1->longitude, p2->longitude, p3->longitude, tn);
            // Altitude [open range]
            d->currentPositionData.altitude  = SkyMath::interpolateHermite(p0->altitude, p1->altitude, p2->altitude, p3->altitude, tn);
            // Pitch: [-90, 90] - no discontinuity at +/- 90
            d->currentPositionData.pitch = SkyMath::interpolateHermite(p0->pitch, p1->pitch, p2->pitch, p3->pitch, tn, Tension);
            // Bank: [-180, 180] - discontinuity at +/- 180
            d->currentPositionData.bank  = SkyMath::interpolateHermite180(p0->bank, p1->bank, p2->bank, p3->bank, tn, Tension);
            // Heading: [0, 360] - discontinuity at 0/360
            d->currentPositionData.heading = SkyMath::interpolateHermite360(p0->heading, p1->heading, p2->heading, p3->heading, tn, Tension);

            // Velocity
            d->currentPositionData.velocityBodyX = SkyMath::interpolateLinear(p1->velocityBodyX, p2->velocityBodyX, tn);
            d->currentPositionData.velocityBodyY = SkyMath::interpolateLinear(p1->velocityBodyY, p2->velocityBodyY, tn);
            d->currentPositionData.velocityBodyZ = SkyMath::interpolateLinear(p1->velocityBodyZ, p2->velocityBodyZ, tn);
            d->currentPositionData.rotationVelocityBodyX = SkyMath::interpolateLinear(p1->rotationVelocityBodyX, p2->rotationVelocityBodyX, tn);
            d->currentPositionData.rotationVelocityBodyY = SkyMath::interpolateLinear(p1->rotationVelocityBodyY, p2->rotationVelocityBodyY, tn);
            d->currentPositionData.rotationVelocityBodyZ = SkyMath::interpolateLinear(p1->rotationVelocityBodyZ, p2->rotationVelocityBodyZ, tn);

            d->currentPositionData.timestamp = adjustedTimestamp;

        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentPositionData = PositionData::NullData;
        }
        d->currentTimestamp = adjustedTimestamp;
    }
    return d->currentPositionData;
}

void Position::clear() noexcept
{
    d->positionData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTime;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

Position::Iterator Position::begin() noexcept
{
    return d->positionData.begin();
}

Position::Iterator Position::end() noexcept
{
    return Iterator(d->positionData.end());
}

const Position::Iterator Position::begin() const noexcept
{
    return Iterator(d->positionData.begin());
}

const Position::Iterator Position::end() const noexcept
{
    return Iterator(d->positionData.end());
}

Position::InsertIterator Position::insertIterator() noexcept
{
    return std::inserter(d->positionData, d->positionData.begin());
}

// OPERATORS

PositionData& Position::operator[](std::size_t index) noexcept
{
    return d->positionData[index];
}

const PositionData& Position::operator[](std::size_t index) const noexcept
{
    return d->positionData[index];
}
