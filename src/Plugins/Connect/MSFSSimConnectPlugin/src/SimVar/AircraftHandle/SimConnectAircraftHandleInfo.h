/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#ifndef SIMCONNECTAIRCRAFTHANDLEINFO_H
#define SIMCONNECTAIRCRAFTHANDLEINFO_H

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Model/SimVar.h>
#include "SimConnectType.h"
#include <Model/AircraftHandleData.h>

/*!
 * Aircraft handle simulation variables that are stored for information purposes only.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftHandleInfo
{
    std::int32_t tailhookHandle {0};
    std::int32_t foldingWingHandlePosition {0};

    SimConnectAircraftHandleInfo(const AircraftHandleData &aircraftHandleData) noexcept
        : SimConnectAircraftHandleInfo()
    {
        fromAircraftHandleData(aircraftHandleData);
    }
    SimConnectAircraftHandleInfo() = default;

    inline void fromAircraftHandleData(const AircraftHandleData &aircraftHandleData) noexcept
    {
        tailhookHandle = aircraftHandleData.tailhookHandlePosition ? 1 : 0;
        foldingWingHandlePosition = aircraftHandleData.foldingWingHandlePosition ? 1 : 0;
    }

    inline AircraftHandleData toAircraftHandleData() const noexcept
    {
        AircraftHandleData aircraftHandleData;
        toAircraftHandleData(aircraftHandleData);
        return aircraftHandleData;
    }

    inline void toAircraftHandleData(AircraftHandleData &aircraftHandleData) const noexcept
    {
        aircraftHandleData.tailhookHandlePosition = tailhookHandle != 0;
        aircraftHandleData.foldingWingHandlePosition = foldingWingHandlePosition != 0;
    }

    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::TailhookHandle, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::FoldingWingHandlePosition, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTHANDLEINFO_H
