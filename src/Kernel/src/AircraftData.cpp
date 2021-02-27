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
      spoilersHandlePosition(0.0),
      flapsHandleIndex(0),
      timestamp(0)
{
    this->latitude = latitude;
    this->longitude = longitude;
    this->altitude = altitude;
}

bool AircraftData::isValid() const {
    return !(latitude == 0.0 && longitude == 0.0 && altitude == 0.0 &&
             pitch == 0.0 && bank == 0.0 && heading == 0.0 && timestamp == 0);
}
