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
#include <QObject>
#include <QByteArray>
#include <QVector>

#include "SkyMath.h"
#include "SkySearch.h"
#include "AircraftInfo.h"
#include "AircraftData.h"
#include "Aircraft.h"

class AircraftPrivate
{
public:
    AircraftPrivate()
        : currentIndex(SkySearch::InvalidIndex)
    {}

    AircraftInfo aircraftInfo;
    QVector<AircraftData> aircraftData;
    AircraftData currentAircraftData;
    mutable int currentIndex;

    // In case we seek 3 seconds "into the future" we use binary search
    // to find the next position (otherwise linear search, assuming that
    // the next position is "nearby" (within the 3 seconds threshold)
    static constexpr qint64 BinaryIntervalSearchThreshold = 3000;
};

// PUBLIC

Aircraft::Aircraft(QObject *parent)
    : QObject(parent),
      d(new AircraftPrivate())
{
}

Aircraft::~Aircraft()
{
    delete d;
}

void Aircraft::setAircraftInfo(AircraftInfo aircraftInfo)
{
    d->aircraftInfo = aircraftInfo;
    emit infoChanged();
}

const AircraftInfo &Aircraft::getAircraftInfo() const
{
    return d->aircraftInfo;
}

void Aircraft::upsertAircraftData(AircraftData aircraftData)
{
    if (d->aircraftData.length() > 0 && d->aircraftData.last().timestamp == aircraftData.timestamp)  {
        // Same timestamp -> replace
        d->aircraftData[d->aircraftData.length() - 1] = aircraftData;
    } else {
        d->aircraftData.append(aircraftData);
    }
    emit dataChanged();
}

const AircraftData &Aircraft::getLastAircraftData() const
{
    if (!d->aircraftData.isEmpty()) {
        return d->aircraftData.last();
    } else {
        return AircraftData::NullAircraftData;
    }
}

const QVector<AircraftData> Aircraft::getAllAircraftData() const
{
    return d->aircraftData;
}

void Aircraft::clear()
{
    d->aircraftData.clear();
    d->currentIndex = SkySearch::InvalidIndex;
    emit dataChanged();
}

