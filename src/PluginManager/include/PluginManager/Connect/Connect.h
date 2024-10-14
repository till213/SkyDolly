/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#ifndef CONNECT_H
#define CONNECT_H

#include <cstdint>

namespace Connect {
    enum struct State: std::uint8_t {
        Disconnected,
        Connected,
        Recording,
        RecordingPaused,
        Replay,
        ReplayPaused
    };

    /*!
     * A flag that indicates whether a reconnect to the flight simulator is required after
     * settings have changed.
     */
    enum struct Mode: std::uint8_t
    {
        /*!
         * A complete reconnect is required, that is existing connections are to
         * to be terminated. This is typically the case when changing connection
         * options such as the network configuration.
         */
        Reconnect,
        /*!
         * Only changing the flight simulation connection setup such is required,
         * taking the new settings such as keyboard shortcuts into account.
         * However a connection is still to be established if not existing yet.
         */
        SetupOnly,
        /*!
         * No reconnect required; settings will typically take effect upon the
         * next recording or replay
         */
        NotRequired
    };
}

#endif // CONNECT_H
