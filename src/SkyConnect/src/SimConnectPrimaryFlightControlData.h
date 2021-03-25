/**
 * Sky Dolly - The black sheep for your flight recordings
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
#ifndef SIMCONNECTPRIMARYFLIGHTCONTROLDATA_H
#define SIMCONNECTPRIMARYFLIGHTCONTROLDATA_H

#include <windows.h>

#include "../../Kernel/src/SkyMath.h"
#include "../../Model/src/SimType.h"
#include "../../Model/src/PrimaryFlightControlData.h"

struct SimConnectPrimaryFlightControlData
{
    double yokeXPosition;
    double yokeYPosition;
    double rudderPosition;
    double elevatorPosition;
    double aileronPosition;

    inline PrimaryFlightControlData toPrimaryFlightControlData() const noexcept
    {
        PrimaryFlightControlData primaryFlightControlData;

        primaryFlightControlData.yokeXPosition = SkyMath::fromPosition(yokeXPosition);
        primaryFlightControlData.yokeYPosition = SkyMath::fromPosition(yokeYPosition);
        primaryFlightControlData.rudderPosition = SkyMath::fromPosition(rudderPosition);
        primaryFlightControlData.elevatorPosition = SkyMath::fromPosition(elevatorPosition);
        primaryFlightControlData.aileronPosition = SkyMath::fromPosition(aileronPosition);

        return primaryFlightControlData;
    }

    inline void fromPrimaryFlightControlData(const PrimaryFlightControlData &primaryFlightControlData) noexcept
    {
        yokeXPosition = SkyMath::toPosition(primaryFlightControlData.yokeXPosition);
        yokeYPosition = SkyMath::toPosition(primaryFlightControlData.yokeYPosition);
        rudderPosition = SkyMath::toPosition(primaryFlightControlData.rudderPosition);
        elevatorPosition = SkyMath::toPosition(primaryFlightControlData.elevatorPosition);
        aileronPosition = SkyMath::toPosition(primaryFlightControlData.aileronPosition);
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};

#endif // SIMCONNECTPRIMARYFLIGHTCONTROLDATA_H
