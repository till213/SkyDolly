#include "Aircraft.h"

class AircraftPrivate
{
public:
    AircraftPrivate(float latitude, float longitude, float altitude)
        : m_latitude(latitude),
          m_longitude(longitude),
          m_altitude(altitude)
    {}

    float m_latitude;
    float m_longitude;
    float m_altitude;
};

// PUBLIC

Aircraft::Aircraft()
    : d(new AircraftPrivate(0.0f, 0.0f, 0.0f))
{

}

Aircraft::~Aircraft()
{
    delete d;
}

void Aircraft::setLatitude(float latitude)
{
    d->m_latitude = latitude;
}

float Aircraft::getLatitude() const
{
    return d->m_latitude;
}

void Aircraft::setLongitude(float longitude)
{
    d->m_longitude = longitude;
}

float Aircraft::getLongitude() const
{
    return d->m_longitude;
}

void Aircraft::setAltitude(float altitude)
{
    d->m_altitude = altitude;
}

float Aircraft::getAltitude() const
{
    return d->m_altitude;
}

