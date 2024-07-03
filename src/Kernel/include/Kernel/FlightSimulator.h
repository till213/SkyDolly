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
#ifndef FLIGHTSIMULATOR_H
#define FLIGHTSIMULATOR_H

#include <QString>

#include "KernelLib.h"

class KERNEL_API FlightSimulator final
{
public:
    enum struct Id {
        None,
        All,
        MSFS,
        Prepar3Dv5
    };

    static constexpr const char *FlightSimulatorNameAll {"All"};
    static constexpr const char *FlightSimulatorNameMSFS {"MSFS"};
    static constexpr const char *FlightSimulatorNamePrepar3Dv5 {"Prepar3Dv5"};

    static inline Id nameToId(const QString &name) noexcept {
        Id id {Id::None};
        if (name == FlightSimulatorNameAll) {
            id = Id::All;
        } else if (name == FlightSimulatorNameMSFS) {
            id = Id::MSFS;
        } else if (name == FlightSimulatorNamePrepar3Dv5) {
            id = Id::Prepar3Dv5;
        }
        return id;
    }

    static bool isRunning(Id id) noexcept;
    static bool isInstalled(Id id) noexcept;

private:
    static bool isMSFSInstalled() noexcept;
};

#endif // FLIGHTSIMULATOR_H
