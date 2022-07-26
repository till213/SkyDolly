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
#ifndef SIMCONNECTSECONDARYFLIGHTCONTROL_H
#define SIMCONNECTSECONDARYFLIGHTCONTROL_H

#include <windows.h>

#include <Kernel/SkyMath.h>
#include <Model/SimType.h>
#include <Model/SecondaryFlightControlData.h>

/*!
 * Simulation variables which represent the secondary flight controls: flaps and spoilers.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectSecondaryFlightControl
{
    float leadingEdgeFlapsLeftPercent {0.0f};
    float leadingEdgeFlapsRightPercent {0.0f};
    float trailingEdgeFlapsLeftPercent {0.0f};
    float trailingEdgeFlapsRightPercent {0.0f};
    float spoilersHandlePosition{0.0f} ;
    qint32 flapsHandleIndex {0};

    inline SecondaryFlightControlData toSecondaryFlightControlData() const noexcept
    {
        SecondaryFlightControlData secondaryFlightControlData;

        secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromPosition(leadingEdgeFlapsLeftPercent);
        secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromPosition(leadingEdgeFlapsRightPercent);
        secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromPosition(trailingEdgeFlapsLeftPercent);
        secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromPosition(trailingEdgeFlapsRightPercent);
        secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(spoilersHandlePosition);
        secondaryFlightControlData.flapsHandleIndex = flapsHandleIndex;

        return secondaryFlightControlData;
    }

    inline void fromSecondaryFlightControlData(const SecondaryFlightControlData &secondaryFlightControlData) noexcept
    {
        leadingEdgeFlapsLeftPercent = SkyMath::toPosition(secondaryFlightControlData.leadingEdgeFlapsLeftPosition);
        leadingEdgeFlapsRightPercent = SkyMath::toPosition(secondaryFlightControlData.leadingEdgeFlapsRightPosition);
        trailingEdgeFlapsLeftPercent = SkyMath::toPosition(secondaryFlightControlData.trailingEdgeFlapsLeftPosition);
        trailingEdgeFlapsRightPercent = SkyMath::toPosition(secondaryFlightControlData.trailingEdgeFlapsRightPosition);
        spoilersHandlePosition = SkyMath::toPercent(secondaryFlightControlData.spoilersHandlePosition);
        flapsHandleIndex = secondaryFlightControlData.flapsHandleIndex;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTSECONDARYFLIGHTCONTROL_H
