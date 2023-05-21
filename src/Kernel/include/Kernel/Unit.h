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
     * Formats the \c latitude into degrees, minutes and seconds (DMS).
     *
     * \param latitude
     *        the latitude to be converted
     * \return the \c latitude in DMS format
     * \sa formatCoordinate
     */
    static QString formatLatitudeDMS(double latitude) noexcept;

    /*!
     * Formats the \c longitude into degrees, minutes and seconds (DMS).
     *
     * \param longitude
     *        the longitude to be converted
     * \return the \c longitude in DMS format
     * \sa formatCoordinate
     */
    static QString formatLongitudeDMS(double longitude) noexcept;

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
    static QString formatLatLongPositionDMS(double latitude, double longitude) noexcept;

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

    QString formatMemory(std::size_t memory) const noexcept;

    QString formatDate(const QDate &date) const noexcept;
    QString formatDate(const QDateTime &dateTime) const noexcept;
    QString formatTime(const QTime &time) const noexcept;
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
     * \param seconds
     *        the seconds to format
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
     * Note: the coordinate is always formatted with a decimal point, in order to fasciliate exchange
     * with other applications / websites.
     *
     * \param coordinate
     *        the coordinate to be formatted
     * \return the text representation of \c coordinate; always using a decimal point ('.')
     * \sa formatLatitudeDMS
     * \sa formatLongitudeDMS
     */
    static inline QString formatCoordinate(double coordinate) noexcept
    {
        return QString::number(coordinate, 'f', CoordinatePrecision);
    }

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
    static inline QString formatCoordinates(double latitude, double longitude) noexcept
    {
        return formatCoordinate(latitude) % ", " % formatCoordinate(longitude);
    }

private:
    std::unique_ptr<UnitPrivate> d;
};

#endif // UNIT_H
