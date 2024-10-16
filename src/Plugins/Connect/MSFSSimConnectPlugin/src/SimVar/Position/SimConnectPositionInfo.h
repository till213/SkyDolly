/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SIMCONNECTPOSITIONINFO_H
#define SIMCONNECTPOSITIONINFO_H

#include <windows.h>
#include <SimConnect.h>

#include <Model/SimVar.h>
#include <Model/PositionData.h>

/*!
 * Aircraft position simulation variables that are either stored for information purposes only
 * or that are sent exclusively to the user aircraft as events.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPositionInfo
{
    double indicatedAltitude {0.0};
    double calibratedIndicatedAltitude {0.0};
    double pressureAltitude {0.0};

    SimConnectPositionInfo(const PositionData &data) noexcept
        : SimConnectPositionInfo()
    {
        fromPositionData(data);
    }

    SimConnectPositionInfo() = default;

    inline void fromPositionData(const PositionData &data) noexcept
    {
        indicatedAltitude = data.indicatedAltitude;
        calibratedIndicatedAltitude = data.calibratedIndicatedAltitude;
        pressureAltitude = data.pressureAltitude;
    }

    inline PositionData toPositionData() const noexcept
    {
        PositionData data;
        toPositionData(data);
        return data;
    }

    inline void toPositionData(PositionData &data) const noexcept
    {
        data.indicatedAltitude = indicatedAltitude;
        data.calibratedIndicatedAltitude = calibratedIndicatedAltitude;
        data.pressureAltitude = pressureAltitude;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::IndicatedAltitude, "Feet", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::CalibratedIndicatedAltitude, "Feet", ::SIMCONNECT_DATATYPE_FLOAT64);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PressureAltitude, "Feet", ::SIMCONNECT_DATATYPE_FLOAT64);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTPOSITIONINFO_H
