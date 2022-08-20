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
#include <memory>
#include <cmath>
#include <limits>
#include <chrono>
#include <cstdint>

#include <GeographicLib/DMS.hpp>

#include <QCoreApplication>
#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>
#include <QLocale>
#include <QDateTime>
#include <QDate>
#include <QTime>

#include "Unit.h"

namespace {
    constexpr int Precision = 2;
    // https://en.wikipedia.org/wiki/Visibility - Fog, mist, haze, and freezing drizzle
    constexpr int Fog = 1000; // In meters
    constexpr int Mist = 2000; // In meters
    constexpr int Haze = 5000; // In meters

    constexpr int MillisecondsPerSecond = 1000;
    constexpr int SecondsPerMinute = 60;
    constexpr int MinutesPerHour = 60;

    // Precision of exported double GNSS coordinate values
    // https://rapidlasso.com/2019/05/06/how-many-decimal-digits-for-storing-longitude-latitude/
    // https://xkcd.com/2170/
    constexpr int CoordinatePrecision = 6;
}

struct UnitPrivate {
public:
    UnitPrivate()
    {}

    QLocale locale;

    static inline QLatin1Char NumberPadding {QLatin1Char('0')};
};

// PUBLIC

Unit::Unit()
    : d(std::make_unique<UnitPrivate>())
{}

Unit::~Unit()
{}

QString Unit::formatLatitudeDMS(double latitude) const noexcept
{
    double degrees {0.0};
    double minutes {0.0};
    double seconds {0.0};

    const QString hemisphere = latitude >= 0.0 ? QCoreApplication::translate("Unit", "N") : QCoreApplication::translate("Unit", "S");
    GeographicLib::DMS::Encode(std::abs(latitude), degrees, minutes, seconds);

    return QStringLiteral("%1° %2' %3\" %4")
            .arg(static_cast<int>(degrees), 2, 10, UnitPrivate::NumberPadding)
            .arg(static_cast<int>(minutes), 2, 10, UnitPrivate::NumberPadding)
            .arg(seconds, 5, 'f', Precision, UnitPrivate::NumberPadding)
            .arg(hemisphere);
}

QString Unit::formatLongitudeDMS(double longitude) const noexcept
{
    double degrees {0.0};
    double minutes {0.0};
    double seconds {0.0};

    const QString hemisphere = longitude >= 0.0 ? QCoreApplication::translate("Unit", "E") : QCoreApplication::translate("Unit", "W");
    GeographicLib::DMS::Encode(std::abs(longitude), degrees, minutes, seconds);
    return QStringLiteral("%1° %2' %3\" %4")
            .arg(static_cast<int>(degrees), 3, 10, UnitPrivate::NumberPadding)
            .arg(static_cast<int>(minutes), 2, 10, UnitPrivate::NumberPadding)
            .arg(seconds, 5, 'f', Precision, UnitPrivate::NumberPadding)
            .arg(hemisphere);
}

QString Unit::formatLatLongPositionDMS(double latitude, double longitude) const noexcept
{
    return formatLatitudeDMS(latitude) % " " % formatLongitudeDMS(longitude);
}

QString Unit::formatFeet(double feet) const noexcept
{
    return d->locale.toString(feet, 'f', Precision) % " ft";
}

QString Unit::formatCelcius(double temperature) const noexcept
{
    return d->locale.toString(temperature, 'f', Precision) % " °C";
}

QString Unit::formatPressureInHPa(double pressure) const noexcept
{
    return d->locale.toString(pressure, 'f', Precision) % " hPa";
}

QString Unit::formatVisibility(double meters) const noexcept
{
    QString visibility;
    if (meters < Fog) {
        visibility = QCoreApplication::translate("Unit", "Fog (< 3,300 ft)");
    } else if (meters < Mist) {
        visibility = QCoreApplication::translate("Unit", "Mist (< 1.2 mi)");
    } else if (meters < Haze) {
        visibility = QCoreApplication::translate("Unit", "Haze (< 3.1 mi)");
    } else {
        visibility = QCoreApplication::translate("Unit", "Clear (>= 3.1 mi)");
    }
    return visibility;
}

QString Unit::formatDegrees(double degrees) const noexcept
{
    return d->locale.toString(degrees, 'f', Precision) % "°";
}

QString Unit::formatHz(double hz) const noexcept
{
    QString hzString;
    if (hz < 1000) {
        hzString = QString("%1 Hz").arg(QString::number(hz, 'f', 1));
    }  else if (hz < 1000 * 1000) {
        hzString = QString("%1 kHz").arg(QString::number(hz / 1000.0, 'f', 1));
    }  else {
        hzString = QString("%1 MHz").arg(QString::number(hz / (1000.0 * 1000), 'f', 2));
    }
    return hzString;
}

QString Unit::formatSpeedInFeetPerSecond(double speed) const noexcept
{
    return d->locale.toString(speed, 'f', Precision) % " ft/s";
}

QString Unit::formatSpeedInRadians(double speed) const noexcept
{
    return d->locale.toString(speed, 'f', Precision) % " rad/s";
}

QString Unit::formatPosition(std::int16_t position) const noexcept
{
    return d->locale.toString(position / static_cast<double>(std::numeric_limits<std::int16_t>::max()) * 100.0, 'f', Precision) % " %";
}

