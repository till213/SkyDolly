#include <windows.h>

#include <SimConnect.h>

#include "SimConnectDataDefinition.h"
#include "SimConnectPosition.h"

void SimConnectPosition::addToDataDefinition(HANDLE simConnectHandle)
{
    HRESULT res;

    res = ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Latitude", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    res = ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Longitude", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    res = ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Altitude", "feet", ::SIMCONNECT_DATATYPE_FLOAT64);
    res = ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Pitch Degrees", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    res = ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Bank Degrees", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
    res = ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, "Plane Heading Degrees True", "degrees", ::SIMCONNECT_DATATYPE_FLOAT64);
}
