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
#ifndef SIMCONNECTEVENT_H
#define SIMCONNECTEVENT_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Model/PrimaryFlightControlData.h>
#include "SimConnectType.h"

class SimConnectEvent
{
public:
    static constexpr std::int32_t InvalidFlapsIndex {-1};

    enum struct Event: ::SIMCONNECT_CLIENT_EVENT_ID {
        SimStart,
        Pause,
        Crashed,
        Frame,
        FreezeLatituteLongitude,
        FreezeAltitude,
        FreezeAttitude,
        EngineAutoStart,
        EngineAutoShutdown,
        // Primary flight controls
        AileronSet,
        ElevatorSet,
        RudderSet,
        // Secondary flight controls
        Flaps_Decrease,
        Flaps_Increase,
        GearSet
    };

    SimConnectEvent(::HANDLE simConnectHandle)
        : m_simConnectHandle(simConnectHandle)
    {}

    void setupEvents()
    {
        // System event subscription
        ::SimConnect_SubscribeToSystemEvent(m_simConnectHandle, Enum::underly(Event::SimStart), "SimStart");
        ::SimConnect_SubscribeToSystemEvent(m_simConnectHandle, Enum::underly(Event::Pause), "Pause");
        ::SimConnect_SubscribeToSystemEvent(m_simConnectHandle, Enum::underly(Event::Crashed), "Crashed");

        // Client events
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::FreezeLatituteLongitude), "FREEZE_LATITUDE_LONGITUDE_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::FreezeAltitude), "FREEZE_ALTITUDE_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::FreezeAttitude), "FREEZE_ATTITUDE_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::EngineAutoStart), "ENGINE_AUTO_START");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::EngineAutoShutdown), "ENGINE_AUTO_SHUTDOWN");
        // Primary flight controls
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::AileronSet), "AILERON_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::ElevatorSet), "ELEVATOR_SET");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::RudderSet), "RUDDER_SET");
        // Secondary flight controls
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::Flaps_Decrease), "FLAPS_DECR");
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::Flaps_Increase), "FLAPS_INCR");
        // Handles
        ::SimConnect_MapClientEventToSimEvent(m_simConnectHandle, Enum::underly(Event::GearSet), "GEAR_SET");
    }

    inline bool setPrimaryFlightControls(const PrimaryFlightControlData &primaryFlightControlData)
    {
        // Event values have opposite sign than recorded simulation variable values
        HRESULT res = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::RudderSet), -primaryFlightControlData.rudderPosition, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        res |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::AileronSet), -primaryFlightControlData.aileronPosition, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        res |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::ElevatorSet), -primaryFlightControlData.elevatorPosition, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return res == S_OK;
    }

    inline bool requestFlapsHandleIndex(std::int32_t index)
    {
        HRESULT res {S_OK};
        m_requestedFlapsIndex = index;
        if (m_requestedFlapsIndex != m_currentFlapsIndex) {
            if (m_currentFlapsIndex != InvalidFlapsIndex) {
                Event event = m_requestedFlapsIndex > m_currentFlapsIndex ? Event::Flaps_Increase : Event::Flaps_Decrease;
                const int steps = std::abs(m_currentFlapsIndex - m_requestedFlapsIndex);
                for (int step = 0; step < steps; ++step) {
                    res |= ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(event), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                }
#ifdef DEBUG
                qDebug() << "SimConnectEvent::requestFlapsHandleIndex: incrementally setting flaps handle index to:" << m_requestedFlapsIndex
                         << "Previous index:" << m_currentFlapsIndex
                         << "Steps:" << steps
                         << "Event ID:" << Enum::underly(event)
                         << "Success:" << (res == S_OK);
#endif
                if (res == S_OK) {
                    m_currentFlapsIndex = m_requestedFlapsIndex;
                }
            } else if (!m_pendingFlapsIndexRequest) {
                // Request current flaps index
                res = ::SimConnect_RequestDataOnSimObject(m_simConnectHandle, Enum::underly(SimConnectType::DataRequest::FlapsHandleIndex),
                                                          Enum::underly(SimConnectType::DataDefinition::FlapsHandleIndex),
                                                          ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_ONCE, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
                if (res == S_OK) {
                    m_pendingFlapsIndexRequest = true;
                }
#ifdef DEBUG
                qDebug() << "SimConnectEvent::requestFlapsHandleIndex: requesting current flaps index"
                         << "Current index:" << m_currentFlapsIndex
                         << "Success:" << (res == S_OK);
#endif
            }
        } // m_requestedFlapsIndex != m_currentFlapsIndex
        return res == S_OK;
    }

    inline bool setCurrentFlapsHandleIndex(std::int32_t index)
    {
        bool ok {true};
        m_pendingFlapsIndexRequest = false;
#ifdef DEBUG
                qDebug() << "SimConnectEvent::setCurrentFlapsHandleIndex: current index received:" << index
                         << "Previous index:" << m_currentFlapsIndex;
#endif
        m_currentFlapsIndex = index;
        if (m_requestedFlapsIndex != m_currentFlapsIndex) {
            ok = requestFlapsHandleIndex(m_requestedFlapsIndex);
        }
        return ok;
    }

    inline bool setGear(std::int32_t gearHandlePosition)
    {
        // Event values have opposite sign than recorded simulation variable values
        HRESULT res = ::SimConnect_TransmitClientEvent(m_simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(Event::GearSet), gearHandlePosition, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        return res == S_OK;
    }

    inline void reset() {
        m_currentFlapsIndex = InvalidFlapsIndex;
        m_requestedFlapsIndex = InvalidFlapsIndex;
        m_pendingFlapsIndexRequest = false;
    }

private:
    ::HANDLE m_simConnectHandle;
    std::int32_t m_currentFlapsIndex {InvalidFlapsIndex};
    std::int32_t m_requestedFlapsIndex {InvalidFlapsIndex};
    bool m_pendingFlapsIndexRequest {false};
};

#endif // SIMCONNECTEVENT_H
