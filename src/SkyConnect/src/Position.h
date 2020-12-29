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

    qint64 timestamp; // in milliseconds since the start of recording

    Position(double latitude = 0.0, double longitude = 0.0, double altitude = 0.0);

    Position (Position &&) = default;
    Position (const Position &) = default;
    Position &operator= (const Position &) = default;

    static const Position NullPosition;
};

#endif // POSITION_H
