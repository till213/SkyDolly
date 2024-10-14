/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef ENGINEDATA_H
#define ENGINEDATA_H

#include <cstdint>

#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API EngineData final : public TimeVariableData
{
    // Implementation note: the throttle and propeller levers can also yield
    // negative thrust, hence the type std::int16_t (position) which also supports negative values
    std::int16_t throttleLeverPosition1 {0};
    std::int16_t throttleLeverPosition2 {0};
    std::int16_t throttleLeverPosition3 {0};
    std::int16_t throttleLeverPosition4 {0};
    std::int16_t propellerLeverPosition1 {0};
    std::int16_t propellerLeverPosition2 {0};
    std::int16_t propellerLeverPosition3 {0};
    std::int16_t propellerLeverPosition4 {0};
    std::uint8_t mixtureLeverPosition1 {0};
    std::uint8_t mixtureLeverPosition2 {0};
    std::uint8_t mixtureLeverPosition3 {0};
    std::uint8_t mixtureLeverPosition4 {0};
    std::uint8_t cowlFlapPosition1 {0};
    std::uint8_t cowlFlapPosition2 {0};
    std::uint8_t cowlFlapPosition3 {0};
    std::uint8_t cowlFlapPosition4 {0};

    bool electricalMasterBattery1 {false};
    bool electricalMasterBattery2 {false};
    bool electricalMasterBattery3 {false};
    bool electricalMasterBattery4 {false};
    bool generalEngineStarter1 {false};
    bool generalEngineStarter2 {false};
    bool generalEngineStarter3 {false};
    bool generalEngineStarter4 {false};
    bool generalEngineCombustion1 {false};
    bool generalEngineCombustion2 {false};
    bool generalEngineCombustion3 {false};
    bool generalEngineCombustion4 {false};

    explicit EngineData(std::int16_t throttleLeverPosition1 = 0, std::int16_t propellerLeverPosition1 = 0, std::uint8_t mixtureLeverPosition1 = 0, std::uint8_t cowlFlapPosition1 = 0) noexcept;

    inline bool hasEngineStarterEnabled() const noexcept
    {
        return (generalEngineStarter1 || generalEngineStarter2 || generalEngineStarter3 || generalEngineStarter4);
    }

    inline bool hasCombustion() const noexcept
    {
        return (generalEngineCombustion1 || generalEngineCombustion2 || generalEngineCombustion3 || generalEngineCombustion4);
    }
};

#endif // ENGINEDATA_H
