
#include "Position.h"

Position::Position(double latitude, double longitude, double altitude)
{
    this->latitude = latitude;
    this->longitude = longitude;
    this->altitude = altitude;
}


// PRIVATE

const Position Position::NullPosition = Position(0.0, 0.0, 0.0);
