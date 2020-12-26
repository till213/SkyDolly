#ifndef POSITION_H
#define POSITION_H

#include <QtGlobal>

struct Position
{
    double latitude;  // degrees
    double longitude; // degrees
    double altitude;  // feet

    double pitch;     // degrees
    double bank;      // degrees
    double heading;   // degrees

    Position(double latitude = 0.0, double longitude = 0.0, double altitude = 0.0);

    static const Position NullPosition;
};

#endif // POSITION_H
