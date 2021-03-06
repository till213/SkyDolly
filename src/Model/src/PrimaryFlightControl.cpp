/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include "PrimaryFlightControl.h"

/**
 * Sky Dolly - The black sheep for your flight recordings
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

#include "../../Kernel/src/SkyMath.h"
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "PrimaryFlightControlData.h"
#include "PrimaryFlightControl.h"

class PrimaryFlightControlPrivate
{
public:
    PrimaryFlightControlPrivate() noexcept
        : currentTimestamp(TimeVariableData::InvalidTime),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    std::vector<PrimaryFlightControlData> primaryFlightControlData;
    qint64 currentTimestamp;
    TimeVariableData::Access currentAccess;
    PrimaryFlightControlData currentPrimaryFlightControlData;
    mutable int currentIndex;

    static inline constexpr qint64 TimestampWindow = 1000; // msec
};

// PUBLIC

PrimaryFlightControl::PrimaryFlightControl(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<PrimaryFlightControlPrivate>())
{}

PrimaryFlightControl::~PrimaryFlightControl() noexcept
{}

void PrimaryFlightControl::upsert(const PrimaryFlightControlData &primaryFlightControlData) noexcept
{
    if (d->primaryFlightControlData.size() > 0 && d->primaryFlightControlData.back().timestamp == primaryFlightControlData.timestamp)  {
        // Same timestamp -> replace
        d->primaryFlightControlData[d->primaryFlightControlData.size() - 1] = primaryFlightControlData;
    } else {
        d->primaryFlightControlData.push_back(primaryFlightControlData);
    }
    emit dataChanged();
}

const PrimaryFlightControlData &PrimaryFlightControl::getFirst() const noexcept
{
    if (!d->primaryFlightControlData.empty()) {
        return d->primaryFlightControlData.front();
    } else {
        return PrimaryFlightControlData::NullData;
    }
}

const PrimaryFlightControlData &PrimaryFlightControl::getLast() const noexcept
{
    if (!d->primaryFlightControlData.empty()) {
        return d->primaryFlightControlData.back();
    } else {
        return PrimaryFlightControlData::NullData;
    }
}

std::size_t PrimaryFlightControl::count() const noexcept
{
    return d->primaryFlightControlData.size();
}

const PrimaryFlightControlData &PrimaryFlightControl::interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const PrimaryFlightControlData *p1, *p2;

    if (d->currentTimestamp != timestamp || d->currentAccess != access) {

        double tn;
        switch (access) {
        case TimeVariableData::Access::Linear:
            if (SkySearch::getLinearInterpolationSupportData(d->primaryFlightControlData, timestamp, d->currentIndex, &p1, &p2)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, timestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            d->currentIndex = SkySearch::updateStartIndex(d->primaryFlightControlData, d->currentIndex, timestamp);
            if (d->currentIndex != SkySearch::InvalidIndex) {
                p1 = &d->primaryFlightControlData.at(d->currentIndex);
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
            d->currentPrimaryFlightControlData.rudderPosition = SkyMath::interpolateLinear(p1->rudderPosition, p2->rudderPosition, tn);
            d->currentPrimaryFlightControlData.elevatorPosition = SkyMath::interpolateLinear(p1->elevatorPosition, p2->elevatorPosition, tn);
            d->currentPrimaryFlightControlData.aileronPosition = SkyMath::interpolateLinear(p1->aileronPosition, p2->aileronPosition, tn);
            d->currentPrimaryFlightControlData.timestamp = timestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentPrimaryFlightControlData = PrimaryFlightControlData::NullData;
        }

        d->currentTimestamp = timestamp;
        d->currentAccess = access;
    }
    return d->currentPrimaryFlightControlData;
}

void PrimaryFlightControl::clear() noexcept
{
    d->primaryFlightControlData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTime;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

PrimaryFlightControl::Iterator PrimaryFlightControl::begin() noexcept
{
    return d->primaryFlightControlData.begin();
}

PrimaryFlightControl::Iterator PrimaryFlightControl::end() noexcept
{
    return Iterator(d->primaryFlightControlData.end());
}

const PrimaryFlightControl::Iterator PrimaryFlightControl::begin() const noexcept
{
    return Iterator(d->primaryFlightControlData.begin());
}

const PrimaryFlightControl::Iterator PrimaryFlightControl::end() const noexcept
{
    return Iterator(d->primaryFlightControlData.end());
}

PrimaryFlightControl::InsertIterator PrimaryFlightControl::insertIterator() noexcept
{
    return std::inserter(d->primaryFlightControlData, d->primaryFlightControlData.begin());
}

// OPERATORS

PrimaryFlightControlData& PrimaryFlightControl::operator[](std::size_t index) noexcept
{
    return d->primaryFlightControlData[index];
}

const PrimaryFlightControlData& PrimaryFlightControl::operator[](std::size_t index) const noexcept
{
    return d->primaryFlightControlData[index];
}
