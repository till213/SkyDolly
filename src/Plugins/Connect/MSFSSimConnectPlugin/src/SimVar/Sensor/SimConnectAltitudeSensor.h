/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SIMCONNECTALTITUDESENSOR
#define SIMCONNECTALTITUDESENSOR

#include <windows.h>
#include <SimConnect.h>

#include <Model/SimVar.h>
#include <Model/AltitudeSensorData.h>

/*!
 * The altitude sensor continuously measures altitudes above ground, also while replaying.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAltitudeSensor
{
    double planeAltitudeAboveGroundMinusCenterGravity {0};

    SimConnectAltitudeSensor(const AltitudeSensorData &attitudeData) noexcept
    {
        fromAltitudeSensorData(attitudeData);
    }

    SimConnectAltitudeSensor() = default;

    inline void fromAltitudeSensorData(const AltitudeSensorData &data) noexcept
    {
        planeAltitudeAboveGroundMinusCenterGravity = data.altitudeAboveGroundMinusCenterGravity;
    }

    inline AltitudeSensorData toAltitudeSensorData() const noexcept
    {
        AltitudeSensorData data;
        toAltitudeSensorData(data);
        return data;
    }

    inline void toAltitudeSensorData(AltitudeSensorData &data) const noexcept
    {
        data.altitudeAboveGroundMinusCenterGravity = planeAltitudeAboveGroundMinusCenterGravity;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PlaneAltAboveGroundMinusCG, "Feet", ::SIMCONNECT_DATATYPE_FLOAT64);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTALTITUDESENSOR
