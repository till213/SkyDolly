#include <windows.h>

#include <SimConnect.h>

#include "SimConnectDataDefinition.h"
#include "SimConnectAircraftInfo.h"

void SimConnectAircraftInfo::addToDataDefinition(HANDLE simConnectHandle)
{
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftInfoDefinition, "Title", nullptr, ::SIMCONNECT_DATATYPE_STRING256);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftInfoDefinition, "Sim On Ground", "bool", SIMCONNECT_DATATYPE_INT32);
    ::SimConnect_AddToDataDefinition(simConnectHandle, SkyConnectDataDefinition::AircraftInfoDefinition, "Airspeed True", "knots", SIMCONNECT_DATATYPE_INT32);
}

