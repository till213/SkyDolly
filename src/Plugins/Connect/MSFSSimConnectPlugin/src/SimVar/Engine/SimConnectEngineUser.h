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
#ifndef SIMCONNECTENGINEUSER_H
#define SIMCONNECTENGINEUSER_H

#include <Kernel/Enum.h>
#include <Model/EngineData.h>
#include "SimConnectType.h"
#include "SimConnectEngineCommon.h"
#include "SimConnectEngineCore.h"

/*!
 * Engine simulation variables that are sent to the user aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectEngineUser
{
    SimConnectEngineCommon common;
    SimConnectEngineCore core;

    SimConnectEngineUser(const EngineData &data) noexcept
        : SimConnectEngineUser()
    {
        fromEngineData(data);
    }

    SimConnectEngineUser() = default;

    inline void fromEngineData(const EngineData &data)
    {
        common.fromEngineData(data);
        core.fromEngineData(data);
    }

    inline EngineData toEngineData() const noexcept
    {
        EngineData data = common.toEngineData();
        core.toEngineData(data);
        return data;
    }

    static inline void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectEngineCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineUser));
        SimConnectEngineCore::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineUser));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTENGINEUSER_H
