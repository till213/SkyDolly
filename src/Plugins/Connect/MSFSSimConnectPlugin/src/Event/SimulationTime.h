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
#ifndef SIMULATIONTIME_H
#define SIMULATIONTIME_H


#include <windows.h>
#include <SimConnect.h>

#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Model/SimType.h>
#include <Model/TimeVariableData.h>
#include "SimConnectEvent.h"

class SimulationTime final
{
public:
    inline bool sendDateAndTime(HANDLE simConnectHandle, DWORD year, DWORD day, DWORD hours, DWORD minutes) noexcept
    {
        HRESULT result {S_OK};
        result = ::SimConnect_TransmitClientEvent(
            simConnectHandle,
            ::SIMCONNECT_OBJECT_ID_USER,
            Enum::underly(SimConnectEvent::Event::ZuluYearSet), year,
            ::SIMCONNECT_GROUP_PRIORITY_HIGHEST,
            ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
        );
        result |= ::SimConnect_TransmitClientEvent(
            simConnectHandle,
            ::SIMCONNECT_OBJECT_ID_USER,
            Enum::underly(SimConnectEvent::Event::ZuluDaySet), day,
            ::SIMCONNECT_GROUP_PRIORITY_HIGHEST,
            ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
        );
        result |= ::SimConnect_TransmitClientEvent(
            simConnectHandle,
            ::SIMCONNECT_OBJECT_ID_USER,
            Enum::underly(SimConnectEvent::Event::ZuluHoursSet), hours,
            ::SIMCONNECT_GROUP_PRIORITY_HIGHEST,
            ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
        );
        result |= ::SimConnect_TransmitClientEvent(
            simConnectHandle,
            ::SIMCONNECT_OBJECT_ID_USER,
            Enum::underly(SimConnectEvent::Event::ZuluMinutesSet), minutes,
            ::SIMCONNECT_GROUP_PRIORITY_HIGHEST,
            ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY
        );

        return result == S_OK;
    }
};

#endif // SIMULATIONTIME_H

