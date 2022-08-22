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

struct UnitPrivate;

class KERNEL_API Unit final
{
public:

    enum struct Name
    {
        Second,
        Feet,
        Knot
    };

    Unit();
    ~Unit();

    /*!
     * Formats the \c latitude into degrees, minutes and seconds (DMS).
     *
     * \param latitude
     *        the latitude to be converted
     * \return the \c latitude in DMS format
     * \sa formatCoordinate
     */
    QString formatLatitudeDMS(double latitude) const noexcept;

    /*!
     * Formats the \c longitude into degrees, minutes and seconds (DMS).
     *
     * \param longitude
     *        the longitude to be converted
     * \return the \c longitude in DMS format
     * \sa formatCoordinate
     */
    QString formatLongitudeDMS(double longitude) const noexcept;

    /*!
     * Formats the \c latitude and \c longitude into degrees, minutes and seconds (DMS).
     *
     * \param latitude
     *        the latitude to be converted
     * \param longitude
     *        the longitude to be converted
     * \return the \c latitude and \c longitude in DMS format
     * \sa formatCoordinates
     */
    QString formatLatLongPositionDMS(double latitude, double longitude) const noexcept;

    QString formatFeet(double feet) const noexcept;
    QString formatCelcius(double temperature) const noexcept;
    QString formatPressureInHPa(double pressure) const noexcept;
    QString formatVisibility(double meters) const noexcept;

    /*!
     * Returns a formatted string for \c degrees [0, 360].
     *
     * \return a formatted string for \c degrees, including unit (°)
     */
    QString formatDegrees(double degrees) const noexcept;
    QString formatHz(double hz) const noexcept;

    QString formatSpeedInFeetPerSecond(double speed) const noexcept;
    QString formatSpeedInRadians(double speed) const noexcept;

    QString formatPosition(std::int16_t position) const noexcept;
    QString formatPercent(std::uint8_t percent) const noexcept;

    QString formatKnots(double speed) const noexcept;

    QString formatMemory(std::int64_t memory) const noexcept;

    QString formatDate(const QDate &date) const noexcept;
    QString formatDate(const QDateTime &dateTime) const noexcept;
    QString formatTime(const QDateTime &dateTime) const noexcept;
    QString formatDateTime(const QDateTime &dateTime) const noexcept;
    QString formatDuration(const QTime &time) const noexcept;

    /*!
     * Returns the name of the month.
     *
     * \param month
     *        the month of year [1, 12]
     * \return then name of the months
     */
    QString formatMonth(int month) const noexcept;

    QString formatNumber(double number, int precision) const noexcept;
    double toNumber(const QString &value, bool *ok = nullptr) const noexcept;

    /*!
     * Formats the \c second, with local thousands separator and unit.
     *
     * \param second
     *        the second to format
     * \return the formatted seconds
     */
    QString formatSeconds(double seconds) const noexcept;

    /*!
     * Formats the \c milliseconds as number, with local thousands separators.
     *
     * Example: 5'432
     *
     * \param milliseconds
     *        the timestamp [milliseconds]
     * \return the number formatted \c milliseconds timestamp
     */
    QString formatTimestamp(std::int64_t milliseconds) const noexcept;

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
    QString formatElapsedTime(std::int64_t milliseconds) const noexcept;

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

    /*!
     * Formats the GNSS \c coordinate (latitude or longitude) with the appropriate decimal point precision.
     *
     * \param coordinate
     *        the coordinate to be formatted
     * \return the text representation of \c coordinate
     * \sa formatLatitude
     * \sa formatLongitude
     */
    static QString formatCoordinate(double coordinate) noexcept;

    /*!
     * Formats the GNSS \c latitude and \c longitude with the appropriate decimal point precision.
     *
     * \param latitude
     *        the latitude to be formatted
     * \param longitude
     *        the longitude to be formatted
     * \return the text representation of the coordinate, separated with a comma (,)
     * \sa formatLatLongPositionDMS
     */
    static QString formatCoordinates(double latitude, double longitude) noexcept;

private:
    const std::unique_ptr<UnitPrivate> d;
};

#endif // UNIT_H
