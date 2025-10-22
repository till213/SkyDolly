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
#ifndef SIMCONNECTAIRCRAFTHANDLEALL_H
#define SIMCONNECTAIRCRAFTHANDLEALL_H

#include <Kernel/Enum.h>
#include <Model/AircraftHandleData.h>
#include "SimConnectType.h"
#include "SimConnectAircraftHandleCommon.h"
#include "SimConnectAircraftHandleCore.h"
#include "SimConnectAircraftHandleCoreEvent.h"
#include "SimConnectAircraftHandleEvent.h"
#include "SimConnectAircraftHandleAnimation.h"
#include "SimConnectAircraftHandleInfo.h"
#include "SimConnectAircraftHandleUser.h"
#include "SimConnectAircraftHandleAi.h"

/*!
 * All aircraft handle simulation variables (reply from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAircraftHandleAll
{
    SimConnectAircraftHandleCommon common;
    SimConnectAircraftHandleCore core;    
    SimConnectAircraftHandleCoreEvent coreEvent;
    SimConnectAircraftHandleEvent event;
    SimConnectAircraftHandleAnimation animation;
    SimConnectAircraftHandleInfo info;

    SimConnectAircraftHandleAll(const AircraftHandleData &aircraftHandle) noexcept
        : SimConnectAircraftHandleAll()
    {
        fromAircraftHandleData(aircraftHandle);
    }

    SimConnectAircraftHandleAll() = default;

    inline void fromAircraftHandleData(const AircraftHandleData &aircraftHandle) noexcept
    {
        common.fromAircraftHandleData(aircraftHandle);
        core.fromAircraftHandleData(aircraftHandle);
        coreEvent.fromAircraftHandleData(aircraftHandle);
        event.fromAircraftHandleData(aircraftHandle);
        animation.fromAircraftHandleData(aircraftHandle);
        info.fromAircraftHandleData(aircraftHandle);
    }

    inline AircraftHandleData toAircraftHandleData() const noexcept
    {
        AircraftHandleData aircraftHandle = common.toAircraftHandleData();
        core.toAircraftHandleData(aircraftHandle);
        coreEvent.toAircraftHandleData(aircraftHandle);
        event.toAircraftHandleData(aircraftHandle);
        animation.toAircraftHandleData(aircraftHandle);
        info.toAircraftHandleData(aircraftHandle);
        return aircraftHandle;
    }

    inline SimConnectAircraftHandleUser user() const noexcept
    {
        SimConnectAircraftHandleUser user;
        user.common = common;
        user.core = core;
        return user;
    }

    inline SimConnectAircraftHandleAi ai() const noexcept
    {
        SimConnectAircraftHandleAi ai;
        ai.common = common;
        ai.event = event;
        ai.animation = animation;
        return ai;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectAircraftHandleCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftHandleAll));
        SimConnectAircraftHandleCore::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftHandleAll));
        SimConnectAircraftHandleCoreEvent::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftHandleAll));
        SimConnectAircraftHandleEvent::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftHandleAll));
        SimConnectAircraftHandleAnimation::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftHandleAll));
        SimConnectAircraftHandleInfo::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AircraftHandleAll));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTAIRCRAFTHANDLEALL_H
