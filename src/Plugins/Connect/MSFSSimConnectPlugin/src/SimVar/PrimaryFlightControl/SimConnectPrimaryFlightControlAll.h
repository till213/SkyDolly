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
#ifndef SIMCONNECTPRIMARYFLIGHTCONTROLREPLY_H
#define SIMCONNECTPRIMARYFLIGHTCONTROLREPLY_H

#include <windows.h>

#include <Kernel/Enum.h>
#include <Model/PrimaryFlightControlData.h>
#include "SimConnectType.h"
#include "SimConnectPrimaryFlightControlEvent.h"
#include "SimConnectPrimaryFlightControlAnimation.h"
#include "SimConnectPrimaryFlightControlAi.h"

/*!
 * All primary flight control simulation variables (reply from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPrimaryFlightControlAll
{
    SimConnectPrimaryFlightControlEvent event;
    SimConnectPrimaryFlightControlAnimation animation;

    SimConnectPrimaryFlightControlAll(const PrimaryFlightControlData &data) noexcept
        : SimConnectPrimaryFlightControlAll()
    {
        fromPrimaryFlightControlData(data);
    }

    SimConnectPrimaryFlightControlAll() = default;

    inline void fromPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept
    {
        event.fromPrimaryFlightControlData(data);
        animation.fromPrimaryFlightControlData(data);
    }

    inline PrimaryFlightControlData toPrimaryFlightControlData() const noexcept
    {
        PrimaryFlightControlData data = event.toPrimaryFlightControlData();
        animation.toPrimaryFlightControlData(data);
        return data;
    }

    inline SimConnectPrimaryFlightControlAi ai() const noexcept
    {
        SimConnectPrimaryFlightControlAi ai;
        ai.animation = animation;
        return ai;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectPrimaryFlightControlEvent::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PrimaryFlightControlAll));
        SimConnectPrimaryFlightControlAnimation::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PrimaryFlightControlAll));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTPRIMARYFLIGHTCONTROLREPLY_H
