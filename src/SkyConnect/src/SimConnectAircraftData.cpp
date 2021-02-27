#include <windows.h>

#include <SimConnect.h>

#include "SimConnectDataDefinition.h"
#include "SimConnectAircraftData.h"

void SimConnectAircraftData::addToDataDefinition(HANDLE simConnectHandle)
{
    // Aircraft position

    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Latitude", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Longitude", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Altitude", "feet", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Pitch Degrees", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Bank Degrees", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Heading Degrees True", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);

    // Aircraft controls
    // Note: while the documentation declares those variables to be of type "Position" (TYPE_SINT16 with a range of -16K to 0)
    //       at least setting those values only seem to work when defined as "degrees" (FLOAT64)
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Yoke Y Position", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Yoke X Position", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Rudder Position", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Elevator Position", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Aileron Position", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    // @todo gear!
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "General Eng Throttle Lever Position:1", "percent", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "General Eng Throttle Lever Position:2", "percent", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "General Eng Throttle Lever Position:3", "percent", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "General Eng Throttle Lever Position:4", "percent", ::SIMCONNECT_DATATYPE_FLOAT64);
    // Spoilers, also known as "speed brake"
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Spoilers Handle Position", "percent", ::SIMCONNECT_DATATYPE_FLOAT64);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Flaps Handle Index", "number", ::SIMCONNECT_DATATYPE_INT64);
}
