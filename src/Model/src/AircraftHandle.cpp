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
#include "AircraftHandleData.h"
#include "AircraftHandle.h"

class AircraftHandlePrivate
{
public:
    AircraftHandlePrivate() noexcept
        : currentTimestamp(TimeVariableData::InvalidTime),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    QVector<AircraftHandleData> aircraftHandleData;
    qint64 currentTimestamp;
    TimeVariableData::Access currentAccess;
    AircraftHandleData currentAircraftHandleData;
    mutable int currentIndex;

    static inline constexpr qint64 TimestampWindow = 1000; // msec
};

// PUBLIC

AircraftHandle::AircraftHandle(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<AircraftHandlePrivate>())
{
}

AircraftHandle::~AircraftHandle() noexcept
{
}

void AircraftHandle::upsert(AircraftHandleData aircraftHandleData) noexcept
{
    if (d->aircraftHandleData.count() > 0 && d->aircraftHandleData.last().timestamp == aircraftHandleData.timestamp)  {
        // Same timestamp -> replace
        d->aircraftHandleData[d->aircraftHandleData.count() - 1] = aircraftHandleData;
#ifdef DEBUG
        qDebug("AircraftHandle::upsertAircraftHandleData: UPDATE sample, timestamp: %llu count: %d", aircraftHandleData.timestamp, d->aircraftHandleData.count());
#endif
    } else {
        d->aircraftHandleData.append(aircraftHandleData);
#ifdef DEBUG
        qDebug("AircraftHandle::upsertAircraftHandleData: INSERT sample, timestamp: %llu count: %d", aircraftHandleData.timestamp, d->aircraftHandleData.count());
#endif
    }
    emit dataChanged();
}

const AircraftHandleData &AircraftHandle::getLast() const noexcept
{
    if (!d->aircraftHandleData.isEmpty()) {
        return d->aircraftHandleData.last();
    } else {
        return AircraftHandleData::NullAircraftHandleData;
    }
}

const QVector<AircraftHandleData> AircraftHandle::getAll() const noexcept
{
    return d->aircraftHandleData;
}

void AircraftHandle::clear() noexcept
{
    d->aircraftHandleData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTime;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

const AircraftHandleData &AircraftHandle::interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const AircraftHandleData *p0, *p1, *p2, *p3;

    if (d->currentTimestamp != timestamp || d->currentAccess != access) {

        double tn;
        switch (access) {
        case TimeVariableData::Access::Linear:
            if (SkySearch::getSupportData(d->aircraftHandleData, timestamp, d->currentIndex, &p0, &p1, &p2, &p3)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, timestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            d->currentIndex = SkySearch::updateStartIndex(d->aircraftHandleData, d->currentIndex, timestamp);
            if (d->currentIndex != SkySearch::InvalidIndex) {
                p1 = &d->aircraftHandleData.at(d->currentIndex);
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
            d->currentAircraftHandleData.gearHandlePosition = p1->gearHandlePosition;
            d->currentAircraftHandleData.brakeLeftPosition = SkyMath::interpolateLinear(p1->brakeLeftPosition, p2->brakeLeftPosition, tn);
            d->currentAircraftHandleData.brakeRightPosition = SkyMath::interpolateLinear(p1->brakeRightPosition, p2->brakeRightPosition, tn);
            d->currentAircraftHandleData.waterRudderHandlePosition = SkyMath::interpolateLinear(p1->waterRudderHandlePosition, p2->waterRudderHandlePosition, tn);
            d->currentAircraftHandleData.tailhookPosition = SkyMath::interpolateLinear(p1->tailhookPosition, p2->tailhookPosition, tn);
            d->currentAircraftHandleData.canopyOpen = SkyMath::interpolateLinear(p1->canopyOpen, p2->canopyOpen, tn);

            d->currentAircraftHandleData.timestamp = timestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentAircraftHandleData = AircraftHandleData::NullAircraftHandleData;
        }

        d->currentTimestamp = timestamp;
        d->currentAccess = access;
#ifdef DEBUG
    } else {
        qDebug("AircraftHandle::interpolateAircraftHandleData: cached result for timestamp: %llu", timestamp);
#endif
    }
    return d->currentAircraftHandleData;
}
