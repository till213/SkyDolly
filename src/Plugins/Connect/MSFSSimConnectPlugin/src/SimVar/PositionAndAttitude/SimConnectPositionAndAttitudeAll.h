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
#ifndef SIMCONNECTPOSITIONANDATTITUDEALL_H
#define SIMCONNECTPOSITIONANDATTITUDEALL_H

#include <windows.h>
#include <SimConnect.h>

#include <Model/PositionData.h>
#include <Model/AttitudeData.h>
#include <Model/InitialPosition.h>
#include "../Position/SimConnectPositionCommon.h"
#include "../Attitude/SimConnectAttitudeCommon.h"
#include "SimConnectPositionAndAttitudeUser.h"
#include "SimConnectPositionAndAttitudeAi.h"

/*!
 * All aircraft position and attitude simulation variables (request to the flight simulator).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectPositionAndAttitudeAll
{
    SimConnectPositionCommon positionCommon;
    SimConnectAttitudeCommon attitudeCommon;

    SimConnectPositionAndAttitudeAll(const PositionData &positionData, const AttitudeData &attitudeData) noexcept
        : SimConnectPositionAndAttitudeAll()
    {
        fromPositionData(positionData);
        fromAttitudeData(attitudeData);
    }

    SimConnectPositionAndAttitudeAll() = default;

    inline void fromPositionData(const PositionData &positionData) noexcept
    {
        positionCommon.fromPositionData(positionData);
    }

    inline void fromAttitudeData(const AttitudeData &attitudeData) noexcept
    {
        attitudeCommon.fromAttitudeData(attitudeData);
    }

    inline SimConnectPositionAndAttitudeUser user() const noexcept
    {
        SimConnectPositionAndAttitudeUser user;
        user.positionCommon = positionCommon;
        user.attitudeCommon = attitudeCommon;
        return user;
    }

    inline SimConnectPositionAndAttitudeAi ai() const noexcept
    {
        SimConnectPositionAndAttitudeAi ai;
        ai.positionCommon = positionCommon;
        ai.attitudeCommon = attitudeCommon;
        return ai;
    }

    static inline SIMCONNECT_DATA_INITPOSITION toInitialPosition(const PositionData &positionData, const AttitudeData &attitudeData, int initialAirspeed)
    {
        SIMCONNECT_DATA_INITPOSITION initialPosition {};

        initialPosition.Latitude = positionData.latitude;
        initialPosition.Longitude = positionData.longitude;
        initialPosition.Altitude = positionData.altitude;
        initialPosition.Pitch = attitudeData.pitch;
        initialPosition.Bank = attitudeData.bank;
        initialPosition.Heading = attitudeData.trueHeading;
        initialPosition.OnGround = attitudeData.onGround ? 1 : 0;
        initialPosition.Airspeed = initialAirspeed;

        return initialPosition;
    }

    static inline SIMCONNECT_DATA_INITPOSITION toInitialPosition(const InitialPosition &initialPosition)
    {
        SIMCONNECT_DATA_INITPOSITION initialSimConnnectPosition {};

        initialSimConnnectPosition.Latitude = initialPosition.latitude;
        initialSimConnnectPosition.Longitude = initialPosition.longitude;
        initialSimConnnectPosition.Altitude = initialPosition.altitude;
        initialSimConnnectPosition.Pitch = initialPosition.pitch;
        initialSimConnnectPosition.Bank = initialPosition.bank;
        initialSimConnnectPosition.Heading = initialPosition.trueHeading;
        initialSimConnnectPosition.OnGround = initialPosition.onGround ? 1 : 0;
        initialSimConnnectPosition.Airspeed = initialPosition.indicatedAirspeed;

        return initialSimConnnectPosition;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept
    {
        SimConnectPositionCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionAndAttitudeAll));
        SimConnectAttitudeCommon::addToDataDefinition(simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionAndAttitudeAll));
    }
};
#pragma pack(pop)

#endif // SIMCONNECTPOSITIONANDATTITUDEALL_H
