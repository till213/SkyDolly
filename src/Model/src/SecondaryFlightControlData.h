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
#ifndef SECONDARYFLIGHTCONTROLDATA_H
#define SECONDARYFLIGHTCONTROLDATA_H

#include <cstdint>

#include <QtGlobal>
#include <QFlags>

#include "SimType.h"
#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API SecondaryFlightControlData : public TimeVariableData
{
    // Flaps & speed brakes
    std::int16_t leadingEdgeFlapsLeftPosition;
    std::int16_t leadingEdgeFlapsRightPosition;
    std::int16_t trailingEdgeFlapsLeftPosition;
    std::int16_t trailingEdgeFlapsRightPosition;
    std::int16_t spoilersHandlePosition;
    std::int8_t flapsHandleIndex;

    SecondaryFlightControlData() noexcept;
    SecondaryFlightControlData(SecondaryFlightControlData &&) = default;
    SecondaryFlightControlData(const SecondaryFlightControlData &) = default;
    SecondaryFlightControlData &operator= (const SecondaryFlightControlData &) = default;

    static const SecondaryFlightControlData NullData;
};

#endif // SECONDARYFLIGHTCONTROLDATA_H
