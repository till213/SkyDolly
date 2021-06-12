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
#include <QFlags>

#include "SimType.h"
#include "PositionData.h"

// PUBLIC

PositionData::PositionData(double latitude, double longitude, double altitude) noexcept
    : TimeVariableData(),
      pitch(0.0),
      bank(0.0),
      heading(0.0),
      velocityBodyX(0.0),
      velocityBodyY(0.0),
      velocityBodyZ(0.0),
      rotationVelocityBodyX(0.0),
      rotationVelocityBodyY(0.0),
      rotationVelocityBodyZ(0.0)
{
    this->latitude = latitude;
    this->longitude = longitude;
    this->altitude = altitude;
}

const PositionData PositionData::NullData = PositionData(0.0, 0.0, 0.0);
