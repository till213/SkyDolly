#ifndef SIMCONNECTPOSITION_H
#define SIMCONNECTPOSITION_H

#include "../../Kernel/src/Position.h"

struct SimConnectPosition
{
    double latitude;  // degrees
    double longitude; // degrees
    double altitude;  // feet

    double pitch;     // degrees
    double bank;      // degrees
    double heading;   // degrees

    inline Position toPosition() const {
        Position position;

        position.latitude = latitude;
        position.longitude = longitude;
        position.altitude = altitude;
        position.pitch = pitch;
        position.bank = bank;
        position.heading = heading;

        return position;
    }

    inline void fromPosition(const Position &position) {
        latitude = position.latitude;
        longitude = position.longitude;
        altitude = position.altitude;
        pitch = position.pitch;
        bank = position.bank;
        heading = position.heading;
    }
};

#endif // SIMCONNECTPOSITION_H
