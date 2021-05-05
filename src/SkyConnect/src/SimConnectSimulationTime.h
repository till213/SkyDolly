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
#ifndef SIMCONNECTSIMULATIONTIME_H
#define SIMCONNECTSIMULATIONTIME_H

#include <windows.h>
#include <SimConnect.h>

#include <QtGlobal>
#include <QDateTime>
#include <QDate>
#include <QTime>

/*!
 * Simulation date and time (local and zulu).
 *
 * Implementation note: this struct needs to be packed.
 */
#pragma pack(push, 1)
struct SimConnectSimulationTime
{
    qint32 localTime;
    qint32 localYear;
    qint32 localMonth;
    qint32 localDay;
    qint32 zuluTime;
    qint32 zuluYear;
    qint32 zuluMonth;
    qint32 zuluDay;

    inline QDateTime toLocalDateTime() const noexcept
    {
        QDateTime dateTime;
        QTime time = QTime(0, 0).addSecs(localTime);
        dateTime.setTime(time);
        QDate date = QDate(localYear, localMonth, localDay);
        dateTime.setDate(date);

        return dateTime;
    }

    inline QDateTime toZuluDateTime() const noexcept
    {
        QDateTime dateTime;
        QTime time = QTime(0, 0).addSecs(zuluTime);
        dateTime.setTime(time);
        QDate date = QDate(zuluYear, zuluMonth, zuluDay);
        dateTime.setDate(date);

        return dateTime;
    }

    static void addToDataDefinition(HANDLE simConnectHandle) noexcept;
};
#pragma pack(pop)

#endif // SIMCONNECTSIMULATIONTIME_H
