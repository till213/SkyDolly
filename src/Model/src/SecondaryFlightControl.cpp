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
#include "SecondaryFlightControlData.h"
#include "SecondaryFlightControl.h"

class SecondaryFlightControlPrivate
{
public:
    SecondaryFlightControlPrivate() noexcept
        : currentTimestamp(TimeVariableData::InvalidTimestamp),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    QVector<SecondaryFlightControlData> secondaryFlightControlData;
    qint64 currentTimestamp;
    TimeVariableData::Access currentAccess;
    SecondaryFlightControlData currentSecondaryFlightControlData;
    mutable int currentIndex;

    static inline constexpr qint64 TimestampWindow = 1000; // msec
};

// PUBLIC

SecondaryFlightControl::SecondaryFlightControl(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<SecondaryFlightControlPrivate>())
{
}

SecondaryFlightControl::~SecondaryFlightControl() noexcept
{
}

void SecondaryFlightControl::upsertSecondaryFlightControlData(SecondaryFlightControlData secondaryFlightControlData) noexcept
{
    if (d->secondaryFlightControlData.count() > 0 && d->secondaryFlightControlData.last().timestamp == secondaryFlightControlData.timestamp)  {
        // Same timestamp -> replace
        d->secondaryFlightControlData[d->secondaryFlightControlData.count() - 1] = secondaryFlightControlData;
#ifdef DEBUG
        qDebug("SecondaryFlightControl::upsertSecondaryFlightControlData: UPDATE sample, timestamp: %llu count: %d", secondaryFlightControlData.timestamp, d->secondaryFlightControlData.count());
#endif
    } else {
        d->secondaryFlightControlData.append(secondaryFlightControlData);
#ifdef DEBUG
        qDebug("SecondaryFlightControl::upsertSecondaryFlightControlData: INSERT sample, timestamp: %llu count: %d", secondaryFlightControlData.timestamp, d->secondaryFlightControlData.count());
#endif
    }
    emit dataChanged();
}

const SecondaryFlightControlData &SecondaryFlightControl::getLastSecondaryFlightControlData() const noexcept
{
    if (!d->secondaryFlightControlData.isEmpty()) {
        return d->secondaryFlightControlData.last();
    } else {
        return SecondaryFlightControlData::NullSecondaryFlightControlData;
    }
}

const QVector<SecondaryFlightControlData> SecondaryFlightControl::getAllSecondaryFlightControlData() const noexcept
{
    return d->secondaryFlightControlData;
}

void SecondaryFlightControl::clear()
{
    d->secondaryFlightControlData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTimestamp;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

const SecondaryFlightControlData &SecondaryFlightControl::interpolateSecondaryFlightControlData(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const SecondaryFlightControlData *p0, *p1, *p2, *p3;

    if (d->currentTimestamp != timestamp || d->currentAccess != access) {

        double tn;
        switch (access) {
        case TimeVariableData::Access::Linear:
            if (SkySearch::getSupportData(d->secondaryFlightControlData, timestamp, d->currentIndex, &p0, &p1, &p2, &p3)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, timestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            d->currentIndex = SkySearch::updateStartIndex(d->secondaryFlightControlData, d->currentIndex, timestamp);
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
            d->currentSecondaryFlightControlData.timestamp = timestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentSecondaryFlightControlData = SecondaryFlightControlData::NullSecondaryFlightControlData;
        }

        d->currentTimestamp = timestamp;
        d->currentAccess = access;
#ifdef DEBUG
    } else {
        qDebug("SecondaryFlightControl::interpolateSecondaryFlightControlData: cached result for timestamp: %llu", timestamp);
#endif
    }
    return d->currentSecondaryFlightControlData;
}
