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
#ifndef SIMCONNECTATTITUDEALL_H
#define SIMCONNECTATTITUDEALL_H

#include <windows.h>
#include <SimConnect.h>

#include <Model/AttitudeData.h>
#include "SimConnectAttitudeCommon.h"
#include "SimConnectAttitudeInfo.h"

/*!
 * All aircraft attitude simulation variables (reply from the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectAttitudeAll
{
    SimConnectAttitudeCommon common;
    SimConnectAttitudeInfo info;

    SimConnectAttitudeAll(const AttitudeData &positionData) noexcept
        : SimConnectAttitudeAll()
    {
        fromAttitudeData(positionData);
    }

    SimConnectAttitudeAll() = default;

    inline void fromAttitudeData(const AttitudeData &data) noexcept
    {
        common.fromAttitudeData(data);
        info.fromAttitudeData(data);
    }

    inline AttitudeData toAttitudeData() const noexcept
    {
        auto positionData = common.toAttitudeData();
        info.toAttitudeData(positionData);
        return positionData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectAttitudeCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AttitudeAll));
        SimConnectAttitudeInfo::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::AttitudeAll));
    }

};
#pragma pack(pop)

#endif // SIMCONNECTATTITUDEALL_H
