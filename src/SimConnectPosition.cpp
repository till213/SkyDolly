#include <windows.h>
#include <SimConnect.h>

#include "SimConnectDataDefinition.h"
#include "SimConnectPosition.h"

HRESULT SimConnectPosition::addDataDefintion(HANDLE simConnectHandler)
{
    HRESULT res = ::SimConnect_AddToDataDefinition(simConnectHandler, AircraftPositionDefinition, "Plane Latitude", "degrees");
    res = ::SimConnect_AddToDataDefinition(simConnectHandler, AircraftPositionDefinition, "Plane Longitude", "degrees");
    res = ::SimConnect_AddToDataDefinition(simConnectHandler, AircraftPositionDefinition, "Plane Altitude", "feet");

    res = SimConnect_AddToDataDefinition(simConnectHandler, AircraftPositionDefinition, "Plane Pitch Degrees", "degrees");
    res = SimConnect_AddToDataDefinition(simConnectHandler, AircraftPositionDefinition, "Plane Bank Degrees", "degrees");
    res = SimConnect_AddToDataDefinition(simConnectHandler, AircraftPositionDefinition, "Plane Heading Degrees True", "degrees");

    return res;
}
