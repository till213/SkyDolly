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

#include "KernelLib.h"

struct KERNEL_API AircraftData
{
    // Position

    // Degrees
    double latitude;
    double longitude;
    double altitude;

    double pitch;
    double bank;
    double heading;

    // Aircraft controls
    double yokeXPosition;
    double yokeYPosition;
    double rudderPosition;
    double elevatorPosition;
    double aileronPosition;

    // General engine
    double throttleLeverPosition1;
    double throttleLeverPosition2;
    double throttleLeverPosition3;
    double throttleLeverPosition4;

    // Flaps & speed brakes
    double leadingEdgeFlapsLeftPercent;
    double leadingEdgeFlapsRightPercent;
    double trailingEdgeFlapsLeftPercent;
    double trailingEdgeFlapsRightPercent;
    double spoilersHandlePosition;
    int flapsHandleIndex;

    // Gears & brakes
    bool gearHandlePosition;
    double waterRudderHandlePosition;
    double brakeLeftPosition;
    double brakeRightPosition;

    // In milliseconds since the start of recording
    qint64 timestamp;

    AircraftData(double latitude = 0.0, double longitude = 0.0, double altitude = 0.0);
    inline bool isNull() const {
        return (latitude == 0.0 && longitude == 0.0 && altitude == 0.0 &&
                pitch == 0.0 && bank == 0.0 && heading == 0.0 && timestamp == 0);
    }

    AircraftData (AircraftData &&) = default;
    AircraftData (const AircraftData &) = default;
    AircraftData &operator= (const AircraftData &) = default;

    static const AircraftData NullAircraftData;
};

#endif // AIRCRAFTDATA_H
