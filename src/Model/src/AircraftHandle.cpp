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
#include <algorithm>
#include <cstdint>

#include <QObject>

#include <Kernel/Settings.h>
#include <Kernel/SkyMath.h>
#include "TimeVariableData.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "AircraftHandleData.h"
#include "AircraftHandle.h"

class AircraftHandlePrivate
{
public:
    AircraftHandlePrivate(const AircraftInfo &aircraftInfo) noexcept
        : aircraftInfo(aircraftInfo),
          currentTimestamp(TimeVariableData::InvalidTime),
          currentAccess(TimeVariableData::Access::Linear),
          currentIndex(SkySearch::InvalidIndex)
    {}

    const AircraftInfo &aircraftInfo;
    std::vector<AircraftHandleData> aircraftHandleData;
    std::int64_t currentTimestamp;
    TimeVariableData::Access currentAccess;
    AircraftHandleData previousAircraftHandleData;
    AircraftHandleData currentAircraftHandleData;
    mutable int currentIndex;
};

// PUBLIC

AircraftHandle::AircraftHandle(const AircraftInfo &aircraftInfo, QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<AircraftHandlePrivate>(aircraftInfo))
{}

AircraftHandle::~AircraftHandle() noexcept
{}

void AircraftHandle::upsertLast(const AircraftHandleData &aircraftHandleData) noexcept
{
    if (d->aircraftHandleData.size() > 0 && d->aircraftHandleData.back() == aircraftHandleData)  {
        // Same timestamp -> replace
        d->aircraftHandleData[d->aircraftHandleData.size() - 1] = aircraftHandleData;
    } else {
        d->aircraftHandleData.push_back(aircraftHandleData);
    }
    emit dataChanged();
}

void AircraftHandle::upsert(const AircraftHandleData &data) noexcept
{
    auto result = std::find_if(d->aircraftHandleData.begin(), d->aircraftHandleData.end(),
                              [&data] (const TimeVariableData &d) { return d.timestamp == data.timestamp; });
    if (result != d->aircraftHandleData.end()) {
        // Same timestamp -> update
        *result = data;
    } else {
        d->aircraftHandleData.push_back(data);
    }
}

const AircraftHandleData &AircraftHandle::getFirst() const noexcept
{
    if (!d->aircraftHandleData.empty()) {
        return d->aircraftHandleData.front();
    } else {
        return AircraftHandleData::NullData;
    }
}

const AircraftHandleData &AircraftHandle::getLast() const noexcept
{
    if (!d->aircraftHandleData.empty()) {
        return d->aircraftHandleData.back();
    } else {
        return AircraftHandleData::NullData;
    }
}

std::size_t AircraftHandle::count() const noexcept
{
    return d->aircraftHandleData.size();
}

const AircraftHandleData &AircraftHandle::interpolate(std::int64_t timestamp, TimeVariableData::Access access) const noexcept
{
    const AircraftHandleData *p1, *p2;
    const std::int64_t timeOffset = access != TimeVariableData::Access::Export ? d->aircraftInfo.timeOffset : 0;
    const std::int64_t adjustedTimestamp = qMax(timestamp + timeOffset, std::int64_t(0));

    if (d->currentTimestamp != adjustedTimestamp || d->currentAccess != access) {

        double tn;
        switch (access) {
        case TimeVariableData::Access::Linear:
            [[fallthrough]];
        case TimeVariableData::Access::Export:
            if (SkySearch::getLinearInterpolationSupportData(d->aircraftHandleData, adjustedTimestamp, SkySearch::DefaultInterpolationWindow, d->currentIndex, &p1, &p2)) {
                tn = SkySearch::normaliseTimestamp(*p1, *p2, adjustedTimestamp);
            }
            break;
        case TimeVariableData::Access::Seek:
            // Get the last sample data just before the seeked position
            // (that sample point may lie far outside of the "sample window")
            d->currentIndex = SkySearch::updateStartIndex(d->aircraftHandleData, d->currentIndex, adjustedTimestamp);
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
            d->currentAircraftHandleData.brakeLeftPosition = SkyMath::interpolateLinear(p1->brakeLeftPosition, p2->brakeLeftPosition, tn);
            d->currentAircraftHandleData.brakeRightPosition = SkyMath::interpolateLinear(p1->brakeRightPosition, p2->brakeRightPosition, tn);
            d->currentAircraftHandleData.waterRudderHandlePosition = SkyMath::interpolateLinear(p1->waterRudderHandlePosition, p2->waterRudderHandlePosition, tn);
            d->currentAircraftHandleData.tailhookPosition = SkyMath::interpolateLinear(p1->tailhookPosition, p2->tailhookPosition, tn);            
            d->currentAircraftHandleData.canopyOpen = SkyMath::interpolateLinear(p1->canopyOpen, p2->canopyOpen, tn);
            // Certain aircraft override the CANOPY OPEN, so values need to be repeatedly set
            if (Settings::getInstance().isRepeatCanopyOpenEnabled()) {
                // We do that my storing the previous values (when the canopy is "open")...
                d->previousAircraftHandleData = d->currentAircraftHandleData;
            } else {
                // "Repeat values" setting disabled
                d->previousAircraftHandleData = AircraftHandleData::NullData;
            }
            d->currentAircraftHandleData.leftWingFolding = SkyMath::interpolateLinear(p1->leftWingFolding, p2->leftWingFolding, tn);
            d->currentAircraftHandleData.rightWingFolding = SkyMath::interpolateLinear(p1->rightWingFolding, p2->rightWingFolding, tn);
            d->currentAircraftHandleData.gearHandlePosition = p1->gearHandlePosition;
            d->currentAircraftHandleData.smokeEnabled = p1->smokeEnabled;

            d->currentAircraftHandleData.timestamp = adjustedTimestamp;
        } else if (!d->previousAircraftHandleData.isNull()) {
            // ... and send the previous values again (for as long as the canopy remains "open")
            d->currentAircraftHandleData = d->previousAircraftHandleData;
            d->currentAircraftHandleData.timestamp = adjustedTimestamp;
        } else {
            // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
            d->currentAircraftHandleData = AircraftHandleData::NullData;
        }

        d->currentTimestamp = adjustedTimestamp;
        d->currentAccess = access;
    }
    return d->currentAircraftHandleData;
}

void AircraftHandle::clear() noexcept
{
    d->aircraftHandleData.clear();
    d->currentTimestamp = TimeVariableData::InvalidTime;
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

AircraftHandle::Iterator AircraftHandle::begin() noexcept
{
    return d->aircraftHandleData.begin();
}

AircraftHandle::Iterator AircraftHandle::end() noexcept
{
    return Iterator(d->aircraftHandleData.end());
}

const AircraftHandle::Iterator AircraftHandle::begin() const noexcept
{
    return Iterator(d->aircraftHandleData.begin());
}

const AircraftHandle::Iterator AircraftHandle::end() const noexcept
{
    return Iterator(d->aircraftHandleData.end());
}

AircraftHandle::BackInsertIterator AircraftHandle::backInsertIterator() noexcept
{
    return std::back_inserter(d->aircraftHandleData);
}

// OPERATORS

AircraftHandleData& AircraftHandle::operator[](std::size_t index) noexcept
{
    return d->aircraftHandleData[index];
}

const AircraftHandleData& AircraftHandle::operator[](std::size_t index) const noexcept
{
    return d->aircraftHandleData[index];
}
