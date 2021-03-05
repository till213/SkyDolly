/**
 * Sky Dolly - The black sheep for your flight recordings
 *
 * Copyright (c) Oliver Knoll
 * All rights reserved.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
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
      propellerLeverPosition1(0.0),
      propellerLeverPosition2(0.0),
      propellerLeverPosition3(0.0),
      propellerLeverPosition4(0.0),
      mixtureLeverPosition1(0.0),
      mixtureLeverPosition2(0.0),
      mixtureLeverPosition3(0.0),
      mixtureLeverPosition4(0.0),
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

