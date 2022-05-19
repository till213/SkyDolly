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
#include <utility>

#include "SimType.h"
#include "FlightCondition.h"

// PUBLIC

FlightCondition::FlightCondition() noexcept
    : groundAltitude{0.0f},
      surfaceType{SimType::SurfaceType::Unknown},
      ambientTemperature{0.0f},
      totalAirTemperature{0.0f},
      windVelocity{0.0f},
      windDirection{0.0f},
      precipitationState{SimType::PrecipitationState::None},
      visibility{0.0f},
      seaLevelPressure{0.0f},
      pitotIcingPercent{0},
      structuralIcingPercent{0},
      inClouds{false}
{}

FlightCondition::FlightCondition(FlightCondition &&other) noexcept
    : groundAltitude(other.groundAltitude),
      surfaceType(other.surfaceType),
      ambientTemperature(other.ambientTemperature),
      totalAirTemperature(other.totalAirTemperature),
      windVelocity(other.windVelocity),
      windDirection(other.windDirection),
      precipitationState(other.precipitationState),
      visibility(other.visibility),
      seaLevelPressure(other.seaLevelPressure),
      pitotIcingPercent(other.pitotIcingPercent),
      structuralIcingPercent(other.structuralIcingPercent),
      inClouds(other.inClouds),
      startLocalTime(std::move(other.startLocalTime)),
      startZuluTime(std::move(other.startZuluTime)),
      endLocalTime(std::move(other.endLocalTime)),
      endZuluTime(std::move(other.endZuluTime))
{}

FlightCondition &FlightCondition::operator=(FlightCondition &&rhs) noexcept
{
    if (this != &rhs) {
        groundAltitude = rhs.groundAltitude;
        surfaceType = rhs.surfaceType;
        ambientTemperature = rhs.ambientTemperature;
        totalAirTemperature = rhs.totalAirTemperature;
        windVelocity = rhs.windVelocity;
        windDirection = rhs.windDirection;
        precipitationState = rhs.precipitationState;
        visibility = rhs.visibility;
        seaLevelPressure = rhs.seaLevelPressure;
        pitotIcingPercent = rhs.pitotIcingPercent;
        structuralIcingPercent = rhs.structuralIcingPercent;
        inClouds = rhs.inClouds;
        startLocalTime = std::move(rhs.startLocalTime);
        startZuluTime = std::move(rhs.startZuluTime);
        endLocalTime = std::move(rhs.endLocalTime);
        endZuluTime = std::move(rhs.endZuluTime);
    }
    return *this;
}

void FlightCondition::clear() noexcept
{
    *this = FlightCondition();
}
