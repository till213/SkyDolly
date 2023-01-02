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
#ifndef SIMCONNECTPOSITIONAI_H
#define SIMCONNECTPOSITIONAI_H

#include <Kernel/Enum.h>
#include <Model/PositionData.h>
#include "SimConnectType.h"
#include "SimConnectPositionCommon.h"

/*!
 * Position simulation variables that are sent to AI aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPositionAi
{
    SimConnectPositionCommon common;

    SimConnectPositionAi(const PositionData &positionData) noexcept
        : SimConnectPositionAi()
    {
        fromPositionData(positionData);
    }

    SimConnectPositionAi() = default;

    inline void fromPositionData(const PositionData &positionData)
    {
        common.fromPositionData(positionData);
    }

    inline PositionData toPositionData() const noexcept
    {
        PositionData positionData = common.toPositionData();
        return positionData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectPositionCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionAi));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTPOSITIONAI_H
