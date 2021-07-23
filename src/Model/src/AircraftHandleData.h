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
#ifndef AIRCRAFTHANDLEDATA_H
#define AIRCRAFTHANDLEDATA_H

#include <QtGlobal>
#include <QFlags>
#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API AircraftHandleData : public TimeVariableData
{
    qint16 brakeLeftPosition;
    qint16 brakeRightPosition;
    // Implementation note: the water rudder can also have negative (-100.0) values,
    // hence hence the type qint16 (position) which also supports negative values
    qint16 waterRudderHandlePosition;
    quint8 tailhookPosition;
    quint8 canopyOpen;
    quint8 leftWingFolding;
    quint8 rightWingFolding;
    bool gearHandlePosition;
    bool smokeEnabled;

    AircraftHandleData() noexcept;
    AircraftHandleData(AircraftHandleData &&) = default;
    AircraftHandleData(const AircraftHandleData &) = default;
    AircraftHandleData &operator= (const AircraftHandleData &) = default;

    static const AircraftHandleData NullData;
};

#endif // AIRCRAFTHANDLEDATA_H

