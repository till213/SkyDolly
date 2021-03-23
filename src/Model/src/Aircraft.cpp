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
#include <QByteArray>
#include <QVector>

#include "../../Kernel/src/SkyMath.h"
#include "SkySearch.h"
#include "Engine.h"
#include "AircraftInfo.h"
#include "AircraftData.h"
#include "Aircraft.h"

class AircraftPrivate
{
public:
    AircraftPrivate() noexcept
        : currentIndex(SkySearch::InvalidIndex)
    {}

    Engine engine;
    AircraftInfo aircraftInfo;

    QVector<AircraftData> aircraftData;
    AircraftData currentAircraftData;
    mutable int currentIndex;
};

// PUBLIC

Aircraft::Aircraft(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<AircraftPrivate>())
{
}

Aircraft::~Aircraft() noexcept
{
}

const Engine &Aircraft::getEngineConst() const
{
    return d->engine;
}

Engine &Aircraft::getEngine() const
{
    return d->engine;
}

void Aircraft::setAircraftInfo(AircraftInfo aircraftInfo) noexcept
{
    d->aircraftInfo = aircraftInfo;
    emit infoChanged();
}

const AircraftInfo &Aircraft::getAircraftInfo() const noexcept
{
    return d->aircraftInfo;
}

void Aircraft::upsertAircraftData(AircraftData aircraftData) noexcept
{
    if (d->aircraftData.count() > 0 && d->aircraftData.last().timestamp == aircraftData.timestamp)  {
        // Same timestamp -> replace
        d->aircraftData[d->aircraftData.count() - 1] = aircraftData;
#ifdef DEBUG
        qDebug("Aircraft::upsertAircraftData: UPDATE sample, timestamp: %llu count: %d", aircraftData.timestamp, d->aircraftData.count());
#endif
    } else {
        d->aircraftData.append(aircraftData);
#ifdef DEBUG
        qDebug("Aircraft::upsertAircraftData: INSERT sample, timestamp: %llu count: %d", aircraftData.timestamp, d->aircraftData.count());
#endif
    }
    emit dataChanged();
}

const AircraftData &Aircraft::getLastAircraftData() const noexcept
{
    if (!d->aircraftData.isEmpty()) {
        return d->aircraftData.last();
    } else {
        return AircraftData::NullAircraftData;
    }
}

const QVector<AircraftData> Aircraft::getAllAircraftData() const noexcept
{
    return d->aircraftData;
}

