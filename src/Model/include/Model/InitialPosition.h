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
#ifndef INITIALPOSITION_H
#define INITIALPOSITION_H

#include <limits>
#include <cstdint>

#include <QtGlobal>

#include <Kernel/Convert.h>
#include "PositionData.h"
#include "ModelLib.h"

struct AircraftInfo;

class MODEL_API InitialPosition
{
public:
    static constexpr int InvalidAirspeed = std::numeric_limits<int>::min();

    double latitude;
    double longitude;
    double altitude;
    double pitch;
    double bank;
    double heading;
    bool onGround;
    // Indicated airspeed [knots]
    int indicatedAirspeed;

    InitialPosition(double latitude = 0.0, double longitude = 0.0, double altitude = 0.0) noexcept;
    InitialPosition(const PositionData &positionData, const AircraftInfo &aircraftInfo) noexcept;

    InitialPosition(InitialPosition &&) = default;
    InitialPosition(const InitialPosition &) = default;
    InitialPosition &operator= (const InitialPosition &) = default;

    inline bool isNull() const noexcept {
        return (indicatedAirspeed == InvalidAirspeed);
    }

    inline void fromPositionData(const PositionData &positionData) {
        latitude = positionData.latitude;
        longitude = positionData.longitude;
        altitude = positionData.altitude;
        pitch = positionData.pitch;
        bank = positionData.bank;
        heading = positionData.heading;
        const double trueAirspeed = Convert::feetPerSecondToKnots(positionData.velocityBodyZ);
        indicatedAirspeed = qRound(Convert::trueToIndicatedAirspeed(trueAirspeed, positionData.altitude));
    }

    static const InitialPosition NullData;
};

#endif // INITIALPOSITION_H
