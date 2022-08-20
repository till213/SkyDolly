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
#include <cstdint>

#include "TimeVariableData.h"
#include "EngineData.h"

// PUBLIC

EngineData::EngineData(std::int16_t theThrottleLeverPosition1, std::int16_t thePropellerLeverPosition1, std::uint8_t theMixtureLeverPosition1, std::uint8_t theCowlFlapPosition1) noexcept
    : TimeVariableData(),
      throttleLeverPosition1(theThrottleLeverPosition1),
      throttleLeverPosition2(0),
      throttleLeverPosition3(0),
      throttleLeverPosition4(0),
      propellerLeverPosition1(thePropellerLeverPosition1),
      propellerLeverPosition2(0),
      propellerLeverPosition3(0),
      propellerLeverPosition4(0),
      mixtureLeverPosition1(theMixtureLeverPosition1),
      mixtureLeverPosition2(0),
      mixtureLeverPosition3(0),
      mixtureLeverPosition4(0),
      cowlFlapPosition1(theCowlFlapPosition1),
      cowlFlapPosition2(0),
      cowlFlapPosition3(0),
      cowlFlapPosition4(0),
      electricalMasterBattery1(false),
      electricalMasterBattery2(false),
      electricalMasterBattery3(false),
      electricalMasterBattery4(false),
      generalEngineStarter1(false),
      generalEngineStarter2(false),
      generalEngineStarter3(false),
      generalEngineStarter4(false),
      generalEngineCombustion1(false),
      generalEngineCombustion2(false),
      generalEngineCombustion3(false),
      generalEngineCombustion4(false)
{}
