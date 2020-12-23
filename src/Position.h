#ifndef POSITION_H
#define POSITION_H

struct Position
{
    double latitude;
    double longitude;
    double altitude;

    Position(double latitude = 0.0, double longitude = 0.0, double altitude = 0.0);

    static const Position NullPosition;
};

#endif // POSITION_H
