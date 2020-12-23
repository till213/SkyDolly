#include <QByteArray>

#include "Position.h"
#include "Aircraft.h"

class AircraftPrivate
{
public:
    AircraftPrivate()
    {}

    Position m_position;
    QByteArray m_name;
};

// PUBLIC

Aircraft::Aircraft()
    : d(new AircraftPrivate())
{

}

Aircraft::~Aircraft()
{
    delete d;
}

void Aircraft::setName(QByteArray name)
{
    d->m_name = name;
}

const QByteArray &Aircraft::getName() const
{
    return d->m_name;
}

void Aircraft::setPosition(Position position)
{
    d->m_position = position;
}

const Position &Aircraft::getPosition() const {
    return d->m_position;
}

