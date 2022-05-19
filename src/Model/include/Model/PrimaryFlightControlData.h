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
#ifndef PRIMARYFLIGHTCONTROLDATA_H
#define PRIMARYFLIGHTCONTROLDATA_H

#include <cstdint>

#include <QtGlobal>
#include <QFlags>

#include "SimType.h"
#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API PrimaryFlightControlData : public TimeVariableData
{
    std::int16_t rudderPosition;
    std::int16_t elevatorPosition;
    std::int16_t aileronPosition;

    PrimaryFlightControlData() noexcept;
    PrimaryFlightControlData(const PrimaryFlightControlData &other) = default;
    PrimaryFlightControlData(PrimaryFlightControlData &&other) = default;
    ~PrimaryFlightControlData() override = default;
    PrimaryFlightControlData &operator=(const PrimaryFlightControlData &) = default;
    PrimaryFlightControlData &operator=(PrimaryFlightControlData &&) = default;

    static const PrimaryFlightControlData NullData;
};

#endif // PRIMARYFLIGHTCONTROLDATA_H
