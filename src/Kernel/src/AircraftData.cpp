#include <QStringBuilder>

#include "AircraftData.h"

// PUBLIC

const AircraftData AircraftData::NullAircraftData = AircraftData(0.0, 0.0, 0.0);

AircraftData::AircraftData(double latitude, double longitude, double altitude)
    : pitch(0.0),
      bank(0.0),
      heading(0.0),
      yokeXPosition(0.0),
      yokeYPosition(0.0),
      rudderPosition(0.0),
      elevatorPosition(0.0),
      aileronPosition(0.0),
      throttleLeverPosition1(0.0),
      throttleLeverPosition2(0.0),
      throttleLeverPosition3(0.0),
      throttleLeverPosition4(0.0),
      leadingEdgeFlapsLeftPercent(0.0),
      leadingEdgeFlapsRightPercent(0.0),
      trailingEdgeFlapsLeftPercent(0.0),
      trailingEdgeFlapsRightPercent(0.0),
      spoilersHandlePosition(0.0),
      flapsHandleIndex(0),
      gearHandlePosition(false),
      waterRudderHandlePosition(0.0),
      brakeLeftPosition(0.0),
      brakeRightPosition(0.0),
      timestamp(0)
{
    this->latitude = latitude;
    this->longitude = longitude;
    this->altitude = altitude;
}

QString AircraftData::toString() const
{
    // Implementation note: the operator % is re-implemented in the
    // included QStringBuilder
    return QString("Latitude: ") % QString::number(latitude) %
           QString(" Longitude: ") % QString::number(longitude) %
           QString(" Altitude: ") % QString::number(altitude) %
           QString(" Pitch: ") % QString::number(pitch) %
           QString(" Bank: ") % QString::number(bank) %
           QString(" Heading: ") % QString::number(heading);
}
