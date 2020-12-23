#include "Position.h"
#include "Aircraft.h"

class AircraftPrivate
{
public:
    AircraftPrivate()
    {}

    Position m_position;
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

void Aircraft::setPosition(Position position)
{
    d->m_position = position;
}

const Position &Aircraft::getPosition() const {
    return d->m_position;
}

