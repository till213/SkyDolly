/**
 * Sky Dolly - The black sheep for your flight recordings
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
#ifndef FLIGHTSIMULATOR_H
#define FLIGHTSIMULATOR_H

#include <QString>

#include "KernelLib.h"

class KERNEL_API FlightSimulator
{
public:
    FlightSimulator();

    enum class Id {
        None,
        All,
        FS2020,
        Prepar3Dv5
    };

    static const char FlightSimulatorNameAll[];
    static const char FlightSimulatorNameFS2020[];
    static const char FlightSimulatorNamePrepar3Dv5[];

    static inline Id nameToId(const QString &name) noexcept {
        Id id;
        if (name == QString(FlightSimulatorNameAll)) {
            id = Id::All;
        } else if (name == QString(FlightSimulatorNameFS2020)) {
            id = Id::FS2020;
        } else if (name == QString(FlightSimulatorNamePrepar3Dv5)) {
            id = Id::Prepar3Dv5;
        } else {
            id = Id::None;
        }
        return id;
    }

    static bool isRunning(Id id) noexcept;
    static bool isInstalled(Id id) noexcept;

private:
    static bool isFS2020Installed() noexcept;
};

#endif // FLIGHTSIMULATOR_H
