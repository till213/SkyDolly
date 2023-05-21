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
#ifndef CLIENTEVENTSHORTCUTS_H
#define CLIENTEVENTSHORTCUTS_H

#include <QKeySequence>

#include "KernelLib.h"

struct KERNEL_API ClientEventShortcuts
{
public:
    QKeySequence record;
    QKeySequence replay;
    QKeySequence pause;
    QKeySequence stop;
    QKeySequence backward;
    QKeySequence forward;
    QKeySequence rewind;

    bool hasShortCut() const noexcept {
        return !record.isEmpty() ||
               !replay.isEmpty() ||
               !pause.isEmpty() ||
               !stop.isEmpty() ||
               !backward.isEmpty() ||
               !forward.isEmpty() ||
               !rewind.isEmpty();
    }

};

inline bool operator==(const ClientEventShortcuts &lhs, const ClientEventShortcuts &rhs) noexcept
{
    return lhs.record == rhs.record &&
           lhs.replay == rhs.replay &&
           lhs.pause == rhs.pause &&
           lhs.stop == rhs.stop &&
           lhs.backward == rhs.backward &&
           lhs.forward == rhs.forward &&
           lhs.rewind == rhs.rewind;
}

inline bool operator!=(const ClientEventShortcuts &lhs, const ClientEventShortcuts &rhs) noexcept
{
    return !(lhs == rhs);
}

#endif // CLIENTEVENTSHORTCUTS_H
