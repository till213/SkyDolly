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
        : currentTimestamp(TimeVariableData::InvalidTimestamp),
          currentEngineDataIndex(SkySearch::InvalidIndex)
    {}

    QVector<EngineData> engineData;
    qint64 currentTimestamp;
    EngineData currentEngineData;
    mutable int currentEngineDataIndex;
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

void Engine::upsertEngineData(EngineData engineData) noexcept
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

const EngineData &Engine::getLastEngineData() const noexcept
{
    if (!d->engineData.isEmpty()) {
        return d->engineData.last();
    } else {
        return EngineData::NullEngineData;
    }
}

const QVector<EngineData> Engine::getAllEngineData() const noexcept
{
    return d->engineData;
}

void Engine::clear()
{
    d->engineData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTimestamp;
    d->currentEngineDataIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

const EngineData &Engine::interpolateEngineData(qint64 timestamp) const noexcept
{
    const EngineData *p0, *p1, *p2, *p3;

    if (d->currentTimestamp != timestamp) {

        if (getSupportData(timestamp, &p0, &p1, &p2, &p3)) {

            double tn = normaliseTimestamp(*p1, *p2, timestamp);

            // Engine
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
#ifdef DEBUG
    } else {
        qDebug("Engine::interpolateEngineData: cached result for timestamp: %llu", timestamp);
#endif
    }
    return d->currentEngineData;
}

// PRIVATE

// Updates the current index with the last index having a timestamp <= the given timestamp
bool Engine::updateCurrentIndex(qint64 timestamp) const noexcept
{
    int index = d->currentEngineDataIndex;
    int size = d->engineData.size();
    if (size > 0 && timestamp <= d->engineData.last().timestamp) {
        if (index != SkySearch::InvalidIndex) {
            if (timestamp < d->engineData.at(index).timestamp) {
                // The timestamp was moved to front ("rewind"), so search the
                // array until and including the current index
                index = SkySearch::BinaryIntervalSearch;
            } else if (timestamp - SkySearch::BinaryIntervalSearchThreshold > d->engineData.at(index).timestamp) {
                index = SkySearch::BinaryIntervalSearch;
            }
        } else {
            // Current index not yet initialised, so search the entire array
            index = SkySearch::BinaryIntervalSearch;
        }
    } else {
        // No data yet, or timestamp not between given range
        index = SkySearch::InvalidIndex;
    }

    if (index != SkySearch::InvalidIndex) {

        // If the given timestamp lies "in the future" (as seen from the timetamp of the current index
        // the we assume that time has progressed "only a little" (normal replay) and we simply do
        // a linear search from the current index onwards
        if (index != SkySearch::BinaryIntervalSearch) {
            // Linear search: increment the current index, until we find a position having a
            // timestamp > the given timestamp
            index = SkySearch::linearIntervalSearch(d->engineData, timestamp, index);
        } else {
            // The given timestamp lies "in the past" and could really be anywwhere
            // -> binary search in the past
            int low;
            int high;
            if (d->currentEngineDataIndex != SkySearch::InvalidIndex) {
                if (timestamp < d->engineData.at(d->currentEngineDataIndex).timestamp) {
                    // Search in "the past"
                    low = 0;
                    high = d->currentEngineDataIndex;
                } else {
                    // Search in "the future"
                    low = d->currentEngineDataIndex;
                    high = d->engineData.size() - 1;
                }
            } else {
                // index not yet initialised -> search entire timeline
                low = 0;
                high = d->engineData.size() - 1;
            }
            index = SkySearch::binaryIntervalSearch(d->engineData, timestamp, low, high);
        }

    }

    d->currentEngineDataIndex = index;
    return d->currentEngineDataIndex != SkySearch::InvalidIndex;
}

bool Engine::getSupportData(qint64 timestamp, const EngineData **p0, const EngineData **p1, const EngineData **p2, const EngineData **p3) const noexcept
{
    if (updateCurrentIndex(timestamp)) {

        *p1 = &d->engineData.at(d->currentEngineDataIndex);
        if (d->currentEngineDataIndex > 0) {
           *p0 = &d->engineData.at(d->currentEngineDataIndex - 1);
        } else {
           *p0 = *p1;
        }
        if (d->currentEngineDataIndex < d->engineData.count() - 1) {
           if (d->currentEngineDataIndex < d->engineData.count() - 2) {
               *p2 = &d->engineData.at(d->currentEngineDataIndex + 1);
               *p3 = &d->engineData.at(d->currentEngineDataIndex + 2);
           } else {
               // p1 is the second to last data
               *p2 = &d->engineData.at(d->currentEngineDataIndex + 1);
               *p3 = *p2;
           }
        } else {
           // p1 is the last data
           *p2 = *p3 = *p1;
        }

    } else {
        *p0 = *p1 = *p2 = *p3 = nullptr;
    }

    return *p0 != nullptr;
}

double Engine::normaliseTimestamp(const EngineData &p1, const EngineData &p2, quint64 timestamp) noexcept
{
    double t1 = timestamp - p1.timestamp;
    double t2 = p2.timestamp - p1.timestamp;
    if (t2 != 0.0) {
        return static_cast<float>(t1) / static_cast<float>(t2);
    } else {
        // p1 and p2 are the same (last sampled) point
        return 0.0;
    }
}
