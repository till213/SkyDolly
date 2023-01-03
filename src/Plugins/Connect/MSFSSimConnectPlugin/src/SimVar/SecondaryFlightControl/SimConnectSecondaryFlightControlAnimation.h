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
#ifndef SIMCONNECTSECONDARYFLIGHTCONTROLANIMATION_H
#define SIMCONNECTSECONDARYFLIGHTCONTROLANIMATION_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Model/SimVar.h>
#include <Model/SecondaryFlightControlData.h>
#include "SimConnectType.h"

/*!
 * Simulation variables which represent the secondary flight controls: flaps and spoilers.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectSecondaryFlightControlAnimation
{
    // Implementation note: while the flaps simulation variables themselves are really named "* Percent"
    // we actually store the normalised position [0.0, 1.0]
    float leadingEdgeFlapsLeftPercent {0.0f};
    float leadingEdgeFlapsRightPercent {0.0f};
    float trailingEdgeFlapsLeftPercent {0.0f};
    float trailingEdgeFlapsRightPercent {0.0f};
    float spoilersLeftPosition {0.0};
    float spoilersRightPosition {0.0};

    SimConnectSecondaryFlightControlAnimation(const SecondaryFlightControlData &secondaryFlightControlData) noexcept
        : SimConnectSecondaryFlightControlAnimation()
    {
        fromSecondaryFlightControlData(secondaryFlightControlData);
    }
    SimConnectSecondaryFlightControlAnimation() = default;

    inline void fromSecondaryFlightControlData(const SecondaryFlightControlData &secondaryFlightControlData) noexcept
    {
        leadingEdgeFlapsLeftPercent = static_cast<float>(SkyMath::toNormalisedPosition(secondaryFlightControlData.leadingEdgeFlapsLeftPosition));
        leadingEdgeFlapsRightPercent = static_cast<float>(SkyMath::toNormalisedPosition(secondaryFlightControlData.leadingEdgeFlapsRightPosition));
        trailingEdgeFlapsLeftPercent = static_cast<float>(SkyMath::toNormalisedPosition(secondaryFlightControlData.trailingEdgeFlapsLeftPosition));
        trailingEdgeFlapsRightPercent = static_cast<float>(SkyMath::toNormalisedPosition(secondaryFlightControlData.trailingEdgeFlapsRightPosition));
        spoilersLeftPosition = static_cast<float>(SkyMath::toNormalisedPosition(secondaryFlightControlData.spoilersLeftPosition));
        spoilersRightPosition = static_cast<float>(SkyMath::toNormalisedPosition(secondaryFlightControlData.spoilersRightPosition));
    }

    inline SecondaryFlightControlData toSecondaryFlightControlData() const noexcept
    {
        SecondaryFlightControlData secondaryFlightControlData;
        toSecondaryFlightControlData(secondaryFlightControlData);
        return secondaryFlightControlData;
    }

    inline void toSecondaryFlightControlData(SecondaryFlightControlData &secondaryFlightControlData) const noexcept
    {
        secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(leadingEdgeFlapsLeftPercent);
        secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(leadingEdgeFlapsRightPercent);
        secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromNormalisedPosition(trailingEdgeFlapsLeftPercent);
        secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromNormalisedPosition(trailingEdgeFlapsRightPercent);
        secondaryFlightControlData.spoilersLeftPosition = SkyMath::fromNormalisedPosition(spoilersLeftPosition);
        secondaryFlightControlData.spoilersRightPosition = SkyMath::fromNormalisedPosition(spoilersRightPosition);
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LeadingEdgeFlapsLeftPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LeadingEdgeFlapsRightPercent, "Position",::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::TrailingEdgeFlapsLeftPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::TrailingEdgeFlapsRightPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, "Spoilers Left Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, "Spoilers Right Position", "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTSECONDARYFLIGHTCONTROLANIMATION_H
