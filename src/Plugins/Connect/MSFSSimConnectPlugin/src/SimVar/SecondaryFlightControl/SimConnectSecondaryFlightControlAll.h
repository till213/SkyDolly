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
#ifndef SIMCONNECTSECONDARYFLIGHTCONTROLALL_H
#define SIMCONNECTSECONDARYFLIGHTCONTROLALL_H

#include <windows.h>

#include <Kernel/Enum.h>
#include <Model/SecondaryFlightControlData.h>
#include "SimConnectType.h"
#include "SimConnectSecondaryFlightControlEvent.h"
#include "SimConnectSecondaryFlightControlAnimation.h"
#include "SimConnectSecondaryFlightControlAi.h"

/*!
 * All secondary flight control simulation variables (reply from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectSecondaryFlightControlAll
{
    SimConnectSecondaryFlightControlEvent event;
    SimConnectSecondaryFlightControlAnimation animation;

    SimConnectSecondaryFlightControlAll(const SecondaryFlightControlData &data) noexcept
        : SimConnectSecondaryFlightControlAll()
    {
        fromSecondaryFlightControlData(data);
    }

    SimConnectSecondaryFlightControlAll() = default;

    inline void fromSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept
    {
        event.fromSecondaryFlightControlData(data);
        animation.fromSecondaryFlightControlData(data);
    }

    inline SecondaryFlightControlData toSecondaryFlightControlData() const noexcept
    {
        SecondaryFlightControlData data = event.toSecondaryFlightControlData();
        animation.toSecondaryFlightControlData(data);
        return data;
    }

    inline SimConnectSecondaryFlightControlAi ai() const noexcept
    {
        SimConnectSecondaryFlightControlAi ai;
        ai.animation = animation;
        return ai;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectSecondaryFlightControlEvent::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::SecondaryFlightControlAll));
        SimConnectSecondaryFlightControlAnimation::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::SecondaryFlightControlAll));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTSECONDARYFLIGHTCONTROLALL_H
