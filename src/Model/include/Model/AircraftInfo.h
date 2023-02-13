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
#ifndef AIRCRAFTINFO_H
#define AIRCRAFTINFO_H

#include <cstdint>

#include <QString>

#include <Kernel/Const.h>
#include "AircraftType.h"
#include "ModelLib.h"

struct MODEL_API AircraftInfo final
{
    std::int64_t aircraftId {0};
    AircraftType aircraftType;
    // Milliseconds
    std::int64_t timeOffset {0};
    QString tailNumber;
    QString airline;
    QString flightNumber;    

    // Feet
    float altitudeAboveGround {0.0f};
    bool startOnGround {false};
    // Knots (TAS)
    int initialAirspeed {0};

    AircraftInfo(std::int64_t aircraftId = Const::InvalidId) noexcept;
    AircraftInfo() noexcept;
    AircraftInfo(const AircraftInfo &rhs) = default;
    AircraftInfo(AircraftInfo &&rhs) = default;
    AircraftInfo &operator=(const AircraftInfo &rhs) = default;
    AircraftInfo &operator=(AircraftInfo &&rhs) = default;
    ~AircraftInfo() = default;

    void clear() noexcept;
};

inline bool operator==(const AircraftInfo &lhs, const AircraftInfo &rhs) {
    return lhs.aircraftId == rhs.aircraftId &&
           lhs.aircraftType == rhs.aircraftType &&
           lhs.timeOffset == rhs.timeOffset &&
           lhs.tailNumber == rhs.tailNumber &&
           lhs.airline == rhs.airline &&
           lhs.flightNumber == rhs.flightNumber &&
           lhs.altitudeAboveGround == rhs.altitudeAboveGround &&
           lhs.startOnGround == rhs.startOnGround &&
           lhs.initialAirspeed == rhs.initialAirspeed;
}

inline bool operator!=(const AircraftInfo &lhs, const AircraftInfo &rhs) {
    return !(lhs == rhs);
}

#endif // AIRCRAFTINFO_H
