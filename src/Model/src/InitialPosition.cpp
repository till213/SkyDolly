/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include "PositionData.h"
#include "AircraftInfo.h"
#include "InitialPosition.h"

// PUBLIC

InitialPosition::InitialPosition(double theLatitude, double theLongitude, double theAltitude) noexcept
    : latitude(theLatitude),
      longitude(theLongitude),
      altitude(theAltitude),
      pitch(0.0),
      bank(0.0),
      heading(0.0),
      indicatedAirspeed(InvalidIndicatedAirspeed),
      onGround(false)
{}

InitialPosition::InitialPosition(const PositionData &positionData, const AircraftInfo &aircraftInfo) noexcept
{
    fromPositionData(positionData);
    onGround = aircraftInfo.startOnGround;
}

const InitialPosition InitialPosition::NullData = InitialPosition(0.0, 0.0, 0.0);
