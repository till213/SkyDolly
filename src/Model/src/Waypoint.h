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
#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QtGlobal>
#include <QString>
#include <QDateTime>

#include "SimType.h"
#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API Waypoint : public TimeVariableData
{
    QString identifier;
    float latitude;
    float longitude;
    float altitude;
    QDateTime localTime;
    QDateTime zuluTime;

    Waypoint() noexcept;
    Waypoint(Waypoint &&) = default;
    Waypoint(const Waypoint &) = default;
    Waypoint &operator= (const Waypoint &) = default;

    /*!
     * Returns whether this waypoint is valid or not.
     *
     * \return \c true if this waypoint is valid (non-empty identifier); \c false else
     */
    bool isValid() const noexcept;

    static const Waypoint NullWaypoint;
};

#endif // WAYPOINT_H
