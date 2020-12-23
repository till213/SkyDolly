#ifndef SIMCONNECTPOSITION_H
#define SIMCONNECTPOSITION_H

#include <windows.h>

#include "Position.h"

struct SimConnectPosition : public Position
{
    static void addDataDefintion(HANDLE simConnectHandler);
};

#endif // SIMCONNECTPOSITION_H
