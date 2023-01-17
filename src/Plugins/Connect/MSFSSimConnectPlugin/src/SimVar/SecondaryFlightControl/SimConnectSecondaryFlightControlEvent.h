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
#ifndef SIMCONNECTSECONDARYFLIGHTCONTROLEVENT_H
#define SIMCONNECTSECONDARYFLIGHTCONTROLEVENT_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Model/SimVar.h>
#include <Model/SecondaryFlightControlData.h>
#include "SimConnectType.h"

/*!
 * Secondary flight control simulation variables that are sent as event to the user aircraft
 * (and possibly as simulation variables to AI aircraft).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectSecondaryFlightControlEvent
{
    float spoilersHandlePosition {0.0f};
    std::int32_t spoilersArmed {0};
    std::int32_t flapsHandleIndex {0};

    SimConnectSecondaryFlightControlEvent(const SecondaryFlightControlData &secondaryFlightControlData) noexcept
        : SimConnectSecondaryFlightControlEvent()
    {
        fromSecondaryFlightControlData(secondaryFlightControlData);
    }

    SimConnectSecondaryFlightControlEvent() = default;

    inline void fromSecondaryFlightControlData(const SecondaryFlightControlData &secondaryFlightControlData) noexcept
    {
        spoilersHandlePosition = static_cast<float>(SkyMath::toPercent(secondaryFlightControlData.spoilersHandlePercent));
        spoilersArmed = secondaryFlightControlData.spoilersArmed ? 1 : 0;
        flapsHandleIndex = secondaryFlightControlData.flapsHandleIndex;
    }

    inline SecondaryFlightControlData toSecondaryFlightControlData() const noexcept
    {
        SecondaryFlightControlData secondaryFlightControlData;
        toSecondaryFlightControlData(secondaryFlightControlData);
        return secondaryFlightControlData;
    }

    inline void toSecondaryFlightControlData(SecondaryFlightControlData &secondaryFlightControlData) const noexcept
    {
        secondaryFlightControlData.spoilersHandlePercent = SkyMath::fromPercent(spoilersHandlePosition);
        secondaryFlightControlData.spoilersArmed = spoilersArmed != 0;
        secondaryFlightControlData.flapsHandleIndex = static_cast<std::int8_t>(flapsHandleIndex);
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        // Spoilers, also known as "speed brakes"
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::SpoilersHandlePosition, "Percent", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::SpoilersArmed, "Boolean", ::SIMCONNECT_DATATYPE_INT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::FlapsHandleIndex, "Number", ::SIMCONNECT_DATATYPE_INT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTSECONDARYFLIGHTCONTROLEVENT_H