void Aircraft::clear()
{
    d->aircraftData.clear();
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

const AircraftData &Aircraft::interpolateAircraftData(qint64 timestamp) const noexcept
{
    const AircraftData *p0, *p1, *p2, *p3;
    const double Tension = 0.0;

    if (getSupportData(timestamp, &p0, &p1, &p2, &p3)) {

        double tn = normaliseTimestamp(*p1, *p2, timestamp);

        // Aircraft position

        // Latitude: [-90, 90] - no discontinuity at +/- 90
        d->currentAircraftData.latitude  = SkyMath::interpolateHermite(p0->latitude, p1->latitude, p2->latitude, p3->latitude, tn, Tension);
        // Longitude: [-180, 180] - discontinuity at the +/- 180 meridian
        d->currentAircraftData.longitude = SkyMath::interpolateHermite180(p0->longitude, p1->longitude, p2->longitude, p3->longitude, tn, Tension);
        // Altitude [open range]
        d->currentAircraftData.altitude  = SkyMath::interpolateHermite(p0->altitude, p1->altitude, p2->altitude, p3->altitude, tn, Tension);

        // Pitch: [-90, 90] - no discontinuity at +/- 90
        d->currentAircraftData.pitch = SkyMath::interpolateHermite(p0->pitch, p1->pitch, p2->pitch, p3->pitch, tn, Tension);
        // Bank: [-180, 180] - discontinuity at +/- 180
        d->currentAircraftData.bank  = SkyMath::interpolateHermite180(p0->bank, p1->bank, p2->bank, p3->bank, tn, Tension);
        // Heading: [0, 360] - discontinuity at 0/360
        d->currentAircraftData.heading = SkyMath::interpolateHermite360(p0->heading, p1->heading, p2->heading, p3->heading, tn, Tension);

        // Velocity
        d->currentAircraftData.velocityBodyX = SkyMath::interpolateLinear(p1->velocityBodyX, p2->velocityBodyX, tn);
        d->currentAircraftData.velocityBodyY = SkyMath::interpolateLinear(p1->velocityBodyY, p2->velocityBodyY, tn);
        d->currentAircraftData.velocityBodyZ = SkyMath::interpolateLinear(p1->velocityBodyZ, p2->velocityBodyZ, tn);
        d->currentAircraftData.rotationVelocityBodyX = SkyMath::interpolateLinear(p1->rotationVelocityBodyX, p2->rotationVelocityBodyX, tn);
        d->currentAircraftData.rotationVelocityBodyY = SkyMath::interpolateLinear(p1->rotationVelocityBodyY, p2->rotationVelocityBodyY, tn);
        d->currentAircraftData.rotationVelocityBodyZ = SkyMath::interpolateLinear(p1->rotationVelocityBodyZ, p2->rotationVelocityBodyZ, tn);

        // Aircraft controls
        d->currentAircraftData.yokeXPosition = SkyMath::interpolateLinear(p1->yokeXPosition, p2->yokeXPosition, tn);
        d->currentAircraftData.yokeYPosition = SkyMath::interpolateLinear(p1->yokeYPosition, p2->yokeYPosition, tn);
        d->currentAircraftData.rudderPosition = SkyMath::interpolateLinear(p1->rudderPosition, p2->rudderPosition, tn);
        d->currentAircraftData.elevatorPosition = SkyMath::interpolateLinear(p1->elevatorPosition, p2->elevatorPosition, tn);
        d->currentAircraftData.aileronPosition = SkyMath::interpolateLinear(p1->aileronPosition, p2->aileronPosition, tn);

        // Flaps & spoilers
        d->currentAircraftData.leadingEdgeFlapsLeftPercent = SkyMath::interpolateLinear(p1->leadingEdgeFlapsLeftPercent, p2->leadingEdgeFlapsLeftPercent, tn);
        d->currentAircraftData.leadingEdgeFlapsRightPercent = SkyMath::interpolateLinear(p1->leadingEdgeFlapsRightPercent, p2->leadingEdgeFlapsRightPercent, tn);
        d->currentAircraftData.trailingEdgeFlapsLeftPercent = SkyMath::interpolateLinear(p1->trailingEdgeFlapsLeftPercent, p2->trailingEdgeFlapsLeftPercent, tn);
        d->currentAircraftData.trailingEdgeFlapsRightPercent = SkyMath::interpolateLinear(p1->trailingEdgeFlapsRightPercent, p2->trailingEdgeFlapsRightPercent, tn);
        d->currentAircraftData.spoilersHandlePosition = SkyMath::interpolateLinear(p1->spoilersHandlePosition, p2->spoilersHandlePosition, tn);

        // No interpolation for flaps position and gear
        d->currentAircraftData.flapsHandleIndex = p1->flapsHandleIndex;
        d->currentAircraftData.gearHandlePosition = p1->gearHandlePosition;
        d->currentAircraftData.brakeLeftPosition = SkyMath::interpolateLinear(p1->brakeLeftPosition, p2->brakeLeftPosition, tn);
        d->currentAircraftData.brakeRightPosition = SkyMath::interpolateLinear(p1->brakeRightPosition, p2->brakeRightPosition, tn);
        d->currentAircraftData.waterRudderHandlePosition = SkyMath::interpolateLinear(p1->waterRudderHandlePosition, p2->waterRudderHandlePosition, tn);
        d->currentAircraftData.tailhookPosition = SkyMath::interpolateLinear(p1->tailhookPosition, p2->tailhookPosition, tn);
        d->currentAircraftData.canopyOpen = SkyMath::interpolateLinear(p1->canopyOpen, p2->canopyOpen, tn);

        // No interpolation for light states
        d->currentAircraftData.lightStates = p1->lightStates;

        d->currentAircraftData.timestamp = timestamp;

    } else {
        // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
        d->currentAircraftData = AircraftData::NullAircraftData;
    }
    return d->currentAircraftData;
}

// PRIVATE

// Updates the current index with the last index having a timestamp <= the given timestamp
bool Aircraft::updateCurrentIndex(qint64 timestamp) const noexcept
{
    int index = d->currentIndex;
    int size = d->aircraftData.size();
    if (size > 0 && timestamp <= d->aircraftData.last().timestamp) {
        if (index != SkySearch::InvalidIndex) {
            if (timestamp < d->aircraftData.at(index).timestamp) {
                // The timestamp was moved to front ("rewind"), so search the
                // array until and including the current index
                index = SkySearch::BinaryIntervalSearch;
            } else if (timestamp - SkySearch::BinaryIntervalSearchThreshold > d->aircraftData.at(index).timestamp) {
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
            index = SkySearch::linearIntervalSearch(d->aircraftData, timestamp, index);
        } else {
            // The given timestamp lies "in the past" and could really be anywwhere
            // -> binary search in the past
            int low;
            int high;
            if (d->currentIndex != SkySearch::InvalidIndex) {
                if (timestamp < d->aircraftData.at(d->currentIndex).timestamp) {
                    // Search in "the past"
                    low = 0;
                    high = d->currentIndex;
                } else {
                    // Search in "the future"
                    low = d->currentIndex;
                    high = d->aircraftData.size() - 1;
                }
            } else {
                // index not yet initialised -> search entire timeline
                low = 0;
                high = d->aircraftData.size() - 1;
            }
            index = SkySearch::binaryIntervalSearch(d->aircraftData, timestamp, low, high);
        }

    }

    d->currentIndex = index;
    return d->currentIndex != SkySearch::InvalidIndex;
}

bool Aircraft::getSupportData(qint64 timestamp, const AircraftData **p0, const AircraftData **p1, const AircraftData **p2, const AircraftData **p3) const noexcept
{
    if (updateCurrentIndex(timestamp)) {

        *p1 = &d->aircraftData.at(d->currentIndex);
        if (d->currentIndex > 0) {
           *p0 = &d->aircraftData.at(d->currentIndex - 1);
        } else {
           *p0 = *p1;
        }
        if (d->currentIndex < d->aircraftData.count() - 1) {
           if (d->currentIndex < d->aircraftData.count() - 2) {
               *p2 = &d->aircraftData.at(d->currentIndex + 1);
               *p3 = &d->aircraftData.at(d->currentIndex + 2);
           } else {
               // p1 is the second to last data
               *p2 = &d->aircraftData.at(d->currentIndex + 1);
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

double Aircraft::normaliseTimestamp(const AircraftData &p1, const AircraftData &p2, quint64 timestamp) noexcept
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
