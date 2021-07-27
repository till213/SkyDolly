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
#include <memory>
#include <vector>
#include <iterator>

#include <QObject>

#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/SkyMath.h"
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "SecondaryFlightControlData.h"
#include "SecondaryFlightControl.h"

class SecondaryFlightControlPrivate
{
public:
    SecondaryFlightControlPrivate(const AircraftInfo &aircraftInfo) noexcept
        : aircraftInfo(aircraftInfo),
          currentTimestamp(TimeVariableData::InvalidTime),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    const AircraftInfo &aircraftInfo;
    std::vector<SecondaryFlightControlData> secondaryFlightControlData;
    qint64 currentTimestamp;
    TimeVariableData::Access currentAccess;
    SecondaryFlightControlData previousSecondaryFlightControlData;
    SecondaryFlightControlData currentSecondaryFlightControlData;
    mutable int currentIndex;

    static inline constexpr qint64 TimestampWindow = 1000; // msec
};

// PUBLIC

SecondaryFlightControl::SecondaryFlightControl(const AircraftInfo &aircraftInfo, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SecondaryFlightControlPrivate>(aircraftInfo))
{}

SecondaryFlightControl::~SecondaryFlightControl() noexcept
{}

void SecondaryFlightControl::upsert(const SecondaryFlightControlData &secondaryFlightControlData) noexcept
{
    if (d->secondaryFlightControlData.size() > 0 && d->secondaryFlightControlData.back().timestamp == secondaryFlightControlData.timestamp)  {
        // Same timestamp -> replace
        d->secondaryFlightControlData[d->secondaryFlightControlData.size() - 1] = secondaryFlightControlData;
    } else {
        d->secondaryFlightControlData.push_back(secondaryFlightControlData);
    }
    emit dataChanged();
}

const SecondaryFlightControlData &SecondaryFlightControl::getFirst() const noexcept
{
    if (!d->secondaryFlightControlData.empty()) {
        return d->secondaryFlightControlData.front();
    } else {
        return SecondaryFlightControlData::NullData;
    }
}

const SecondaryFlightControlData &SecondaryFlightControl::getLast() const noexcept
{
    if (!d->secondaryFlightControlData.empty()) {
        return d->secondaryFlightControlData.back();
    } else {
        return SecondaryFlightControlData::NullData;
    }
}

std::size_t SecondaryFlightControl::count() const noexcept
{
    return d->secondaryFlightControlData.size();
}

const SecondaryFlightControlData &SecondaryFlightControl::interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const SecondaryFlightControlData *p1, *p2;
    const qint64 adjustedTimestamp = qMax(timestamp + d->aircraftInfo.timeOffset, 0LL);

    if (d->currentTimestamp != adjustedTimestamp || d->currentAccess != access) {

        double tn;
        switch (access) {
        case TimeVariableData::Access::Linear:
            if (SkySearch::getLinearInterpolationSupportData(d->secondaryFlightControlData, adjustedTimestamp, d->currentIndex, &p1, &p2)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            d->currentIndex = SkySearch::updateStartIndex(d->secondaryFlightControlData, d->currentIndex, adjustedTimestamp);
            if (d->currentIndex != SkySearch::InvalidIndex) {
                p1 = &d->secondaryFlightControlData.at(d->currentIndex);
                p2 = p1;
                tn = 0.0;
            } else {
                p1 = p2 = nullptr;
            }
            break;
        default:
            p1 = p2 = nullptr;
            break;
        }

        if (p1 != nullptr) {
            d->currentSecondaryFlightControlData.leadingEdgeFlapsLeftPercent = SkyMath::interpolateLinear(p1->leadingEdgeFlapsLeftPercent, p2->leadingEdgeFlapsLeftPercent, tn);
            d->currentSecondaryFlightControlData.leadingEdgeFlapsRightPercent = SkyMath::interpolateLinear(p1->leadingEdgeFlapsRightPercent, p2->leadingEdgeFlapsRightPercent, tn);
            d->currentSecondaryFlightControlData.trailingEdgeFlapsLeftPercent = SkyMath::interpolateLinear(p1->trailingEdgeFlapsLeftPercent, p2->trailingEdgeFlapsLeftPercent, tn);
            d->currentSecondaryFlightControlData.trailingEdgeFlapsRightPercent = SkyMath::interpolateLinear(p1->trailingEdgeFlapsRightPercent, p2->trailingEdgeFlapsRightPercent, tn);
            d->currentSecondaryFlightControlData.spoilersHandlePosition = SkyMath::interpolateLinear(p1->spoilersHandlePosition, p2->spoilersHandlePosition, tn);

            // No interpolation for flaps handle position
            d->currentSecondaryFlightControlData.flapsHandleIndex = p1->flapsHandleIndex;
            // Certain aircrafts automatically override the FLAPS HANDLE INDEX, so values need to be repeatedly set
            if (Settings::getInstance().isRepeatFlapsHandleIndexEnabled()) {
                // We do that my storing the previous values (when the flaps are set)...
                d->previousSecondaryFlightControlData = d->currentSecondaryFlightControlData;
            } else {
                // "Repeat values" setting disabled
                d->previousSecondaryFlightControlData = SecondaryFlightControlData::NullData;
            }

            d->currentSecondaryFlightControlData.timestamp = adjustedTimestamp;
        } else if (!d->previousSecondaryFlightControlData.isNull()) {
            // ... and send the previous values again (for as long as the flaps are extracted)
            d->currentSecondaryFlightControlData = d->previousSecondaryFlightControlData;
            d->currentSecondaryFlightControlData.timestamp = adjustedTimestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentSecondaryFlightControlData = SecondaryFlightControlData::NullData;
        }

        d->currentTimestamp = adjustedTimestamp;
        d->currentAccess = access;
    }
    return d->currentSecondaryFlightControlData;
}

void SecondaryFlightControl::clear() noexcept
{
    d->secondaryFlightControlData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTime;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

SecondaryFlightControl::Iterator SecondaryFlightControl::begin() noexcept
{
    return d->secondaryFlightControlData.begin();
}

SecondaryFlightControl::Iterator SecondaryFlightControl::end() noexcept
{
    return Iterator(d->secondaryFlightControlData.end());
}

const SecondaryFlightControl::Iterator SecondaryFlightControl::begin() const noexcept
{
    return Iterator(d->secondaryFlightControlData.begin());
}

const SecondaryFlightControl::Iterator SecondaryFlightControl::end() const noexcept
{
    return Iterator(d->secondaryFlightControlData.end());
}

SecondaryFlightControl::InsertIterator SecondaryFlightControl::insertIterator() noexcept
{
    return std::inserter(d->secondaryFlightControlData, d->secondaryFlightControlData.begin());
}

// OPERATORS

SecondaryFlightControlData& SecondaryFlightControl::operator[](std::size_t index) noexcept
{
    return d->secondaryFlightControlData[index];
}

const SecondaryFlightControlData& SecondaryFlightControl::operator[](std::size_t index) const noexcept
{
    return d->secondaryFlightControlData[index];
}
