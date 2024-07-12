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
#include <QString>
#include <QStringBuilder>

class QDateTime;
class QDate;
class QTime;

#include "KernelLib.h"

struct UnitPrivate;

/*!
 * Formats values having either a unit like month, week, seconds or hour or units
 * having a special format (convention) such as DMS or decimal coordinates.
 *
 * The returned QStrings are represented according to the current locale which
 * determines the thousand delimiter and decimal point vs comma. Those strings
 * are hence meant to be displayed in the user interface.
 *
 * For exporting values use "raw number conversion" functions such as QString::number()
 * instead.
 */
class KERNEL_API Unit final
{
public:
    static constexpr std::int64_t MillisecondsPerSecond = 1000;
    static constexpr std::int64_t SecondsPerMinute = 60;
    static constexpr std::int64_t MinutesPerHour = 60;
    static constexpr std::int64_t HoursPerDay = 24;
    static constexpr std::int64_t DaysPerWeek = 7;
    // Approximation
    static constexpr std::int64_t DaysPerMonth = 30;
    static constexpr std::int64_t DaysPerYear = 365;
    static constexpr std::int64_t MonthPerYear = 12;

    static constexpr std::int64_t MillisecondsPerMinute {SecondsPerMinute * MillisecondsPerSecond};
    static constexpr std::int64_t MillisecondsPerHour {MinutesPerHour * MillisecondsPerMinute};
    static constexpr std::int64_t MillisecondsPerDay {HoursPerDay * MillisecondsPerHour};
    static constexpr std::int64_t MillisecondsPerWeek {DaysPerWeek * MillisecondsPerDay};
    static constexpr std::int64_t MillisecondsPerMonth {DaysPerMonth * MillisecondsPerDay};
    static constexpr std::int64_t MillisecondsPerYear {DaysPerYear * MillisecondsPerDay};


    // Precision of exported double GNSS coordinate values
    // https://rapidlasso.com/2019/05/06/how-many-decimal-digits-for-storing-longitude-latitude/
    // https://xkcd.com/2170/
    static constexpr int CoordinatePrecision = 6;

    enum struct Name
    {
        Second,
        Feet,
        Knot
    };

    Unit();
    Unit(const Unit &rhs) = delete;
    Unit(Unit &&rhs) noexcept;
    Unit &operator=(const Unit &rhs) = delete;
    Unit &operator=(Unit &&rhs) noexcept;
    ~Unit();

    /*!
     * Formats the \p latitude into degrees, minutes and seconds (DMS).
     *
     * \param latitude
     *        the latitude to be converted
     * \return the \p latitude in DMS format
     * \sa formatCoordinate
     */
    static QString formatLatitudeDMS(double latitude) noexcept;

    /*!
     * Formats the \p longitude into degrees, minutes and seconds (DMS).
     *
     * \param longitude
     *        the longitude to be converted
     * \return the \p longitude in DMS format
     * \sa formatCoordinate
     */
    static QString formatLongitudeDMS(double longitude) noexcept;

    /*!
     * Formats the \p latitude and \p longitude into degrees, minutes and seconds (DMS).
     *
     * \param latitude
     *        the latitude to be converted
     * \param longitude
     *        the longitude to be converted
     * \return the \p latitude and \p longitude in DMS format
     * \sa formatCoordinates
     */
    static QString formatLatLongPositionDMS(double latitude, double longitude) noexcept;

    QString formatFeet(double feet) const noexcept;
    QString formatCelcius(double temperature) const noexcept;
    QString formatPressureInHPa(double pressure) const noexcept;
    QString formatVisibility(double meters) const noexcept;

    /*!
     * Returns a formatted string for \p degrees [0, 360].
     *
     * \return a formatted string for \p degrees, including unit (°)
     */
    QString formatDegrees(double degrees) const noexcept;
    QString formatHz(double hz) const noexcept;

    QString formatSpeedInFeetPerSecond(double speed) const noexcept;
    QString formatSpeedInRadians(double speed) const noexcept;

    QString formatPosition(std::int16_t position) const noexcept;
    QString formatPercent(std::uint8_t percent) const noexcept;

    QString formatKnots(double speed) const noexcept;