const AircraftData &Aircraft::getAllAircraftData(qint64 timestamp) const
{
    const AircraftData *p0, *p1, *p2, *p3;
    const double Tension = 0.0;

    if (getSupportData(timestamp, &p0, &p1, &p2, &p3)) {

        double tn = normaliseTimestamp(*p1, *p2, timestamp);

        // Aircraft position

        // Latitude: [-90, 90]
        d->currentAircraftData.latitude  = SkyMath::interpolateHermite180(p0->latitude, p1->latitude, p2->latitude, p3->latitude, tn, Tension);
        // Longitude: [-180, 180]
        d->currentAircraftData.longitude = SkyMath::interpolateHermite180(p0->longitude, p1->longitude, p2->longitude, p3->longitude, tn, Tension);
        // Altitude [open range]
        d->currentAircraftData.altitude  = SkyMath::interpolateHermite(p0->altitude, p1->altitude, p2->altitude, p3->altitude, tn, Tension);

        // Pitch: [-90, 90]
        d->currentAircraftData.pitch = SkyMath::interpolateHermite180(p0->pitch, p1->pitch, p2->pitch, p3->pitch, tn, Tension);
        // Bank: [-180, 180]
        d->currentAircraftData.bank  = SkyMath::interpolateHermite180(p0->bank, p1->bank, p2->bank, p3->bank, tn, Tension);
        // Heading: [0, 360]
        d->currentAircraftData.heading = SkyMath::interpolateHermite360(p0->heading, p1->heading, p2->heading, p3->heading, tn, Tension);

        // Aircraft controls
        d->currentAircraftData.yokeXPosition = SkyMath::interpolateHermite(
          p0->yokeXPosition,
          p1->yokeXPosition,
          p2->yokeXPosition,
          p3->yokeXPosition, tn, Tension);
        d->currentAircraftData.yokeYPosition = SkyMath::interpolateHermite(
          p0->yokeYPosition,
          p1->yokeYPosition,
          p2->yokeYPosition,
          p3->yokeYPosition, tn, Tension);
        d->currentAircraftData.rudderPosition = SkyMath::interpolateHermite(
          p0->rudderPosition,
          p1->rudderPosition,
          p2->rudderPosition,
          p3->rudderPosition, tn, Tension);
        d->currentAircraftData.elevatorPosition = SkyMath::interpolateHermite(
          p0->elevatorPosition,
          p1->elevatorPosition,
          p2->elevatorPosition,
          p3->elevatorPosition, tn, Tension);
        d->currentAircraftData.aileronPosition = SkyMath::interpolateHermite(
          p0->aileronPosition,
          p1->aileronPosition,
          p2->aileronPosition,
          p3->aileronPosition, tn, Tension);

        // Engine
        d->currentAircraftData.throttleLeverPosition1 = SkyMath::interpolateHermite(
          p0->throttleLeverPosition1,
          p1->throttleLeverPosition1,
          p2->throttleLeverPosition1,
          p3->throttleLeverPosition1, tn, Tension);
        d->currentAircraftData.throttleLeverPosition2 = SkyMath::interpolateHermite(
          p0->throttleLeverPosition2,
          p1->throttleLeverPosition2,
          p2->throttleLeverPosition2,
          p3->throttleLeverPosition2, tn, Tension);
        d->currentAircraftData.throttleLeverPosition3 = SkyMath::interpolateHermite(
          p0->throttleLeverPosition3,
          p1->throttleLeverPosition3,
          p2->throttleLeverPosition3,
          p3->throttleLeverPosition3, tn, Tension);
        d->currentAircraftData.throttleLeverPosition4 = SkyMath::interpolateHermite(
          p0->throttleLeverPosition4,
          p1->throttleLeverPosition4,
          p2->throttleLeverPosition4,
          p3->throttleLeverPosition4, tn, Tension);

        // Flaps & spoilers
        d->currentAircraftData.leadingEdgeFlapsLeftPercent = SkyMath::interpolateHermite(
          p0->leadingEdgeFlapsLeftPercent,
          p1->leadingEdgeFlapsLeftPercent,
          p2->leadingEdgeFlapsLeftPercent,
          p3->leadingEdgeFlapsLeftPercent, tn, Tension);
        d->currentAircraftData.leadingEdgeFlapsRightPercent = SkyMath::interpolateHermite(
          p0->leadingEdgeFlapsRightPercent,
          p1->leadingEdgeFlapsRightPercent,
          p2->leadingEdgeFlapsRightPercent,
          p3->leadingEdgeFlapsRightPercent, tn, Tension);
        d->currentAircraftData.trailingEdgeFlapsLeftPercent = SkyMath::interpolateHermite(
          p0->trailingEdgeFlapsLeftPercent,
          p1->trailingEdgeFlapsLeftPercent,
          p2->trailingEdgeFlapsLeftPercent,
          p3->trailingEdgeFlapsLeftPercent, tn, Tension);
        d->currentAircraftData.trailingEdgeFlapsRightPercent = SkyMath::interpolateHermite(
          p0->trailingEdgeFlapsRightPercent,
          p1->trailingEdgeFlapsRightPercent,
          p2->trailingEdgeFlapsRightPercent,
          p3->trailingEdgeFlapsRightPercent, tn, Tension);
        d->currentAircraftData.spoilersHandlePosition = SkyMath::interpolateHermite(
          p0->spoilersHandlePosition,
          p1->spoilersHandlePosition,
          p2->spoilersHandlePosition,
          p3->spoilersHandlePosition, tn, Tension);

        // No interpolation for flaps position and gear
        d->currentAircraftData.flapsHandleIndex = p1->flapsHandleIndex;
        d->currentAircraftData.gearHandlePosition = p1->gearHandlePosition;

        d->currentAircraftData.waterRudderHandlePosition = SkyMath::interpolateHermite(
          p0->waterRudderHandlePosition,
          p1->waterRudderHandlePosition,
          p2->waterRudderHandlePosition,
          p3->waterRudderHandlePosition, tn, Tension);
        d->currentAircraftData.brakeLeftPosition = SkyMath::interpolateHermite(
          p0->brakeLeftPosition,
          p1->brakeLeftPosition,
          p2->brakeLeftPosition,
          p3->brakeLeftPosition, tn, Tension);
        d->currentAircraftData.brakeRightPosition = SkyMath::interpolateHermite(
          p0->brakeRightPosition,
          p1->brakeRightPosition,
          p2->brakeRightPosition,
          p3->brakeRightPosition, tn, Tension);

        d->currentAircraftData.timestamp = timestamp;

    } else {
        // No recorded data, or the timestamp exceeds the timestamp of the last recorded position
        d->currentAircraftData = AircraftData::NullAircraftData;
    }
    return d->currentAircraftData;
}

// PRIVATE

// Updates the current index with the last index having a timestamp <= the given timestamp
bool Aircraft::updateCurrentIndex(qint64 timestamp) const
{
    int index = d->currentIndex;
    int size = d->aircraftData.size();
    if (size > 0 && timestamp <= d->aircraftData.last().timestamp) {
        if (index != SkySearch::InvalidIndex) {
            if (timestamp < d->aircraftData.at(index).timestamp) {
                // The timestamp was moved to front ("rewind"), so search the
                // array until and including the current index
                index = SkySearch::BinaryIntervalSearch;
            } else if (timestamp - AircraftPrivate::BinaryIntervalSearchThreshold > d->aircraftData.at(index).timestamp) {
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

bool Aircraft::getSupportData(qint64 timestamp, const AircraftData **p0, const AircraftData **p1, const AircraftData **p2, const AircraftData **p3) const
{
    if (updateCurrentIndex(timestamp)) {

        *p1 = &d->aircraftData.at(d->currentIndex);
        if (d->currentIndex > 0) {
           *p0 = &d->aircraftData.at(d->currentIndex - 1);
        } else {
           *p0 = *p1;
        }
        if (d->currentIndex < d->aircraftData.length() - 1) {
           if (d->currentIndex < d->aircraftData.length() - 2) {
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

double Aircraft::normaliseTimestamp(const AircraftData &p1, const AircraftData &p2, quint64 timestamp)
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
