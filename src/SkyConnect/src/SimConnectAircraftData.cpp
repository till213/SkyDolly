#include <windows.h>

#include <SimConnect.h>

#include "SimConnectDataDefinition.h"
#include "SimConnectAircraftData.h"

void SimConnectAircraftData::addToDataDefinition(HANDLE simConnectHandle)
{
    // Aircraft position
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Latitude", "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Longitude", "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Altitude", "feet", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Pitch Degrees", "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Bank Degrees", "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Heading Degrees True", "Degrees", ::SIMCONNECT_DATATYPE_FLOAT64);

    // Aircraft controls
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Yoke Y Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Yoke X Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Rudder Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Elevator Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Aileron Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);

    // Engine
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "General Eng Throttle Lever Position:1", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "General Eng Throttle Lever Position:2", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "General Eng Throttle Lever Position:3", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "General Eng Throttle Lever Position:4", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);

    // Flaps & speed brake
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Leading Edge Flaps Left Percent", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Leading Edge Flaps Right Percent", "Position",::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Trailing Edge Flaps Left Percent", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Trailing Edge Flaps Right Percent", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    // Spoilers, also known as "speed brake"
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Spoilers Handle Position", "Percent", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Flaps Handle Index", "Number", ::SIMCONNECT_DATATYPE_INT32);

    // Gear & Brakes
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Gear Handle Position", "Bool", ::SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Water Rudder Handle Position", "Percent Over 100", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Brake Left Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Brake Right Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Brake Parking Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT64);
}
