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
#ifndef ENGINEDATA_H
#define ENGINEDATA_H

#include <QtGlobal>
#include <QFlags>

#include "SimType.h"
#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API EngineData : public TimeVariableData
{
    // Implementation note: the throttle and propeller levers can also yield
    // negative thrust, hence the type qint16 (position) which also supports negative values
    double generalEngineFuelPressure1;
    double generalEngineFuelPressure2;
    double generalEngineFuelPressure3;
    double generalEngineFuelPressure4;
    qint16 throttleLeverPosition1;
    qint16 throttleLeverPosition2;
    qint16 throttleLeverPosition3;
    qint16 throttleLeverPosition4;
    qint16 propellerLeverPosition1;
    qint16 propellerLeverPosition2;
    qint16 propellerLeverPosition3;
    qint16 propellerLeverPosition4;
    quint8 mixtureLeverPosition1;
    quint8 mixtureLeverPosition2;
    quint8 mixtureLeverPosition3;
    quint8 mixtureLeverPosition4;
    quint8 cowlFlapPosition1;
    quint8 cowlFlapPosition2;
    quint8 cowlFlapPosition3;
    quint8 cowlFlapPosition4;

    bool electricalMasterBattery1;
    bool electricalMasterBattery2;
    bool electricalMasterBattery3;
    bool electricalMasterBattery4;
    bool generalEngineStarter1;
    bool generalEngineStarter2;
    bool generalEngineStarter3;
    bool generalEngineStarter4;
    bool generalEngineCombustion1;
    bool generalEngineCombustion2;
    bool generalEngineCombustion3;
    bool generalEngineCombustion4;

    EngineData(qint16 throttleLeverPosition1 = 0, qint16 propellerLeverPosition1 = 0, quint8 mixtureLeverPosition1 = 0, quint8 cowlFlapPosition1 = 0) noexcept;
    EngineData(EngineData &&) = default;
    EngineData(const EngineData &) = default;
    EngineData &operator= (const EngineData &) = default;

    static const EngineData NullEngineData;
};

#endif // ENGINEDATA_H
