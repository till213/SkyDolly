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
#ifndef SIMCONNECTPOSITIONALL_H
#define SIMCONNECTPOSITIONALL_H

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/Enum.h>
#include <Model/PositionData.h>
#include "SimConnectPositionCommon.h"
#include "SimConnectType.h"
#include "SimConnectPositionInfo.h"

/*!
 * All aircraft position simulation variables (reply from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPositionAll
{
    SimConnectPositionCommon common;
    SimConnectPositionInfo info;

    SimConnectPositionAll(const PositionData &data) noexcept
        : SimConnectPositionAll()
    {
        fromPositionData(data);
    }

    SimConnectPositionAll() = default;

    inline void fromPositionData(const PositionData &data) noexcept
    {
        common.fromPositionData(data);
        info.fromPositionData(data);
    }

    inline PositionData toPositionData() const noexcept
    {
        auto data = common.toPositionData();
        info.toPositionData(data);
        return data;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectPositionCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionAll));
        SimConnectPositionInfo::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionAll));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTPOSITIONALL_H
