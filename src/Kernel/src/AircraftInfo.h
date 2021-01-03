#ifndef AIRCRAFTINFO_H
#define AIRCRAFTINFO_H

#include <QByteArray>

#include "KernelLib.h"

struct KERNEL_API AircraftInfo
{
    QByteArray name;
    bool startOnGround;
    int initialAirspeed; // [knots]

    AircraftInfo();

    AircraftInfo (AircraftInfo &&) = default;
    AircraftInfo (const AircraftInfo &) = default;
    AircraftInfo &operator= (const AircraftInfo &) = default;
};

#endif // AIRCRAFTINFO_H
