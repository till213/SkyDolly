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
#include <utility>

#include "Waypoint.h"

// PUBLIC

Waypoint::Waypoint(float theLatitude, float theLongitude, float theAltitude) noexcept
    : TimeVariableData(),
      latitude(theLatitude),
      longitude(theLongitude),
      altitude(theAltitude)
{}

Waypoint::Waypoint(Waypoint &&other) noexcept
    : TimeVariableData(std::move(other)),
      identifier(std::move(other.identifier)),
      latitude(other.latitude),
      longitude(other.longitude),
      altitude(other.altitude),
      localTime(std::move(other.localTime)),
      zuluTime(std::move(other.zuluTime))
{}

Waypoint &Waypoint::operator=(Waypoint &&rhs) noexcept
{
    if (this != &rhs) {
        TimeVariableData::operator=(rhs);
        identifier = std::move(rhs.identifier);
        latitude = rhs.latitude;
        longitude = rhs.longitude;
        altitude = rhs.altitude;
        localTime = std::move(rhs.localTime);
        zuluTime = std::move(rhs.zuluTime);
    }
    return *this;
}

bool Waypoint::isValid() const noexcept
{
    return !identifier.isEmpty();
}

const Waypoint Waypoint::NullWaypoint = Waypoint();
const char Waypoint::CustomDepartureIdentifier[] = "CUSTD";
const char Waypoint::CustomArrivalIdentifier[] = "CUSTA";
