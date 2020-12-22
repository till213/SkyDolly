#ifndef AIRCRAFT_H
#define AIRCRAFT_H

class AircraftPrivate;

class Aircraft
{
public:
    Aircraft();
    virtual ~Aircraft();

    void setLongitude(float longitude);
    float getLongitude() const;

    void setLatitude(float latitude);
    float getLatitude() const;

    void setAltitude(float altitude);
    float getAltitude() const;

private:
    AircraftPrivate *d;
};

#endif // AIRCRAFT_H
