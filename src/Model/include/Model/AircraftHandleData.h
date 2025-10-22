/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#ifndef AIRCRAFTHANDLEDATA_H
#define AIRCRAFTHANDLEDATA_H

#include <cstdint>

#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API AircraftHandleData final : public TimeVariableData
{
    std::int16_t brakeLeftPosition {0};
    std::int16_t brakeRightPosition {0};
    std::int16_t gearSteerPosition {0};
    // Implementation note: the water rudder can also have negative (-100.0) values,
    // hence hence the type std::int16_t (position) which also supports negative values
    std::int16_t waterRudderHandlePosition {0};
    std::uint8_t tailhookPosition {0};
    std::uint8_t canopyOpen {0};
    std::uint8_t leftWingFolding {0};
    std::uint8_t rightWingFolding {0};
    // True: up - false: down
    bool gearHandlePosition {false};
    // True: engaged - false: disengaged
    bool tailhookHandlePosition {false};
    // True: retracted - false: extended
    bool foldingWingHandlePosition {false};
    // True: enabled - false: disabled
    bool smokeEnabled {false};
};

#endif // AIRCRAFTHANDLEDATA_H

