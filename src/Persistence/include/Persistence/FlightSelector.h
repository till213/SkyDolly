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
#ifndef FLIGHTSELECTOR_H
#define FLIGHTSELECTOR_H

#include <QDate>
#include <QString>

#include <Model/SimType.h>
#include "PersistenceLib.h"

struct PERSISTENCE_API FlightSelector
{
    QDate fromDate {MinDate};
    QDate toDate {MaxDate};
    QString searchKeyword;
    bool hasFormation {false};
    SimType::EngineType engineType {SimType::EngineType::All};
    int mininumDurationMinutes {0};

    // The first flight in human history: Orville piloted the gasoline-powered,
    // propeller-driven biplane, which stayed aloft for 12 seconds and covered
    // 120 feet on its inaugural flight.
    // [https://www.history.com/this-day-in-history/first-airplane-flies]
    static inline const QDate MinDate {1903, 12, 17};
    static inline const QDate MaxDate {9999, 12, 31};
};

#endif // FLIGHTSELECTOR_H
