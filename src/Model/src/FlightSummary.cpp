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
#include "Flight.h"
#include "FlightSummary.h"

// PUBLIC

FlightSummary::FlightSummary() noexcept
    : flightId(Flight::InvalidId),
      aircraftCount(0)
{}

FlightSummary::~FlightSummary() noexcept
{}

FlightSummary::FlightSummary(FlightSummary &&other) noexcept
    : flightId(other.flightId),
      creationDate(std::move(other.creationDate)),
      aircraftType(std::move(other.aircraftType)),
      aircraftCount(other.aircraftCount),
      startSimulationLocalTime(std::move(other.startSimulationLocalTime)),
      startSimulationZuluTime(std::move(other.startSimulationZuluTime)),
      endSimulationLocalTime(std::move(other.endSimulationLocalTime)),
      endSimulationZuluTime(std::move(other.endSimulationZuluTime)),
      startLocation(std::move(other.startLocation)),
      endLocation(std::move(other.endLocation)),
      title(std::move(other.title))
{}

FlightSummary &FlightSummary::operator = (FlightSummary &&rhs) noexcept
{
    if (this != &rhs) {
        flightId = rhs.flightId;
        creationDate = std::move(rhs.creationDate);
        aircraftType = std::move(rhs.aircraftType);
        aircraftCount = rhs.aircraftCount;
        startSimulationLocalTime = std::move(rhs.startSimulationLocalTime);
        startSimulationZuluTime = std::move(rhs.startSimulationZuluTime);
        endSimulationLocalTime = std::move(rhs.endSimulationLocalTime);
        endSimulationZuluTime = std::move(rhs.endSimulationZuluTime);
        startLocation = std::move(rhs.startLocation);
        endLocation = std::move(rhs.endLocation);
        title = std::move(rhs.title);
    }
    return *this;
}
