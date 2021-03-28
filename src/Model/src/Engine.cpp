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
#include "EngineData.h"
#include "Engine.h"

class EnginePrivate
{
public:
    EnginePrivate() noexcept
        : currentTimestamp(TimeVariableData::InvalidTime),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    QVector<EngineData> engineData;
    qint64 currentTimestamp;
    TimeVariableData::Access currentAccess;
    EngineData currentEngineData;
    mutable int currentIndex;

    static inline constexpr qint64 TimestampWindow = 1000; // msec
};

// PUBLIC

Engine::Engine(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<EnginePrivate>())
{
}

Engine::~Engine() noexcept
{
}

void Engine::upsert(EngineData engineData) noexcept
{
    if (d->engineData.count() > 0 && d->engineData.last().timestamp == engineData.timestamp)  {
        // Same timestamp -> replace
        d->engineData[d->engineData.count() - 1] = engineData;
#ifdef DEBUG
        qDebug("Engine::upsertEngineData: UPDATE sample, timestamp: %llu count: %d", engineData.timestamp, d->engineData.count());
#endif
    } else {
        d->engineData.append(engineData);
#ifdef DEBUG
        qDebug("Engine::upsertEngineData: INSERT sample, timestamp: %llu count: %d", engineData.timestamp, d->engineData.count());
#endif
    }
    emit dataChanged();
}

const EngineData &Engine::getLast() const noexcept
{
    if (!d->engineData.isEmpty()) {
        return d->engineData.last();
    } else {
        return EngineData::NullEngineData;
    }
}

const QVector<EngineData> Engine::getAll() const noexcept
{
    return d->engineData;
}

void Engine::clear() noexcept
{
    d->engineData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTime;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

const EngineData &Engine::interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const EngineData *p0, *p1, *p2, *p3;

    if (d->currentTimestamp != timestamp || d->currentAccess != access) {

        double tn;
        switch (access) {
        case TimeVariableData::Access::Linear:
            if (SkySearch::getSupportData(d->engineData, timestamp, d->currentIndex, &p0, &p1, &p2, &p3)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, timestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            d->currentIndex = SkySearch::updateStartIndex(d->engineData, d->currentIndex, timestamp);
            if (d->currentIndex != SkySearch::InvalidIndex) {
                p1 = &d->engineData.at(d->currentIndex);
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
            d->currentEngineData.throttleLeverPosition1 = SkyMath::interpolateLinear(p1->throttleLeverPosition1, p2->throttleLeverPosition1, tn);
            d->currentEngineData.throttleLeverPosition2 = SkyMath::interpolateLinear(p1->throttleLeverPosition2, p2->throttleLeverPosition2, tn);
            d->currentEngineData.throttleLeverPosition3 = SkyMath::interpolateLinear(p1->throttleLeverPosition3, p2->throttleLeverPosition3, tn);
            d->currentEngineData.throttleLeverPosition4 = SkyMath::interpolateLinear(p1->throttleLeverPosition4, p2->throttleLeverPosition4, tn);
            d->currentEngineData.propellerLeverPosition1 = SkyMath::interpolateLinear(p1->propellerLeverPosition1, p2->propellerLeverPosition1, tn);
            d->currentEngineData.propellerLeverPosition2 = SkyMath::interpolateLinear(p1->propellerLeverPosition2, p2->propellerLeverPosition2, tn);
            d->currentEngineData.propellerLeverPosition3 = SkyMath::interpolateLinear(p1->propellerLeverPosition3, p2->propellerLeverPosition3, tn);
            d->currentEngineData.propellerLeverPosition4 = SkyMath::interpolateLinear(p1->propellerLeverPosition4, p2->propellerLeverPosition4, tn);
            d->currentEngineData.mixtureLeverPosition1 = SkyMath::interpolateLinear(p1->mixtureLeverPosition1, p2->mixtureLeverPosition1, tn);
            d->currentEngineData.mixtureLeverPosition2 = SkyMath::interpolateLinear(p1->mixtureLeverPosition2, p2->mixtureLeverPosition2, tn);
            d->currentEngineData.mixtureLeverPosition3 = SkyMath::interpolateLinear(p1->mixtureLeverPosition3, p2->mixtureLeverPosition3, tn);
            d->currentEngineData.mixtureLeverPosition4 = SkyMath::interpolateLinear(p1->mixtureLeverPosition4, p2->mixtureLeverPosition4, tn);
            d->currentEngineData.timestamp = timestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentEngineData = EngineData::NullEngineData;
        }

        d->currentTimestamp = timestamp;
        d->currentAccess = access;
#ifdef DEBUG
    } else {
        qDebug("Engine::interpolateEngineData: cached result for timestamp: %llu", timestamp);
#endif
    }
    return d->currentEngineData;
}