QString Unit::formatPercent(std::uint8_t percent) const noexcept
{
    return d->locale.toString(percent / static_cast<double>(std::numeric_limits<std::uint8_t>::max()) * 100.0, 'f', Precision) % " %";
}

QString Unit::formatKnots(double speed) const noexcept
{
    return d->locale.toString(speed, 'f', Precision) % " knots";
}

QString Unit::formatMemory(std::int64_t memory) const noexcept
{
    QString size;
    if (memory < 1024) {
        size = QString("%1 bytes").arg(memory);
    } else if (memory < 1024 * 1024) {
        size = QString("%1 KiB").arg(QString::number(static_cast<double>(memory) / 1024.0, 'f', 1));
    } else if (memory < 1024 * 1024 * 1024) {
        size = QString("%1 MiB").arg(QString::number(static_cast<double>(memory) / (1024.0 * 1024.0), 'f', 2));
    } else if (memory < 1024ll * 1024ll * 1024ll * 1024ll) {
        size = QString("%1 GiB").arg(QString::number(static_cast<double>(memory) / (1024.0 * 1024.0 * 1024.0), 'f', 2));
    } else {
        size = QString("%1 TiB").arg(QString::number(static_cast<double>(memory) / (1024.0 * 1024.0 * 1024.0 * 1024.0), 'f', 2));
    }
    return size;
}

QString Unit::formatDate(const QDate &date) const noexcept
{
    return d->locale.toString(date, QLocale::ShortFormat);
}

QString Unit::formatDate(const QDateTime &dateTime) const noexcept
{
    return formatDate(dateTime.date());
}

QString Unit::formatTime(const QDateTime &dateTime) const noexcept
{
    QTime time(dateTime.time());
    return d->locale.toString(time, QLocale::ShortFormat);
}

QString Unit::formatDateTime(const QDateTime &dateTime) const noexcept
{
    return d->locale.toString(dateTime, QLocale::ShortFormat);
}

QString Unit::formatDuration(const QTime &time) const noexcept
{
    return d->locale.toString(time, "HH:mm:ss");
}

QString Unit::formatMonth(int month) const noexcept
{
    return d->locale.monthName(month);
}

QString Unit::formatNumber(double number, int precision) const noexcept
{
    return d->locale.toString(number, 'f', precision);
}

double Unit::toNumber(const QString &value, bool *ok) const noexcept
{
    return d->locale.toDouble(value, ok);
}

QString Unit::formatSeconds(double seconds) const noexcept
{
    return QCoreApplication::translate("Unit", "%1 seconds", nullptr, static_cast<int>(seconds)).arg(formatNumber(seconds, 2));
}

QString Unit::formatTimestamp(std::int64_t milliseconds) const noexcept
{
    return d->locale.toString(milliseconds);
}

QString Unit::formatElapsedTime(std::int64_t milliSeconds) const noexcept
{
    QString elapsedTime;
    if (qAbs(milliSeconds) < ::MillisecondsPerSecond) {
        elapsedTime = QCoreApplication::translate("Unit", "%1 milliseconds", nullptr, static_cast<int>(milliSeconds)).arg(milliSeconds);
    } else if (qAbs(milliSeconds) < ::MillisecondsPerSecond * ::SecondsPerMinute) {
        const double seconds = static_cast<double>(milliSeconds) / static_cast<double>(::MillisecondsPerSecond);
        elapsedTime = QCoreApplication::translate("Unit", "%1 seconds", nullptr, static_cast<int>(seconds)).arg(QString::number(seconds, 'f', 1));
    } else if (qAbs(milliSeconds) < ::MillisecondsPerSecond * ::SecondsPerMinute * ::MinutesPerHour) {
        const double minutes = static_cast<double>(milliSeconds) / static_cast<double>(::MillisecondsPerSecond * ::SecondsPerMinute);
        elapsedTime = QCoreApplication::translate("Unit", "%1 minutes", nullptr, static_cast<int>(minutes)).arg(QString::number(minutes, 'f', 1));
    } else {
        const double hours = static_cast<double>(milliSeconds) / static_cast<double>(::MillisecondsPerSecond * ::SecondsPerMinute * ::MinutesPerHour);
        elapsedTime = QCoreApplication::translate("Unit", "%1 hours", nullptr, static_cast<int>(hours)).arg(QString::number(hours, 'f', 1));
    }

    return elapsedTime;
}

QString Unit::formatHHMMSS(std::int64_t milliSeconds) noexcept
{
    std::chrono::milliseconds msecs {milliSeconds};
    std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(msecs);
    msecs -= seconds;
    std::chrono::minutes minutes = std::chrono::duration_cast<std::chrono::minutes>(seconds);
    seconds -= minutes;
    std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(minutes);
    minutes -= hours;

    QTime time(static_cast<int>(hours.count()), static_cast<int>(minutes.count()), static_cast<int>(seconds.count()));
    return time.toString("hh:mm:ss");
}

QString Unit::formatBoolean(bool value) noexcept
{
    return value ? QCoreApplication::translate("Unit", "Yes") : QCoreApplication::translate("Unit", "No");
}

QString Unit::formatCoordinate(double coordinate) noexcept
{
    return QString::number(coordinate, 'f', ::CoordinatePrecision);
}

QString Unit::formatCoordinates(double latitude, double longitude) noexcept
{
    return formatCoordinate(latitude) % ", " % formatCoordinate(longitude);
}
