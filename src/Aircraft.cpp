#include <QObject>
#include <QByteArray>
#include <QVector>

#include "Position.h"
#include "Aircraft.h"

class AircraftPrivate
{
public:
    AircraftPrivate()
    {}

    QVector<Position> m_positions;
    QByteArray m_name;
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
    emit positionChanged();
}

