#ifndef SIMCONNECTAIRCRAFTDATA_H
#define SIMCONNECTAIRCRAFTDATA_H

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
#include <windows.h>

#include "../../Kernel/src/AircraftData.h"

struct SimConnectAircraftData
{
    // Aircraft position
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
    double propellerLeverPosition1;
    double propellerLeverPosition2;
    double propellerLeverPosition3;
    double propellerLeverPosition4;
    double mixtureLeverPosition1;
    double mixtureLeverPosition2;
    double mixtureLeverPosition3;
    double mixtureLeverPosition4;

    // Flaps & speed brakes
    double leadingEdgeFlapsLeftPercent;
    double leadingEdgeFlapsRightPercent;
    double trailingEdgeFlapsLeftPercent;
    double trailingEdgeFlapsRightPercent;
    double spoilersHandlePosition;
    qint32 flapsHandleIndex;

    // Gear & brakes
    qint32 gearHandlePosition;
    double waterRudderHandlePosition;
    double brakeLeftPosition;
    double brakeRightPosition;

    inline AircraftData toAircraftData() const {
        AircraftData aircraftData;

        aircraftData.latitude = latitude;
        aircraftData.longitude = longitude;
        aircraftData.altitude = altitude;
        aircraftData.pitch = pitch;
        aircraftData.bank = bank;
        aircraftData.heading = heading;

        aircraftData.yokeXPosition = yokeXPosition;
        aircraftData.yokeYPosition = yokeYPosition;
        aircraftData.rudderPosition = rudderPosition;
        aircraftData.elevatorPosition = elevatorPosition;
        aircraftData.aileronPosition = aileronPosition;

        aircraftData.throttleLeverPosition1 = throttleLeverPosition1;
        aircraftData.throttleLeverPosition2 = throttleLeverPosition2;
        aircraftData.throttleLeverPosition3 = throttleLeverPosition3;
        aircraftData.throttleLeverPosition4 = throttleLeverPosition4;
        aircraftData.propellerLeverPosition1 = propellerLeverPosition1;
        aircraftData.propellerLeverPosition2 = propellerLeverPosition2;
        aircraftData.propellerLeverPosition3 = propellerLeverPosition3;
        aircraftData.propellerLeverPosition4 = propellerLeverPosition4;
        aircraftData.mixtureLeverPosition1 = mixtureLeverPosition1;
        aircraftData.mixtureLeverPosition2 = mixtureLeverPosition2;
        aircraftData.mixtureLeverPosition3 = mixtureLeverPosition3;
        aircraftData.mixtureLeverPosition4 = mixtureLeverPosition4;

        aircraftData.leadingEdgeFlapsLeftPercent = leadingEdgeFlapsLeftPercent;
        aircraftData.leadingEdgeFlapsRightPercent = leadingEdgeFlapsRightPercent;
        aircraftData.trailingEdgeFlapsLeftPercent = trailingEdgeFlapsLeftPercent;
        aircraftData.trailingEdgeFlapsRightPercent = trailingEdgeFlapsRightPercent;
        aircraftData.spoilersHandlePosition = spoilersHandlePosition;
        aircraftData.flapsHandleIndex = flapsHandleIndex;

        aircraftData.gearHandlePosition = gearHandlePosition != 0;
        aircraftData.waterRudderHandlePosition = waterRudderHandlePosition;
        aircraftData.brakeLeftPosition= brakeLeftPosition;
        aircraftData.brakeRightPosition = brakeRightPosition;

        return aircraftData;
    }

    inline void fromAircraftData(const AircraftData &aircraftData) {
        latitude = aircraftData.latitude;
        longitude = aircraftData.longitude;
        altitude = aircraftData.altitude;
        pitch = aircraftData.pitch;
        bank = aircraftData.bank;
        heading = aircraftData.heading;

        yokeXPosition = aircraftData.yokeXPosition;
        yokeYPosition = aircraftData.yokeYPosition;
        rudderPosition = aircraftData.rudderPosition;
        elevatorPosition = aircraftData.elevatorPosition;
        aileronPosition = aircraftData.aileronPosition;

        throttleLeverPosition1 = aircraftData.throttleLeverPosition1;
        throttleLeverPosition2 = aircraftData.throttleLeverPosition2;
        throttleLeverPosition3 = aircraftData.throttleLeverPosition3;
        throttleLeverPosition4 = aircraftData.throttleLeverPosition4;
        propellerLeverPosition1 = aircraftData.propellerLeverPosition1;
        propellerLeverPosition2 = aircraftData.propellerLeverPosition2;
        propellerLeverPosition3 = aircraftData.propellerLeverPosition3;
        propellerLeverPosition4 = aircraftData.propellerLeverPosition4;
        mixtureLeverPosition1 = aircraftData.mixtureLeverPosition1;
        mixtureLeverPosition2 = aircraftData.mixtureLeverPosition2;
        mixtureLeverPosition3 = aircraftData.mixtureLeverPosition3;
        mixtureLeverPosition4 = aircraftData.mixtureLeverPosition4;

        leadingEdgeFlapsLeftPercent = aircraftData.leadingEdgeFlapsLeftPercent;
        leadingEdgeFlapsRightPercent = aircraftData.leadingEdgeFlapsRightPercent;
        trailingEdgeFlapsLeftPercent = aircraftData.trailingEdgeFlapsLeftPercent;
        trailingEdgeFlapsRightPercent = aircraftData.trailingEdgeFlapsRightPercent;
        spoilersHandlePosition = aircraftData.spoilersHandlePosition;
        flapsHandleIndex = aircraftData.flapsHandleIndex;

        gearHandlePosition = aircraftData.gearHandlePosition ? 1 : 0;
        waterRudderHandlePosition = aircraftData.waterRudderHandlePosition;
        brakeLeftPosition = aircraftData.brakeLeftPosition;
        brakeRightPosition = aircraftData.brakeRightPosition;
    }

    static void addToDataDefinition(HANDLE simConnectHandle);
};

#endif // SIMCONNECTAIRCRAFTDATA_H
