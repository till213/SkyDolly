#include <windows.h>

#include <SimConnect.h>

#include "SimConnectDataDefinition.h"
#include "SimConnectAircraftInfo.h"

void SimConnectAircraftInfo::addToDataDefinition(HANDLE simConnectHandle)
{
    HRESULT res;

    res = ::SimConnect_AddToDataDefinition(simConnectHandle, AircraftInfoDefinition, "title", nullptr, SIMCONNECT_DATATYPE_STRING256);
}

