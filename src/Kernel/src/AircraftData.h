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

    // Forces
    double velocityBodyX;
    double velocityBodyY;
    double velocityBodyZ;
    double rotationVelocityBodyX;
    double rotationVelocityBodyY;
    double rotationVelocityBodyZ;

    // Aircraft controls
    qint16 yokeXPosition;
    qint16 yokeYPosition;
    qint16 rudderPosition;
    qint16 elevatorPosition;
    qint16 aileronPosition;

    // General engine
    // Implementation note: the throttle can also yield negative thrust, hence the
    // type qint16 (position) which also supports negative values
    qint16 throttleLeverPosition1;
    qint16 throttleLeverPosition2;
    qint16 throttleLeverPosition3;
    qint16 throttleLeverPosition4;
    qint16 propellerLeverPosition1;
    qint16 propellerLeverPosition2;
    qint16 propellerLeverPosition3;
    qint16 propellerLeverPosition4;
    quint8 mixtureLeverPosition1;
    quint8 mixtureLeverPosition2;
    quint8 mixtureLeverPosition3;
    quint8 mixtureLeverPosition4;

    // Flaps & speed brakes
    quint8 leadingEdgeFlapsLeftPercent;
    quint8 leadingEdgeFlapsRightPercent;
    quint8 trailingEdgeFlapsLeftPercent;
    quint8 trailingEdgeFlapsRightPercent;
    quint8 spoilersHandlePosition;
    qint8 flapsHandleIndex;

    // Gears, brakes and handles
    bool gearHandlePosition;
    qint16 brakeLeftPosition;
    qint16 brakeRightPosition;
    // Implementation note: the water rudder can also have negative (-100.0) values,
    // hence hence the type qint16 (position) which also supports negative values
    qint16 waterRudderHandlePosition;
    quint8 tailhookPosition;
    quint8 canopyOpen;

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
