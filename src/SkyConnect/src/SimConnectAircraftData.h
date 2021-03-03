#ifndef SIMCONNECTAIRCRAFTDATA_H
#define SIMCONNECTAIRCRAFTDATA_H

#include <windows.h>

#include "../../Kernel/src/AircraftData.h"

struct SimConnectAircraftData
{
    // Aircraft position

    double latitude;  // degrees
    double longitude; // degrees
    double altitude;  // feet

    double pitch;     // degrees
    double bank;      // degrees
    double heading;   // degrees

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
