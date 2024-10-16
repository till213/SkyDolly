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
#ifndef SIMCONNECTPOSITIONANDATTITUDEAI_H
#define SIMCONNECTPOSITIONANDATTITUDEAI_H

#include <Kernel/Enum.h>
#include <Model/PositionData.h>
#include <Model/AttitudeData.h>
#include "SimConnectType.h"
#include "../Position/SimConnectPositionCommon.h"
#include "../Attitude/SimConnectAttitudeCommon.h"

/*!
 * Position and attitude simulation variables that are sent to AI aircraft.
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPositionAndAttitudeAi
{
    SimConnectPositionCommon positionCommon;
    SimConnectAttitudeCommon attitudeCommon;

    SimConnectPositionAndAttitudeAi(const PositionData &positionData, const AttitudeData &attitudeData) noexcept
        : SimConnectPositionAndAttitudeAi()
    {
        fromPositionData(positionData);
        fromAttitudeData(attitudeData);
    }

    SimConnectPositionAndAttitudeAi() = default;

    inline void fromPositionData(const PositionData &positionData)
    {
        positionCommon.fromPositionData(positionData);
    }

    inline PositionData toPositionData() const noexcept
    {
        const auto positionData = positionCommon.toPositionData();
        return positionData;
    }

    inline void fromAttitudeData(const AttitudeData &attitudeData)
    {
        attitudeCommon.fromAttitudeData(attitudeData);
    }

    inline AttitudeData toAttitudeData() const noexcept
    {
        const auto attitudeData = attitudeCommon.toAttitudeData();
        return attitudeData;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectPositionCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionAndAttitudeAi));
        SimConnectAttitudeCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionAndAttitudeAi));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTPOSITIONANDATTITUDEAI_H
