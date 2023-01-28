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
#ifndef FLIGHTCONDITION_H
#define FLIGHTCONDITION_H

#include <cstdint>

#include <QtGlobal>
#include <QDateTime>

#include "SimType.h"
#include "ModelLib.h"

struct MODEL_API FlightCondition final
{
    QDateTime startLocalTime;
    QDateTime startZuluTime;
    QDateTime endLocalTime;
    QDateTime endZuluTime;
    SimType::SurfaceType surfaceType {SimType::SurfaceType::Unknown};
    SimType::SurfaceCondition surfaceCondition {SimType::SurfaceCondition::Unknown};
    SimType::PrecipitationState precipitationState {SimType::PrecipitationState::None};
    float groundAltitude {0.0f};
    float ambientTemperature {0.0f};
    float totalAirTemperature {0.0f};
    float windSpeed {0.0f};
    float windDirection {0.0f};    
    float visibility {0.0f};
    float seaLevelPressure {0.0f};
    std::uint8_t pitotIcingPercent {0};
    std::uint8_t structuralIcingPercent {0};
    bool inClouds {false};
    bool onAnyRunway {false};
    bool onParkingSpot {false};

    void clear() noexcept;
};

#endif // FLIGHTCONDITION_H
