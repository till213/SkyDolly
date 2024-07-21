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

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/SkyMath.h>
#include <Model/SimVar.h>
#include <Model/SecondaryFlightControlData.h>
#include "SimConnectType.h"

/*!
 * Secondary flightr control simulation variables that are sent exclusively to AI aircraft for animation.
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

    SimConnectSecondaryFlightControlAnimation(const SecondaryFlightControlData &data) noexcept
        : SimConnectSecondaryFlightControlAnimation()
    {
        fromSecondaryFlightControlData(data);
    }

    SimConnectSecondaryFlightControlAnimation() = default;

    inline void fromSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept
    {
        leadingEdgeFlapsLeftPercent = static_cast<float>(SkyMath::toNormalisedPosition(data.leftLeadingEdgeFlapsPosition));
        leadingEdgeFlapsRightPercent = static_cast<float>(SkyMath::toNormalisedPosition(data.rightLeadingEdgeFlapsPosition));
        trailingEdgeFlapsLeftPercent = static_cast<float>(SkyMath::toNormalisedPosition(data.leftTrailingEdgeFlapsPosition));
        trailingEdgeFlapsRightPercent = static_cast<float>(SkyMath::toNormalisedPosition(data.rightTrailingEdgeFlapsPosition));
        spoilersLeftPosition = static_cast<float>(SkyMath::toNormalisedPosition(data.leftSpoilersPosition));
        spoilersRightPosition = static_cast<float>(SkyMath::toNormalisedPosition(data.rightSpoilersPosition));
    }

    inline SecondaryFlightControlData toSecondaryFlightControlData() const noexcept
    {
        SecondaryFlightControlData data;
        toSecondaryFlightControlData(data);
        return data;
    }

    inline void toSecondaryFlightControlData(SecondaryFlightControlData &data) const noexcept
    {
        data.leftLeadingEdgeFlapsPosition = SkyMath::fromNormalisedPosition(leadingEdgeFlapsLeftPercent);
        data.rightLeadingEdgeFlapsPosition = SkyMath::fromNormalisedPosition(leadingEdgeFlapsRightPercent);
        data.leftTrailingEdgeFlapsPosition = SkyMath::fromNormalisedPosition(trailingEdgeFlapsLeftPercent);
        data.rightTrailingEdgeFlapsPosition = SkyMath::fromNormalisedPosition(trailingEdgeFlapsRightPercent);
        data.leftSpoilersPosition = SkyMath::fromNormalisedPosition(spoilersLeftPosition);
        data.rightSpoilersPosition = SkyMath::fromNormalisedPosition(spoilersRightPosition);
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle, ::SIMCONNECT_DATA_DEFINITION_ID dataDefinitionId) noexcept
    {
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LeadingEdgeFlapsLeftPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::LeadingEdgeFlapsRightPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::TrailingEdgeFlapsLeftPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::TrailingEdgeFlapsRightPercent, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::SpoilersLeftPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
        ::SimConnect_AddToDataDefinition(simConnectHandle, dataDefinitionId, SimVar::SpoilersRightPosition, "Position", ::SIMCONNECT_DATATYPE_FLOAT32);
    }
};
#pragma pack(pop)

#endif // SIMCONNECTSECONDARYFLIGHTCONTROLANIMATION_H
