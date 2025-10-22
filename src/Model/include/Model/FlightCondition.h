/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#include <utility>

#include <QtGlobal>
#include <QDateTime>
#include <QTimeZone>

#include "SimType.h"
#include "ModelLib.h"

struct MODEL_API FlightCondition final
{
public:
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

    FlightCondition() noexcept;

    void clear() noexcept;

    inline QDateTime getStartLocalDateTime() const noexcept
    {
        return m_startLocalDateTime;
    };

    inline void setStartLocalDateTime(QDateTime startTime) noexcept
    {
        m_startLocalDateTime = std::move(startTime);
    }

    inline QDateTime getEndLocalDateTime() const noexcept
    {
        return m_endLocalDateTime;
    };

    inline void setEndLocalDateTime(QDateTime endTime) noexcept
    {
        m_endLocalDateTime = std::move(endTime);
    }

    inline QDateTime getStartZuluDateTime() const noexcept
    {
        return m_startZuluDateTime;
    };

    inline void setStartZuluDateTime(QDateTime startTime) noexcept
    {
        m_startZuluDateTime = std::move(startTime);
        m_startZuluDateTime.setTimeZone(QTimeZone::UTC);
    }

    inline QDateTime getEndZuluDateTime() const noexcept
    {
        return m_endZuluDateTime;
    };

    inline void setEndZuluDateTime(QDateTime endTime) noexcept
    {
        m_endZuluDateTime = std::move(endTime);
        m_endZuluDateTime.setTimeZone(QTimeZone::UTC);
    }

private:
    // Simulation times (not real-world times)
    QDateTime m_startLocalDateTime;
    QDateTime m_endLocalDateTime;
    QDateTime m_startZuluDateTime;
    QDateTime m_endZuluDateTime;
};

#endif // FLIGHTCONDITION_H
