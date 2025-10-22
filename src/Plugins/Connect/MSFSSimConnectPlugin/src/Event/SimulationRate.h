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
#ifndef SIMULATIONRATE_H
#define SIMULATIONRATE_H

#include <cstdlib>

#include <windows.h>
#include <SimConnect.h>

#ifdef DEBUG
#include <QDebug>
#endif

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

    inline bool requestSimulationRate(HANDLE simConnectHandle)
    {
        HRESULT result = ::SimConnect_RequestDataOnSimObject(simConnectHandle, Enum::underly(SimConnectType::DataRequest::SimulationRate),
                                                             Enum::underly(SimConnectType::DataDefinition::SimulationRate),
                                                             ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_ONCE, ::SIMCONNECT_DATA_REQUEST_FLAG_DEFAULT);
        return result == S_OK;
    }

private:
    EventState::StatefulSwitch<int> m_simulationRateIndex;

    // TODO: There is (now?) actually a SIM_RATE_SET event that supposedly takes the desired rate directly - does that work or not? -> Test
    //       https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Miscellaneous_Events.htm#SIM_RATE_SET
    inline bool sendSimulationRate(HANDLE simConnectHandle) noexcept
    {
        HRESULT result {S_OK};
        if (m_simulationRateIndex.needsUpdate()) {
            if (m_simulationRateIndex.valid) {
                SimConnectEvent::Event event = m_simulationRateIndex.requested > m_simulationRateIndex.current ? SimConnectEvent::Event::SimRateIncr : SimConnectEvent::Event::SimRateDecr;
                const int steps = std::abs(m_simulationRateIndex.current - m_simulationRateIndex.requested);
                for (int step = 0; step < steps; ++step) {
                    result |= ::SimConnect_TransmitClientEvent(simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(event), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
                }
#ifdef DEBUG
                qDebug() << "SimulationRate::sendSimulationRate: incrementally setting simulation rate"
                         << "from" << m_simulationRateIndex.current
                         << "to:" << m_simulationRateIndex.requested
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
                const bool ok = requestSimulationRate(simConnectHandle);
                result = ok ? S_OK : S_FALSE;
                if (ok) {
                    m_simulationRateIndex.pending = true;
                }
            }
        } // needsUpdate
        return result == S_OK;
    }

    // Returns the simulation rate index according to 'rate' as follows:
    //
    // Index : Standard Rate
    // 0: 0.0625, 1: 0.125, 2: 0.25, 3: 0.5, 4: 1, 5: 2, 6: 4, 7: 8, 8: 16, 9: 32, 10: 64, 11: 128
    //
    // The 'rate' is rounded to the next standard rate (0.0625, 0.125, 0.25, 0.5, 1, 2, 4, ... 128),
    // or in other words: index 0 for any rate < (0.0625f + 0.125f) / 2.0f and index 11 for any rate >= 96.0
    //
    // Also refer to: https://docs.flightsimulator.com/html/Programming_Tools/Programming_APIs.htm#SIMULATION%20RATE
    static constexpr int rateToIndex(float rate) noexcept
    {
        int index {0};
        if (rate < (0.0625f + 0.125f) / 2.0f) {
            index = 0;
        } else if (rate < (0.125f + 0.25f) / 2.0f) {
            index = 1;
        } else if (rate < (0.25f + 0.5f) / 2.0f) {
            index = 2;
        } else if (rate < (0.5f + 1.0f) / 2.0f) {
            index = 3;
        } else if (rate < (1.0f + 2.0f) / 2.0f) {
            index = 4;
        } else if (rate < (2.0f + 4.0f) / 2.0f) {
            index = 5;
        } else if (rate < (4.0f + 8.0f) / 2.0f) {
            index = 6;
        } else if (rate < (8.0f + 16.0f) / 2.0f) {
            index = 7;
        } else if (rate < (16.0f + 32.0f) / 2.0f) {
            index = 8;
        } else if (rate < (32.0f + 64.0f) / 2.0f) {
            index = 9;
        } else if (rate < (64.0f + 128.0f) / 2.0f) {
            index = 10;
        } else {
            index = 11;
        };
        return index;
    }
};

#endif // SIMULATIONRATE_H
