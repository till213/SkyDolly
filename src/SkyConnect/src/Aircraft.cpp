#include <QObject>
#include <QByteArray>
#include <QVector>

#include "Position.h"
#include "Aircraft.h"

namespace {
    const int InvalidIndex = -1;
}

class AircraftPrivate
{
public:
    AircraftPrivate()
        : m_previousIndex(::InvalidIndex)
    {}

    QVector<Position> m_positions;
    QByteArray m_name;
    int m_previousIndex;
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
    d->m_name = name;
    emit infoChanged();
}

const QByteArray &Aircraft::getName() const
{
    return d->m_name;
}

void Aircraft::appendPosition(Position position)
{
    d->m_positions.append(position);
    emit positionChanged();
}

const Position &Aircraft::getLastPosition() const {
    if (!d->m_positions.isEmpty()) {
        return d->m_positions.last();
    } else {
        return Position::NullPosition;
    }
}

const QVector<Position> Aircraft::getPositions() const
{
    return d->m_positions;
}

void Aircraft::clear()
{
    d->m_positions.clear();
    d->m_previousIndex = ::InvalidIndex;
    emit positionChanged();
}

const Position *Aircraft::getPosition(qint64 timestamp) const {
    const Position *position;
    const Position *previousPosition;
    const Position *nextPosition;
    int index;

    if (d->m_positions.length() > 0 && timestamp <= d->m_positions.last().timestamp) {

        if (d->m_previousIndex != ::InvalidIndex) {

            index = d->m_previousIndex + 1;
            if (index < d->m_positions.length() && d->m_positions.at(index).timestamp >= timestamp) {
                 previousPosition = &(d->m_positions.at(d->m_previousIndex));
            } else {
                // The timestamp was moved to front ("rewind"), so start searching from beginning
                // @todo binary search (O(log n)
                previousPosition = &(d->m_positions.at(0));
                index = 1;
            }
        } else {
            // Start from beginning (O(n))
            // @todo binary search (O(log n)
            previousPosition = &(d->m_positions.at(0));
            index = 1;
        }

        nextPosition = nullptr;
        while (nextPosition == nullptr && index < d->m_positions.length()) {
            if (d->m_positions.at(index).timestamp >= timestamp) {
                nextPosition = &(d->m_positions.at(index));
                previousPosition = &(d->m_positions.at(index - 1));
            } else {
                ++index;
            }
        }
        if (nextPosition != nullptr) {
            // Nearest neighbour interpolation (simple)
            qint64 t1 = timestamp - previousPosition->timestamp;
            qint64 t2 = nextPosition->timestamp - timestamp;
            if (t1 < t2) {
                position = previousPosition;
            } else {
                position = nextPosition;
            }
        } else {
            // previousPosition is the only position
            position = previousPosition;
        }


    } else {
        // No recorded positions, or the timestamp exceeds the timestamp of the last recorded position
        position = nullptr;
    }

    return position;
}

