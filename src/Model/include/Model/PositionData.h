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
#ifndef POSITIONDATA_H
#define POSITIONDATA_H

#include <QtGlobal>
#include <QFlags>

#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API PositionData final : public TimeVariableData
{
    // Position
    double latitude {0.0};
    double longitude {0.0};
    // GPS altitude
    double altitude {0.0};
    // Indicated pressure altitude (analytical purposes only)
    double indicatedAltitude {0.0};
    double pitch {0.0};
    double bank {0.0};
    double trueHeading {0.0};

    // Velocity
    double velocityBodyX {0.0};
    double velocityBodyY {0.0};
    double velocityBodyZ {0.0};

    explicit PositionData(double latitude = 0.0, double longitude = 0.0, double altitude = 0.0) noexcept;
};

#endif // POSITIONDATA_H
