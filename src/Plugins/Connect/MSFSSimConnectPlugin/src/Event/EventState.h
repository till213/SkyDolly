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
#ifndef EVENTSTATE_H
#define EVENTSTATE_H

#include <type_traits>

#include "SimConnectType.h"
#include "SimConnectEvent.h"

namespace EventState
{
    enum struct Engine: int {
        Unknown,
        Starting,
        Started,
        Stopped
    };

    /*!
     * The stateless switch - also known as "test and set"-switch - only stores the requested
     * value. Each time a new request is made the remote switch state (remote: <em>in the flight simulator</em>)
     * has to be queried ("test"), and if different needs to be updated accordingly (by sending the
     * corresponding event: "set").
     *
     * This switch is useful for scenarios where switches may be dependent on each other: for instance
     * in the A320neo the Navigation and Logo lights are connected (in fact, it's a single switch for
     * both light states).
     */
    template <typename T>
    struct StatelessSwitch
    {
        /*!
         * The requested value.
         */
        T requested {};

        /*!
         * This flag indicates whether a "test" query is pending (\c true) or not (\c false).
         */
        bool pending {false};

        StatelessSwitch() noexcept
        {
            reset();
        }

        /*!
         * Returns whether an update needs to be sent, as the \c requested value differs from the \c current
         * value.
         *
         * \param current
         *        the current value, typically as just received from the flight simulator ("test" reply)
         * \return \c true if the \c current value is different from the \c requested value; \c false else
         */
        bool needsUpdate(T current) const noexcept
        {
            return current != requested;
        }

        void reset() noexcept
        {
            if constexpr(std::is_integral_v<T>) {
                requested = 0;
            }
            else if constexpr(std::is_floating_point_v<T>) {
                requested = 0.0;
            } else if constexpr(std::is_same_v<T, bool>) {
                requested = false;
            } else {
                requested = {};
            }
            pending = false;
        }
    };

    /*!
     * The stateful switch stores the last requested value - the state. It is expected that the state
     * of the switch and the corresponding simulation variable state in the flight simulator are
     * always kept in sync, hence the state is only ever queried and updated when \c valid is
     * \c false (typically when resetting the state after a timeline \em seek operation).
     */
    template <typename T>
    struct StatefulSwitch : public StatelessSwitch<T>
    {
        /*! The current value (state) */
        T current {};

        /*! This flag indicates whether the current value is valid (\c true) or not (\c false). */
        bool valid {false};

        StatefulSwitch() noexcept
        {
            reset();
        }

        /*!
         * Returns whether an update needs to be sent, as the \c requested value differs from the \c current
         * value.
         *
         * \return \c true if the \c current value is different from the \c requested value, or the \c current
         *         value is not \c valid; \c false else
         */
        bool needsUpdate() const noexcept
        {
            return !valid || StatelessSwitch<T>::needsUpdate(current);
        }

        void reset() noexcept
        {
            StatelessSwitch<T>::reset();
            if constexpr(std::is_integral_v<T>) {
                current = 0;
            }
            else if constexpr(std::is_floating_point_v<T>) {
                current = 0.0;
            } else if constexpr(std::is_same_v<T, bool>) {
                current = false;
            } else {
                current = {};
            }
            valid = false;   
        }
    };

    /*!
     * A stateless boolean switch represening "on" and "off" states (only).
     * It stores the \c toggleEvent to toggle beweteen those two states.
     *
     * The remote state needs to be tested each time a value is requested.
     */
    struct StatelessToggle final : public StatelessSwitch<bool>
    {
        SimConnectEvent::Event toggleEvent;
        SimConnectType::DataRequest dataRequest;
        SimConnectType::DataDefinition dataDefinition;


        StatelessToggle(SimConnectEvent::Event toggleEvent, SimConnectType::DataRequest dataRequest, SimConnectType::DataDefinition dataDefinition) noexcept
            : toggleEvent(toggleEvent),
              dataRequest(dataRequest),
              dataDefinition(dataDefinition)
        {
            StatelessSwitch<bool>::reset();
        }
    };

    /*!
     * A stateful boolean switch represening "on" and "off" states (only).
     * It stores the \c toggleEvent to toggle beweteen those two states.
     *
     * The remote state is expected to be in sync with the state of this toggle.
     */
    struct StatefulToggle final : public StatefulSwitch<bool>
    {
        SimConnectEvent::Event toggleEvent;

        StatefulToggle(SimConnectEvent::Event toggleEvent) noexcept
            : toggleEvent(toggleEvent)
        {
            StatefulSwitch<bool>::reset();
        }
    };
}

#endif // EVENTSTATE_H
