/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#ifndef REPLAY_H
#define REPLAY_H

#include <cstdint>

/*!
 * Replay related data structures.
 */
namespace Replay
{
    /*!
     * The replay speed factor: absolute or percent.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct SpeedUnit: std::uint8_t {
        First = 0,
        Absolute = First,
        Percent,
        Last = Percent
    };

    /*!
     * Defines how the time is to be synchronised during replay.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct TimeMode: std::uint8_t
    {
        First = 0,
        /*! No time synchronisation is done: the current simulation time is kept */
        None = First,
        /*! The simulation time is synchronised, starting from the user aircraft simulation local start date/time (zulu). */
        SimulationTime,
        /*! The real-world local time is synchronised, starting from the flight recording date/time. */
        CreationRealWorldTime,
        Last =  CreationRealWorldTime
    };
}

#endif // REPLAY_H
