#include <windows.h>
#include <SimConnect.h>

#include "SimConnectDataDefinition.h"
#include "SimConnectPosition.h"

void SimConnectPosition::addDataDefintion(HANDLE simConnectHandler)
{
    HRESULT res = ::SimConnect_AddToDataDefinition(simConnectHandler, AircraftPositionDefinition, "Plane Latitude", "degrees");
    res = ::SimConnect_AddToDataDefinition(simConnectHandler, AircraftPositionDefinition, "Plane Longitude", "degrees");
    res = ::SimConnect_AddToDataDefinition(simConnectHandler, AircraftPositionDefinition, "Plane Altitude", "feet");
}
