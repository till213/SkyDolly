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
#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QtGlobal>
#include <QString>
#include <QDateTime>

#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API Waypoint final : public TimeVariableData
{
    QString identifier;
    QDateTime localTime;
    QDateTime zuluTime;
    float latitude {0.0f};
    float longitude {0.0f};
    float altitude {0.0f};

    explicit Waypoint(float latitude = 0.0f, float longitude = 0.0f, float altitude = 0.0f) noexcept;

    /*!
     * Returns whether this waypoint is valid or not.
     *
     * \return \c true if this waypoint is valid (non-empty identifier); \c false else
     */
    bool isValid() const noexcept;

    // Note: declaring the constant as "inline" (with initialisation in the header)
    // apparently won't make the Waypoint accessible from outside this library (DLL)
    static const Waypoint NullWaypoint;
    static constexpr const char *CustomDepartureIdentifier {"CUSTD"};
    static constexpr const char *CustomArrivalIdentifier {"CUSTA"};
};

#endif // WAYPOINT_H
