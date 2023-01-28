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
#ifndef SIMULATIONRATE_H
#define SIMULATIONRATE_H

#ifdef DEBUG
#include <QDebug>
#endif

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/Enum.h>
#include "EventState.h"

/*!
 * Controls the simulation rate in discrete steps (powers of 2).
 */
class SimulationRate final
{
public:
    inline bool sendSimulationRate(HANDLE simConnectHandle, float rate) noexcept
    {
        m_simulationRateIndex.requested = rateToIndex(rate);
        return sendSimulationRate(simConnectHandle);
    }

    inline void setCurrentSimulationRate(HANDLE simConnectHandle, float rate) noexcept
    {
        m_simulationRateIndex.current = rateToIndex(rate);
        m_simulationRateIndex.valid = true;
        sendSimulationRate(simConnectHandle);
    }

    inline void reset() noexcept
    {
        m_simulationRateIndex.reset();
    }

private:
    EventState::StatefulSwitch<int> m_simulationRateIndex;

    inline bool sendSimulationRate(HANDLE simConnectHandle) noexcept
    {
        HRESULT result {S_OK};
        if (m_simulationRateIndex.needsUpdate()) {
            if (m_simulationRateIndex.valid) {
                SimConnectEvent::Event event = m_simulationRateIndex.requested > m_simulationRateIndex.current ? SimConnectEvent::Event::SimRateIncr : SimConnectEvent::Event::SimRateDecr;
                const int steps = std::abs(m_simulationRateIndex.current - m_simulationRateIndex.requested);
                for (int step = 0.0; step < steps; ++step) {
                    result |= ::SimConnect_TransmitClientEvent(simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(event), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                }
#ifdef DEBUG
                qDebug() << "SimulationRate::sendSimulationRate: incrementally setting simulation rate index to:" << m_simulationRateIndex.requested
                         << "Previous rate index:" << m_simulationRateIndex.current
                         << "Steps:" << steps
                         << "Event ID:" << Enum::underly(event)
                         << "Success:" << (result == S_OK);
#endif
                if (result == S_OK) {
                    m_simulationRateIndex.current = m_simulationRateIndex.requested;
                }
                m_simulationRateIndex.pending = false;
            } else if (!m_simulationRateIndex.pending) {
                // Request current simulation rate
                result = ::SimConnect_RequestDataOnSimObject(simConnectHandle, Enum::underly(SimConnectType::DataRequest::SimulationRate),
                                                             Enum::underly(SimConnectType::DataDefinition::SimulationRate),
                                                             ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_ONCE, ::SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT);
                if (result == S_OK) {
                    m_simulationRateIndex.pending = true;
                }
            }
        } // needsUpdate
        return result == S_OK;
    }

    // Returns the simulation rate index according to 'rate' as follows:
    //
    // Rate:index:
    // 0.25:0, 0.5:1, 1.0:2, 2.0:3, 4.0:4, 8.0:5, 16.0:6
    //
    // The 'rate' is rounded to the next standard rate (0.25, 0.5, 1.0, 2.0, ..., 16.0),
    // or in other words: index 0 for any rate < 0.375 and index 6 for any rate >= 12.0
    static constexpr inline int rateToIndex(float rate) noexcept
    {
        int index {0};
        if (rate < 0.375f) {
            index = 0;
        } else if (rate < 0.75f) {
            index = 1;
        } else if (rate < 1.5f) {
            index = 2;
        } else if (rate < 3.0f) {
            index = 3;
        } else if (rate < 6.0f) {
            index = 4;
        } else if (rate < 12.0f) {
            index = 5;
        } else {
            index = 6;
        };
        return index;
    }
};

#endif // SIMULATIONRATE_H
