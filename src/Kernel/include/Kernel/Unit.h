/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#ifndef UNIT_H
#define UNIT_H

#include <memory>
#include <cstdint>

#include <QtGlobal>

class QString;
class QDateTime;
class QDate;
class QTime;

#include "KernelLib.h"

class UnitPrivate;

class KERNEL_API Unit
{
public:
    Unit();
    ~Unit();

    QString formatLatitude(double latitude) noexcept;
    QString formatLongitude(double longitude) noexcept;
    QString formatLatLongPosition(double latitude, double longitude) noexcept;

    QString formatFeet(double feet) noexcept;
    QString formatCelcius(double temperature) noexcept;
    QString formatPressureInHPa(double pressure) noexcept;
    QString formatVisibility(double meters) noexcept;

    /*!
     * Returns a formatted string for \c degrees [0, 360].
     *
     * @return a formatted string for \c degrees, including unit (°)
     */
    QString formatDegrees(double degrees) noexcept;
    QString formatHz(double hz) noexcept;

    QString formatVelocityInFeetPerSecond(double velocity) noexcept;
    QString formatVelocityInRadians(double velocity) noexcept;

    QString formatPosition(std::int16_t position) noexcept;
    QString formatPercent(std::uint8_t percent) noexcept;

    QString formatKnots(double velocity) noexcept;

    QString formatMemory(std::int64_t memory) noexcept;

    QString formatDate(const QDate &date) noexcept;
    QString formatDate(const QDateTime &dateTime) noexcept;
    QString formatTime(const QDateTime &dateTime) noexcept;
    QString formatDateTime(const QDateTime &dateTime) noexcept;
    QString formatDuration(const QTime &time) noexcept;

    /*!
     * Returns the name of the month.
     *
     * \param month
     *        the month of year [1, 12]
     * \return then name of the months
     */
    QString formatMonth(int month) noexcept;

    QString formatNumber(double number, int precision) noexcept;
    double toNumber(const QString &value, bool *ok = nullptr) noexcept;

    /*!
     * Formats the \c milliseconds as number, with local thousands separators.
     *
     * Example: 5'432
     *
     * \param milliseconds
     *        the timestamp [milliseconds]
     * \return the number formatted \c milliseconds timestamp
     */
    QString formatTimestamp(std::int64_t milliseconds) noexcept;

    /*!
     * Formats the elapsed \c milliseconds (timestamp) as either (fractional) milliseconds, seconds,
     * minutes or hours.
     *
     * Example: 5.43 minutes
     *
     * \param milliseconds
     *        the ellapsed time [milliseconds]
     * \return the elapsed time formatted \c milliseconds
     */
    QString formatElapsedTime(std::int64_t milliseconds) noexcept;

    /*!
     * Formats the \c milliseconds as hh:mm:ss timestamp.
     *
     * Example: 05:43:21
     *
     * \param milliseconds
     *        the timestamp [milliseconds]
     * \return the hh:mm:ss formatted \c milliseconds timestamp
     */
    static QString formatHHMMSS(std::int64_t milliseconds) noexcept;

    static QString formatBoolean(bool value) noexcept;

private:
    std::unique_ptr<UnitPrivate> d;
};

#endif // UNIT_H
