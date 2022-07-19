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
#ifndef AIRCRAFTHANDLEDATA_H
#define AIRCRAFTHANDLEDATA_H

#include <cstdint>

#include <QtGlobal>
#include <QFlags>
#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API AircraftHandleData : public TimeVariableData
{
    std::int16_t brakeLeftPosition;
    std::int16_t brakeRightPosition;
    // Implementation note: the water rudder can also have negative (-100.0) values,
    // hence hence the type std::int16_t (position) which also supports negative values
    std::int16_t waterRudderHandlePosition;
    std::uint8_t tailhookPosition;
    std::uint8_t canopyOpen;
    std::uint8_t leftWingFolding;
    std::uint8_t rightWingFolding;
    bool gearHandlePosition;
    bool smokeEnabled;

    AircraftHandleData() noexcept;
    AircraftHandleData(const AircraftHandleData &other) = default;
    AircraftHandleData(AircraftHandleData &&other) = default;
    ~AircraftHandleData() override = default;
    AircraftHandleData &operator=(const AircraftHandleData &rhs) = default;
    AircraftHandleData &operator=(AircraftHandleData &&rhs) = default;

    static const AircraftHandleData NullData;
};

#endif // AIRCRAFTHANDLEDATA_H

