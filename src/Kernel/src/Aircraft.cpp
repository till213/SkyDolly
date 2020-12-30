#include <QObject>
#include <QByteArray>
#include <QVector>

#include "Position.h"
#include "Aircraft.h"

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

namespace {
    const int InvalidIndex = -1;
}

class AircraftPrivate
{
public:
    AircraftPrivate()
        : previousIndex(::InvalidIndex)
    {}

    QVector<Position> positions;
    Position position;
    QByteArray name;
    int previousIndex;
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

void Aircraft::setName(QByteArray name)
{
    d->name = name;
    emit infoChanged();
}

const QByteArray &Aircraft::getName() const
{
    return d->name;
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
    d->previousIndex = ::InvalidIndex;
    emit positionChanged();
}

const Position &Aircraft::getPosition(qint64 timestamp) const
{
    const Position *previousPosition;
    const Position *nextPosition;
    int index;

    if (d->positions.length() > 0 && timestamp <= d->positions.last().timestamp) {

        if (d->previousIndex != ::InvalidIndex) {

            index = d->previousIndex + 1;
            if (index < d->positions.length() && d->positions.at(index).timestamp >= timestamp) {
                 previousPosition = &(d->positions.at(d->previousIndex));
            } else {
                // The timestamp was moved to front ("rewind"), so start searching from beginning
                // @todo binary search (O(log n)
                previousPosition = &(d->positions.at(0));
                index = 1;
            }
        } else {
            // Start from beginning (O(n))
            // @todo binary search (O(log n)
            previousPosition = &(d->positions.at(0));
            index = 1;
        }

        nextPosition = nullptr;
        while (nextPosition == nullptr && index < d->positions.length()) {
            if (d->positions.at(index).timestamp >= timestamp) {
                nextPosition = &(d->positions.at(index));
                previousPosition = &(d->positions.at(index - 1));
            } else {
                ++index;
            }
        }
        if (nextPosition != nullptr) {
            // Linear interpolation - @todo Implement "boundary" cases, specifically the dateline "switch" from latitude -180 to 180
            double l = static_cast<double>(timestamp - previousPosition->timestamp) / static_cast<double>(nextPosition->timestamp - previousPosition->timestamp);
            d->position.latitude = previousPosition->latitude + l * (nextPosition->latitude - previousPosition->latitude);
            d->position.longitude = previousPosition->longitude + l * (nextPosition->longitude - previousPosition->longitude);
            d->position.altitude = previousPosition->altitude + l * (nextPosition->altitude - previousPosition->altitude);

            // Interpolate on the smaller arc
            // Pitch: [-180, 180[
            double diff = nextPosition->pitch - previousPosition->pitch;
            if (qAbs(diff) < 180.0f) {
                d->position.pitch = previousPosition->pitch + l * diff;
            } else {
                diff = -(360.0 - qAbs(diff)) * sgn(diff);
                d->position.pitch = previousPosition->pitch + l * diff;
                if (d->position.pitch < -180.0) {
                    d->position.pitch += 360.0;
                } else if (d->position.pitch > 180.0) {
                   d->position.pitch -= 360.0;
               }
            }

            // Bank: [-180, 180[
            diff = nextPosition->bank - previousPosition->bank;
            if (qAbs(diff) < 180.0f) {
                d->position.bank = previousPosition->bank + l * diff;
            } else {
                diff = -(360.0 - qAbs(diff)) * sgn(diff);
                d->position.bank = previousPosition->bank + l * diff;
                if (d->position.bank < -180.0) {
                    d->position.bank += 360.0;
                } else if (d->position.bank > 180.0) {
                    d->position.bank -= 360.0;
                }
            }

            // Heading: [0, 360[
            diff = nextPosition->heading - previousPosition->heading;
            if (qAbs(diff) < 180.0f) {
                d->position.heading = previousPosition->heading + l * diff;
            } else {
                diff = -(360.0 - qAbs(diff)) * sgn(diff);
                d->position.heading = previousPosition->heading + l * diff;
                if (d->position.heading < 0.0) {
                    d->position.heading += 360.0;
                } else if (d->position.heading > 360.0) {
                    d->position.heading -= 360.0;
                }
            }

            d->position.timestamp = timestamp;

        } else {
            // previousPosition is the only position
            d->position = *previousPosition;
        }


    } else {
        // No recorded positions, or the timestamp exceeds the timestamp of the last recorded position
        d->position = Position::NullPosition;
    }

    return d->position;
}

