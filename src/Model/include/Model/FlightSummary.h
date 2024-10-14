/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef FLIGHTSUMMARY_H
#define FLIGHTSUMMARY_H

#include "Kernel/Const.h"
#include <cstdint>

#include <QString>
#include <QDateTime>

#include <Kernel/Const.h>
#include "ModelLib.h"

struct MODEL_API FlightSummary final
{
    std::int64_t flightId {Const::InvalidId};
    QDateTime creationDate;
    QString aircraftType;
    QString flightNumber;
    std::size_t aircraftCount {0};
    QDateTime startSimulationLocalTime;
    QDateTime startSimulationZuluTime;
    QDateTime endSimulationLocalTime;
    QDateTime endSimulationZuluTime;
    QString startLocation;
    QString endLocation;
    QString title;

    FlightSummary() noexcept;
};

#endif // FLIGHTSUMMARY_H
