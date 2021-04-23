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

#include <QObject>
#include <QVector>

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

    QVector<PrimaryFlightControlData> primaryFlightControlData;
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
{
}

PrimaryFlightControl::~PrimaryFlightControl() noexcept
{
}

void PrimaryFlightControl::upsert(PrimaryFlightControlData primaryFlightControlData) noexcept
{
    if (d->primaryFlightControlData.count() > 0 && d->primaryFlightControlData.last().timestamp == primaryFlightControlData.timestamp)  {
        // Same timestamp -> replace
        d->primaryFlightControlData[d->primaryFlightControlData.count() - 1] = primaryFlightControlData;
#ifdef DEBUG
        qDebug("PrimaryFlightControl::upsertPrimaryFlightControlData: UPDATE sample, timestamp: %llu count: %d", primaryFlightControlData.timestamp, d->primaryFlightControlData.count());
#endif
    } else {
        d->primaryFlightControlData.append(primaryFlightControlData);
#ifdef DEBUG
        qDebug("PrimaryFlightControl::upsertPrimaryFlightControlData: INSERT sample, timestamp: %llu count: %d", primaryFlightControlData.timestamp, d->primaryFlightControlData.count());
#endif
    }
    emit dataChanged();
}

const PrimaryFlightControlData &PrimaryFlightControl::getLast() const noexcept
{
    if (!d->primaryFlightControlData.isEmpty()) {
        return d->primaryFlightControlData.last();
    } else {
        return PrimaryFlightControlData::NullPrimaryFlightControlData;
    }
}

QVector<PrimaryFlightControlData> &PrimaryFlightControl::getAll() const noexcept
{
    return d->primaryFlightControlData;
}

const QVector<PrimaryFlightControlData> &PrimaryFlightControl::getAllConst() const noexcept
{
    return d->primaryFlightControlData;
}

void PrimaryFlightControl::clear() noexcept
{
    d->primaryFlightControlData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTime;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
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
            d->currentPrimaryFlightControlData = PrimaryFlightControlData::NullPrimaryFlightControlData;
        }

        d->currentTimestamp = timestamp;
        d->currentAccess = access;
#ifdef DEBUG
    } else {
        qDebug("PrimaryFlightControl::interpolatePrimaryFlightControlData: cached result for timestamp: %llu", timestamp);
#endif
    }
    return d->currentPrimaryFlightControlData;
}
