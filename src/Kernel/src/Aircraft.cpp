#include <QObject>
#include <QByteArray>
#include <QVector>

#include "SkyMath.h"
#include "AircraftInfo.h"
#include "AircraftData.h"
#include "Aircraft.h"

namespace {
    const int InvalidIndex = -1;
}

class AircraftPrivate
{
public:
    AircraftPrivate()
        : currentIndex(::InvalidIndex)
    {}

    AircraftInfo aircraftInfo;
    QVector<AircraftData> aircraftData;
    AircraftData currentAircraftData;
    mutable int currentIndex;
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
    d->currentIndex = ::InvalidIndex;
    emit dataChanged();
}

const AircraftData &Aircraft::getAllAircraftData(qint64 timestamp) const
{
    const AircraftData *p0, *p1, *p2, *p3;
    const double Tension = 0.0;

    if (this->getSupportData(timestamp, &p0, &p1, &p2, &p3)) {

        double tn = this->normaliseTimestamp(*p1, *p2, timestamp);

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
          static_cast<double>(p0->yokeXPosition),
          static_cast<double>(p1->yokeXPosition),
          static_cast<double>(p2->yokeXPosition),
          static_cast<double>(p3->yokeXPosition), tn, Tension);
        d->currentAircraftData.yokeYPosition = SkyMath::interpolateHermite(
          static_cast<double>(p0->yokeYPosition),
          static_cast<double>(p1->yokeYPosition),
          static_cast<double>(p2->yokeYPosition),
          static_cast<double>(p3->yokeYPosition), tn, Tension);
        d->currentAircraftData.rudderPosition = SkyMath::interpolateHermite(
          static_cast<double>(p0->rudderPosition),
          static_cast<double>(p1->rudderPosition),
          static_cast<double>(p2->rudderPosition),
          static_cast<double>(p3->rudderPosition), tn, Tension);
        d->currentAircraftData.elevatorPosition = SkyMath::interpolateHermite(
          static_cast<double>(p0->elevatorPosition),
          static_cast<double>(p1->elevatorPosition),
          static_cast<double>(p2->elevatorPosition),
          static_cast<double>(p3->elevatorPosition), tn, Tension);
        d->currentAircraftData.aileronPosition = SkyMath::interpolateHermite(
          static_cast<double>(p0->aileronPosition),
          static_cast<double>(p1->aileronPosition),
          static_cast<double>(p2->aileronPosition),
          static_cast<double>(p3->aileronPosition), tn, Tension);

        // Engine
        d->currentAircraftData.throttleLeverPosition1 = SkyMath::interpolateHermite(
          static_cast<double>(p0->throttleLeverPosition1),
          static_cast<double>(p1->throttleLeverPosition1),
          static_cast<double>(p2->throttleLeverPosition1),
          static_cast<double>(p3->throttleLeverPosition1), tn, Tension);
        d->currentAircraftData.throttleLeverPosition2 = SkyMath::interpolateHermite(
          static_cast<double>(p0->throttleLeverPosition2),
          static_cast<double>(p1->throttleLeverPosition2),
          static_cast<double>(p2->throttleLeverPosition2),
          static_cast<double>(p3->throttleLeverPosition2), tn, Tension);
        d->currentAircraftData.throttleLeverPosition3 = SkyMath::interpolateHermite(
          static_cast<double>(p0->throttleLeverPosition3),
          static_cast<double>(p1->throttleLeverPosition3),
          static_cast<double>(p2->throttleLeverPosition3),
          static_cast<double>(p3->throttleLeverPosition3), tn, Tension);
        d->currentAircraftData.throttleLeverPosition4 = SkyMath::interpolateHermite(
          static_cast<double>(p0->throttleLeverPosition4),
          static_cast<double>(p1->throttleLeverPosition4),
          static_cast<double>(p2->throttleLeverPosition4),
          static_cast<double>(p3->throttleLeverPosition4), tn, Tension);

        d->currentAircraftData.spoilersHandlePosition = SkyMath::interpolateHermite(
          static_cast<double>(p0->spoilersHandlePosition),
          static_cast<double>(p1->spoilersHandlePosition),
          static_cast<double>(p2->spoilersHandlePosition),
          static_cast<double>(p3->spoilersHandlePosition), tn, Tension);

        // No interpolation for flaps position and gear
        d->currentAircraftData.flapsHandleIndex = p1->flapsHandleIndex;
        d->currentAircraftData.gearHandlePosition = p1->gearHandlePosition;

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
    int length = d->aircraftData.length();
    if (length > 0 && timestamp <= d->aircraftData.last().timestamp) {
        if (index != ::InvalidIndex) {
            if (d->aircraftData.at(d->currentIndex).timestamp > timestamp) {
                // The timestamp was moved to front ("rewind"), so start searching from beginning
                // @todo binary search (O(log n)
                index = 0;
            }
        } else {
            // Start from beginning (O(n))
            // @todo binary search (O(log n)
            index = 0;
        }        
    } else {
        // No data yet, or timestamp not between given range
        index = ::InvalidIndex;
    }

    if (index != ::InvalidIndex) {
        // Increment the current index, until we find a position having a timestamp > the given timestamp
        bool found = false;
        while (!found && index < length) {
            if (index < (length - 1)) {
                if (d->aircraftData.at(index + 1).timestamp > timestamp) {
                    // The next index has a larger timestamp, so this index is the one we are looking for
                    found = true;
                } else {
                    ++index;
                }
            } else {
                // Reached the last index
                found = true;
            }
        }
    }

    d->currentIndex = index;
    return d->currentIndex != ::InvalidIndex;
}

bool Aircraft::getSupportData(qint64 timestamp, const AircraftData **p0, const AircraftData **p1, const AircraftData **p2, const AircraftData **p3) const
{
    if (this->updateCurrentIndex(timestamp)) {

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
