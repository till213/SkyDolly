/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#include <cstdint>
#include <utility>

#include <QString>

#include "AircraftType.h"
#include "AircraftInfo.h"

// PUBLIC

AircraftInfo::AircraftInfo(std::int64_t theAircraftId) noexcept
    : aircraftId(theAircraftId),
      timeOffset(0),
      altitudeAboveGround(0.0f),
      startOnGround(false),
      initialAirspeed(0)
{}

AircraftInfo::AircraftInfo(AircraftInfo &&rhs) noexcept
    : aircraftId(rhs.aircraftId),
      aircraftType(std::move(rhs.aircraftType)),
      timeOffset(rhs.timeOffset),
      tailNumber(std::move(rhs.tailNumber)),
      airline(std::move(rhs.airline)),
      flightNumber(std::move(rhs.flightNumber)),
      altitudeAboveGround(rhs.altitudeAboveGround),
      startOnGround(rhs.startOnGround),
      initialAirspeed(rhs.initialAirspeed)
{}

AircraftInfo &AircraftInfo::operator=(AircraftInfo &&rhs) noexcept
{
    if (this != &rhs) {
        aircraftId = rhs.aircraftId;
        aircraftType = std::move(rhs.aircraftType),
        timeOffset = rhs.timeOffset,
        tailNumber = std::move(rhs.tailNumber),
        airline = std::move(rhs.airline),
        flightNumber = std::move(rhs.flightNumber),
        altitudeAboveGround = rhs.altitudeAboveGround,
        startOnGround = rhs.startOnGround,
        initialAirspeed = rhs.initialAirspeed;
    }
    return *this;
}

void AircraftInfo::clear() noexcept
{
    *this = AircraftInfo(aircraftId);
}
