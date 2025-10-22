/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#ifndef SIMCONNECTENGINECOMMON_H
#define SIMCONNECTENGINECOMMON_H

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Model/SimVar.h>
#include <Model/EngineData.h>

/*!
 * Common engine simulation variables that are sent both to the user- and AI aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineCommon
{
    // Position
    float throttleLeverPosition1 {0.0f};
    float throttleLeverPosition2 {0.0f};
    float throttleLeverPosition3 {0.0f};
    float throttleLeverPosition4 {0.0f};
    float propellerLeverPosition1 {0.0f};
    float propellerLeverPosition2 {0.0f};
    float propellerLeverPosition3 {0.0f};
    float propellerLeverPosition4 {0.0f};
    // Percent
    float recipEngineCowlFlapPosition1 {0.0f};
    float recipEngineCowlFlapPosition2 {0.0f};
    float recipEngineCowlFlapPosition3 {0.0f};
    float recipEngineCowlFlapPosition4 {0.0f};

    SimConnectEngineCommon(const EngineData &data) noexcept
        : SimConnectEngineCommon()
    {
        fromEngineData(data);
    }

    SimConnectEngineCommon() = default;

    inline void fromEngineData(const EngineData &data) noexcept
    {
        throttleLeverPosition1 = static_cast<float>(SkyMath::toNormalisedPosition(data.throttleLeverPosition1));
        throttleLeverPosition2 = static_cast<float>(SkyMath::toNormalisedPosition(data.throttleLeverPosition2));
        throttleLeverPosition3 = static_cast<float>(SkyMath::toNormalisedPosition(data.throttleLeverPosition3));
        throttleLeverPosition4 = static_cast<float>(SkyMath::toNormalisedPosition(data.throttleLeverPosition4));
        propellerLeverPosition1 = static_cast<float>(SkyMath::toNormalisedPosition(data.propellerLeverPosition1));
        propellerLeverPosition2 = static_cast<float>(SkyMath::toNormalisedPosition(data.propellerLeverPosition2));
        propellerLeverPosition3 = static_cast<float>(SkyMath::toNormalisedPosition(data.propellerLeverPosition3));
        propellerLeverPosition4 = static_cast<float>(SkyMath::toNormalisedPosition(data.propellerLeverPosition4));
        recipEngineCowlFlapPosition1 = static_cast<float>(SkyMath::toPercent(data.cowlFlapPosition1));
        recipEngineCowlFlapPosition2 = static_cast<float>(SkyMath::toPercent(data.cowlFlapPosition2));
        recipEngineCowlFlapPosition3 = static_cast<float>(SkyMath::toPercent(data.cowlFlapPosition3));
        recipEngineCowlFlapPosition4 = static_cast<float>(SkyMath::toPercent(data.cowlFlapPosition4));
    }

    inline EngineData toEngineData() const noexcept
    {
        EngineData data;

        // Note: the throttle can also yield negative thrust, hence the Sky Dolly internal type
        //       position (std::int16_t) which supports negative values as well
        data.throttleLeverPosition1 = SkyMath::fromNormalisedPosition(throttleLeverPosition1);
        data.throttleLeverPosition2 = SkyMath::fromNormalisedPosition(throttleLeverPosition2);
        data.throttleLeverPosition3 = SkyMath::fromNormalisedPosition(throttleLeverPosition3);
        data.throttleLeverPosition4 = SkyMath::fromNormalisedPosition(throttleLeverPosition4);
        data.propellerLeverPosition1 = SkyMath::fromNormalisedPosition(propellerLeverPosition1);
        data.propellerLeverPosition2 = SkyMath::fromNormalisedPosition(propellerLeverPosition2);
        data.propellerLeverPosition3 = SkyMath::fromNormalisedPosition(propellerLeverPosition3);
        data.propellerLeverPosition4 = SkyMath::fromNormalisedPosition(propellerLeverPosition4);
        data.cowlFlapPosition1 = SkyMath::fromPercent(recipEngineCowlFlapPosition1);
        data.cowlFlapPosition2 = SkyMath::fromPercent(recipEngineCowlFlapPosition2);
        data.cowlFlapPosition3 = SkyMath::fromPercent(recipEngineCowlFlapPosition3);
        data.cowlFlapPosition4 = SkyMath::fromPercent(recipEngineCowlFlapPosition4);

        return data;
    }

    static void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ThrottleLeverPosition1, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ThrottleLeverPosition2, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ThrottleLeverPosition3, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::ThrottleLeverPosition4, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PropellerLeverPosition1, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PropellerLeverPosition2, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PropellerLeverPosition3, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::PropellerLeverPosition4, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition1, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition2, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition3, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::RecipEngineCowlFlapPosition4, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTENGINECOMMON_H
