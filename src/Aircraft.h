#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include "Position.h"

class AircraftPrivate;

class Aircraft
{
public:
    Aircraft();
    virtual ~Aircraft();

    void setPosition(Position position);
    const Position &getPosition() const;

private:
    AircraftPrivate *d;
};

#endif // AIRCRAFT_H
