#include <QObject>
#include <QByteArray>
#include <QVector>

#include "SkyMath.h"
#include "AircraftInfo.h"
#include "Position.h"
#include "Aircraft.h"

namespace {
    const int InvalidIndex = -1;
}

class AircraftPrivate
{
public:
    AircraftPrivate()
        : startOnGround(false),
          currentIndex(::InvalidIndex)
    {}

    AircraftInfo aircraftInfo;
    QVector<Position> positions;
    Position position;
    QByteArray name;
    bool startOnGround;
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

void Aircraft::appendPosition(Position position)
{
    if (d->positions.length() > 0 && d->positions.last().timestamp == position.timestamp)  {
        // Same timestamp -> replace
        d->positions[d->positions.length() - 1] = position;
    } else {
        d->positions.append(position);
    }
    emit positionChanged();
}

const Position &Aircraft::getLastPosition() const
{
    if (!d->positions.isEmpty()) {
        return d->positions.last();
    } else {
        return Position::NullPosition;
    }
}

const QVector<Position> Aircraft::getPositions() const
{
    return d->positions;
}

void Aircraft::clear()
{
    d->positions.clear();
    d->currentIndex = ::InvalidIndex;
    emit positionChanged();
}

const Position &Aircraft::getPosition(qint64 timestamp) const
{
    const Position *p0, *p1, *p2, *p3;

    if (this->getSupportPositions(timestamp, &p0, &p1, &p2, &p3)) {

        double tn = this->normaliseTimestamp(*p1, *p2, timestamp);

        // Latitude: [-90, 90]
        d->position.latitude  = SkyMath::interpolateHermite180(p0->latitude, p1->latitude, p2->latitude, p3->latitude, tn);
        // Longitude: [-180, 180]
        d->position.longitude = SkyMath::interpolateHermite180(p0->longitude, p1->longitude, p2->longitude, p3->longitude, tn);
        // Altitude [open range]
        d->position.altitude  = SkyMath::interpolateHermite(p0->altitude, p1->altitude, p2->altitude, p3->altitude, tn);

        // Pitch: [-90, 90]
        d->position.pitch = SkyMath::interpolateHermite180(p0->pitch, p1->pitch, p2->pitch, p3->pitch, tn);
        // Bank: [-180, 180]
        d->position.bank  = SkyMath::interpolateHermite180(p0->bank, p1->bank, p2->bank, p3->bank, tn);
        // Heading: [0, 360]
        d->position.heading = SkyMath::interpolateHermite360(p0->heading, p1->heading, p2->heading, p3->heading, tn);

        d->position.timestamp = timestamp;

    } else {
        // No recorded positions, or the timestamp exceeds the timestamp of the last recorded position
        d->position = Position::NullPosition;
    }

    return d->position;
}

// PRIVATE

// Updates the current index with the last index having a timestamp <= the given timestamp
bool Aircraft::updateCurrentIndex(qint64 timestamp) const
{
    int length = d->positions.length();
    if (length > 0 && timestamp <= d->positions.last().timestamp) {

        if (d->currentIndex != ::InvalidIndex) {
            if (d->positions.at(d->currentIndex).timestamp > timestamp) {
                // The timestamp was moved to front ("rewind"), so start searching from beginning
                // @todo binary search (O(log n)
                d->currentIndex = 0;
            }
        } else {
            // Start from beginning (O(n))
            // @todo binary search (O(log n)
            d->currentIndex = 0;
        }
    } else {
        // No positions yet, or timestamp not between given range
        d->currentIndex = ::InvalidIndex;
    }

    if (d->currentIndex != ::InvalidIndex) {
        // Increment the current index, until we find a position having a timestamp > the given timestamp
        bool found = false;
        while (!found && d->currentIndex < length) {
            if (d->currentIndex < (length - 1)) {
                if (d->positions.at(d->currentIndex + 1).timestamp > timestamp) {
                    // The next index has a larger timestamp, so this index is the one we are looking for
                    found = true;
                } else {
                    ++d->currentIndex;
                }
            } else {
                // Reached the last index
                found = true;
            }
        }
    }
    return d->currentIndex != ::InvalidIndex;
}

bool Aircraft::getSupportPositions(qint64 timestamp, const Position **p0, const Position **p1, const Position **p2, const Position **p3) const
{
    if (this->updateCurrentIndex(timestamp)) {

        *p1 = &d->positions.at(d->currentIndex);
        if (d->currentIndex > 0) {
           *p0 = &d->positions.at(d->currentIndex - 1);
        } else {
           *p0 = *p1;
        }
        if (d->currentIndex < d->positions.length() - 1) {
           if (d->currentIndex < d->positions.length() - 2) {
               *p2 = &d->positions.at(d->currentIndex + 1);
               *p3 = &d->positions.at(d->currentIndex + 2);
           } else {
               // p1 is the second to last position
               *p2 = &d->positions.at(d->currentIndex + 1);
               *p3 = *p2;
           }
        } else {
           // p1 is the last position
           *p2 = *p3 = *p1;
        }

    } else {
        *p0 = *p1 = *p2 = *p3 = nullptr;
    }

    return *p0 != nullptr;
}

double Aircraft::normaliseTimestamp(const Position &p1, const Position &p2, quint64 timestamp)
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

