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
#ifndef SIMCONNECTPRIMARYFLIGHTCONTROLAI_H
#define SIMCONNECTPRIMARYFLIGHTCONTROLAI_H

#include <Kernel/Enum.h>
#include <Model/PrimaryFlightControlData.h>
#include "SimConnectType.h"
#include "SimConnectPrimaryFlightControlAnimation.h"

/*!
 * Primary flight control simulation variables that are sent to AI aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPrimaryFlightControlAi
{
    SimConnectPrimaryFlightControlAnimation animation;

    SimConnectPrimaryFlightControlAi(const PrimaryFlightControlData &data) noexcept
        : SimConnectPrimaryFlightControlAi()
    {
        fromPrimaryFlightControlData(data);
    }

    SimConnectPrimaryFlightControlAi() = default;

    inline void fromPrimaryFlightControlData(const PrimaryFlightControlData &data)
    {
        animation.fromPrimaryFlightControlData(data);
    }

    inline PrimaryFlightControlData toPrimaryFlightControlData() const noexcept
    {
        PrimaryFlightControlData data = animation.toPrimaryFlightControlData();
        return data;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectPrimaryFlightControlAnimation::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PrimaryFlightControlAi));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTPRIMARYFLIGHTCONTROLAI_H
