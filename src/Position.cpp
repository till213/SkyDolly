#include "Position.h"

// PUBLIC

const Position Position::NullPosition = Position(0.0, 0.0, 0.0);

Position::Position(double latitude, double longitude, double altitude)
    : pitch(0.0),
      bank(0.0),
      heading(0.0)
{
    this->latitude = latitude;
    this->longitude = longitude;
    this->altitude = altitude;
}