    QString formatMemory(std::size_t memory) const noexcept;

    QString formatDate(const QDate &date) const noexcept;
    QString formatDate(const QDateTime &dateTime) const noexcept;
    QString formatTime(const QTime &time) const noexcept;
    QString formatTime(const QDateTime &dateTime) const noexcept;
    QString formatDateTime(const QDateTime &dateTime) const noexcept;
    QString formatDuration(std::int64_t milliseconds) const noexcept;

    /*!
     * Returns the name of the \p month.
     *
     * \param month
     *        the month of year [1, 12]
     * \return then name of the months
     */
    QString formatMonth(int month) const noexcept;

    /*!
     * Formats the \p number in a locale-specific way.
     *
     * Example: 6'543.21
     *
     * \param number
     *        the number to be formatted
     * \param precision
     *        the number of digits to conisder
     * \return the formatted number, with locale-specific delimiters
     */
    QString formatNumber(double number, int precision) const noexcept;

    /*!
     * Formats the \p number in a locale-specific way.
     *
     * Example: 6'543
     *
     * \param number
     *        the number to be formatted
     * \return the formatted number, with locale-specific delimiters
     */
    QString formatNumber(std::int64_t number) const noexcept;

    double toNumber(const QString &value, bool *ok = nullptr) const noexcept;

    /*!
     * Formats the \p seconds, with local thousands separator and unit.
     *
     * \param seconds
     *        the seconds to format
     * \return the formatted seconds
     */
    QString formatSeconds(double seconds) const noexcept;

    /*!
     * Formats the \p milliseconds (timestamp).
     *
     * \param milliseconds
     *        the milliseconds [milliseconds]
     * \param startDate
     *        the start date and time of the recording
     * \return the number formatted \p milliseconds timestamp
     */
    QString formatTimestamp(std::int64_t milliseconds, const QDateTime &startDate) const noexcept;

    /*!
     * Formats the elapsed \p milliseconds (timestamp) as either (fractional) milliseconds, seconds,
     * minutes or hours.
     *
     * Example: 5.43 minutes
     *
     * \param milliseconds
     *        the ellapsed time [milliseconds]
     * \return the elapsed time formatted \p milliseconds
     */
    QString formatElapsedTime(std::int64_t milliseconds) const noexcept;

    /*!
     * Formats the \p milliseconds as hh:mm:ss timestamp.
     * \param milliseconds
     *        the timestamp [milliseconds]
     * \return the hh:mm:ss formatted \p milliseconds timestamp
     * \sa formatTime
     */
    static QString formatHHMMSS(std::int64_t milliseconds) noexcept;

    /*!
     * Formats the \p time as hh:mm:ss timestamp.
     *
     * Example: 05:43:21
     *
     * \param time
     *        the time
     * \return the hh:mm:ss formatted \p time timestamp
     */
    static QString formatHHMMSS(QTime time) noexcept;

    static QString formatBoolean(bool value) noexcept;

    /*!
     * Formats the GNSS \p coordinate (latitude or longitude) with the appropriate decimal point precision.
     *
     * Note: the coordinate is always formatted with a decimal point, in order to fasciliate exchange
     * with other applications / websites.
     *
     * \param coordinate
     *        the coordinate to be formatted
     * \return the text representation of \p coordinate; always using a decimal point ('.')
     * \sa formatLatitudeDMS
     * \sa formatLongitudeDMS
     */
    static inline QString formatCoordinate(double coordinate) noexcept
    {
        return QString::number(coordinate, 'f', CoordinatePrecision);
    }

    /*!
     * Formats the GNSS \p latitude and \p longitude with the appropriate decimal point precision.
     *
     * \param latitude
     *        the latitude to be formatted
     * \param longitude
     *        the longitude to be formatted
     * \return the text representation of the coordinate, separated with a comma (,)
     * \sa formatLatLongPositionDMS
     */
    static inline QString formatCoordinates(double latitude, double longitude) noexcept
    {
        return formatCoordinate(latitude) % ", " % formatCoordinate(longitude);
    }

private:
    std::unique_ptr<UnitPrivate> d;
};

#endif // UNIT_H
