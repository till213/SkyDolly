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

#include "SimConnectEvent.h"

namespace EventState
{
    enum struct Engine: int {
        Unknown,
        Starting,
        Started,
        Stopped
    };

    template <typename T>
    struct Switch
    {
        T current {};
        T requested {};
        bool valid {false};
        bool pending {false};

        Switch() noexcept
        {
            reset();
        }

        /*!
         * Returns whether an update needs to be sent, as the \c requested value differs from the \c current
         * value.
         *
         * \return \c true if the \c current value is different from the \c requested value; \c false else
         */
        bool needsUpdate() const noexcept
        {
            return current != requested;
        }

        void reset() noexcept
        {
            if constexpr(std::is_integral_v<T>) {
                current = 0;
                requested = 0;
            }
            else if constexpr(std::is_floating_point_v<T>) {
                current = 0.0;
                requested = 0.0;
            } else if constexpr(std::is_same_v<T, bool>) {
                current = false;
                requested = false;
            } else {
                current = {};
                requested = {};
            }
            valid = false;
            pending = false;
        }
    };

    struct Toggle final : public Switch<bool>
    {
        SimConnectEvent::Event toggleEvent;

        Toggle(SimConnectEvent::Event toggleEvent) noexcept
            : toggleEvent(toggleEvent)
        {
            Switch<bool>::reset();
        }
    };
}

#endif // EVENTSTATE_H
