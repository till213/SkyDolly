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
#ifndef AIRCRAFTDATA_H
#define AIRCRAFTDATA_H

#include <QtGlobal>
#include <QFlags>

#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API AircraftData : public TimeVariableData
{
    // Position

    // Degrees
    double latitude;
    double longitude;
    double altitude;
    double pitch;
    double bank;
    double heading;

    // Velocity
    double velocityBodyX;
    double velocityBodyY;
    double velocityBodyZ;
    double rotationVelocityBodyX;
    double rotationVelocityBodyY;
    double rotationVelocityBodyZ;

    // Acceleration
    double accelerationBodyX;
    double accelerationBodyY;
    double accelerationBodyZ;
    double rotationAccelerationBodyX;
    double rotationAccelerationBodyY;
    double rotationAccelerationBodyZ;

    AircraftData(double latitude = 0.0, double longitude = 0.0, double altitude = 0.0) noexcept;

    AircraftData(AircraftData &&) = default;
    AircraftData(const AircraftData &) = default;
    AircraftData &operator= (const AircraftData &) = default;

    static const AircraftData NullAircraftData;
};

#endif // AIRCRAFTDATA_H
