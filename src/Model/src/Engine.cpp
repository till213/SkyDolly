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

#include "../../Kernel/src/SkyMath.h"
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "EngineData.h"
#include "Engine.h"

class EnginePrivate
{
public:
    EnginePrivate(const AircraftInfo &aircraftInfo) noexcept
        : aircraftInfo(aircraftInfo),
          currentTimestamp(TimeVariableData::InvalidTime),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    const AircraftInfo &aircraftInfo;
    std::vector<EngineData> engineData;
    qint64 currentTimestamp;
    TimeVariableData::Access currentAccess;
    EngineData currentEngineData;
    mutable int currentIndex;
};

// PUBLIC

Engine::Engine(const AircraftInfo &aircraftInfo, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<EnginePrivate>(aircraftInfo))
{}

Engine::~Engine() noexcept
{}

void Engine::upsert(const EngineData &engineData) noexcept
{
    if (d->engineData.size() > 0 && d->engineData.back().timestamp == engineData.timestamp)  {
        // Same timestamp -> replace
        d->engineData[d->engineData.size() - 1] = engineData;
    } else {
        d->engineData.push_back(engineData);
    }
    emit dataChanged();
}

const EngineData &Engine::getFirst() const noexcept
{
    if (!d->engineData.empty()) {
        return d->engineData.front();
    } else {
        return EngineData::NullData;
    }
}

const EngineData &Engine::getLast() const noexcept
{
    if (!d->engineData.empty()) {
        return d->engineData.back();
    } else {
        return EngineData::NullData;
    }
}

std::size_t Engine::count() const noexcept
{
    return d->engineData.size();
}

const EngineData &Engine::interpolate(qint64 timestamp, TimeVariableData::Access access) const noexcept
{
    const EngineData *p1, *p2;
    const qint64 adjustedTimestamp = qMax(timestamp + d->aircraftInfo.timeOffset, 0LL);

    if (d->currentTimestamp != timestamp || d->currentAccess != access) {

        double tn;
        switch (access) {
        case TimeVariableData::Access::Linear:
            if (SkySearch::getLinearInterpolationSupportData(d->engineData, adjustedTimestamp, SkySearch::DefaultInterpolationWindow, d->currentIndex, &p1, &p2)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            d->currentIndex = SkySearch::updateStartIndex(d->engineData, d->currentIndex, adjustedTimestamp);
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
            d->currentEngineData.cowlFlapPosition1 = SkyMath::interpolateLinear(p1->cowlFlapPosition1, p2->cowlFlapPosition1, tn);
            d->currentEngineData.cowlFlapPosition2 = SkyMath::interpolateLinear(p1->cowlFlapPosition2, p2->cowlFlapPosition2, tn);
            d->currentEngineData.cowlFlapPosition3 = SkyMath::interpolateLinear(p1->cowlFlapPosition3, p2->cowlFlapPosition3, tn);
            d->currentEngineData.cowlFlapPosition4 = SkyMath::interpolateLinear(p1->cowlFlapPosition4, p2->cowlFlapPosition4, tn);

            // No interpolation for battery and starter/combustion states (boolean)
            d->currentEngineData.electricalMasterBattery1 = p1->electricalMasterBattery1;
            d->currentEngineData.electricalMasterBattery2 = p1->electricalMasterBattery2;
            d->currentEngineData.electricalMasterBattery3 = p1->electricalMasterBattery3;
            d->currentEngineData.electricalMasterBattery4 = p1->electricalMasterBattery4;
            d->currentEngineData.generalEngineStarter1 = p1->generalEngineStarter1;
            d->currentEngineData.generalEngineStarter2 = p1->generalEngineStarter2;
            d->currentEngineData.generalEngineStarter3 = p1->generalEngineStarter3;
            d->currentEngineData.generalEngineStarter4 = p1->generalEngineStarter4;
            d->currentEngineData.generalEngineCombustion1 = p1->generalEngineCombustion1;
            d->currentEngineData.generalEngineCombustion2 = p1->generalEngineCombustion2;
            d->currentEngineData.generalEngineCombustion3 = p1->generalEngineCombustion3;
            d->currentEngineData.generalEngineCombustion4 = p1->generalEngineCombustion4;

            d->currentEngineData.timestamp = adjustedTimestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentEngineData = EngineData::NullData;
        }

        d->currentTimestamp = adjustedTimestamp;
        d->currentAccess = access;
    }
    return d->currentEngineData;
}

void Engine::clear() noexcept
{
    d->engineData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTime;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

Engine::Iterator Engine::begin() noexcept
{
    return d->engineData.begin();
}

Engine::Iterator Engine::end() noexcept
{
    return Iterator(d->engineData.end());
}

const Engine::Iterator Engine::begin() const noexcept
{
    return Iterator(d->engineData.begin());
}

const Engine::Iterator Engine::end() const noexcept
{
    return Iterator(d->engineData.end());
}

Engine::InsertIterator Engine::insertIterator() noexcept
{
    return std::inserter(d->engineData, d->engineData.begin());
}

// OPERATORS

EngineData& Engine::operator[](std::size_t index) noexcept
{
    return d->engineData[index];
}

const EngineData& Engine::operator[](std::size_t index) const noexcept
{
    return d->engineData[index];
}
