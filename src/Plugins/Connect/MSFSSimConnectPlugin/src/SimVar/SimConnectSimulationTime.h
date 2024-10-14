/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#ifndef SIMCONNECTSIMULATIONTIME_H
#define SIMCONNECTSIMULATIONTIME_H

#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QTimeZone>

/*!
 * Simulation date and time (local and zulu).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectSimulationTime
{
    std::int32_t localTime {0};
    std::int32_t localYear {0};
    std::int32_t localMonth {0};
    std::int32_t localDay {0};
    std::int32_t zuluTime {0};
    std::int32_t zuluYear {0};
    std::int32_t zuluMonth {0};
    std::int32_t zuluDay {0};

    inline QDateTime toLocalDateTime() const noexcept
    {
        QDateTime dateTime;
        const auto time = QTime::fromMSecsSinceStartOfDay(localTime * 1000);
        dateTime.setTime(time);
        const QDate date = QDate(localYear, localMonth, localDay);
        dateTime.setDate(date);

        return dateTime;
    }

    inline QDateTime toZuluDateTime() const noexcept
    {
        QDateTime dateTime;
        const auto time = QTime::fromMSecsSinceStartOfDay(zuluTime * 1000);
        dateTime.setTime(time);
        const QDate date = QDate(zuluYear, zuluMonth, zuluDay);
        dateTime.setDate(date);
        dateTime.setTimeZone(QTimeZone::UTC);

        return dateTime;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTSIMULATIONTIME_H
