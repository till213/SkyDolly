/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <cstdint>
#include <cstdlib>

#include <GeographicLib/DMS.hpp>

#include <QCoreApplication>
#include <QString>
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
}

struct UnitPrivate {
    QLocale locale;

    static constexpr QLatin1Char NumberPadding {'0'};
};

// PUBLIC

Unit::Unit()
    : d {std::make_unique<UnitPrivate>()}
{}

Unit::Unit(Unit &&rhs) noexcept = default;
Unit &Unit::operator=(Unit &&rhs) noexcept = default;
Unit::~Unit() = default;

QString Unit::formatLatitudeDMS(double latitude) noexcept
{
    double degrees {0.0};
    double minutes {0.0};
    double seconds {0.0};

    const QString hemisphere = latitude >= 0.0 ? QCoreApplication::translate("Unit", "N") : QCoreApplication::translate("Unit", "S");
    GeographicLib::DMS::Encode(std::abs(latitude), degrees, minutes, seconds);

    return QString("%1° %2' %3\" %4")
           .arg(static_cast<int>(degrees), 2, 10, UnitPrivate::NumberPadding)
           .arg(static_cast<int>(minutes), 2, 10, UnitPrivate::NumberPadding)
           .arg(seconds, 5, 'f', Precision, UnitPrivate::NumberPadding)
           .arg(hemisphere);
}

QString Unit::formatLongitudeDMS(double longitude) noexcept
{
    double degrees {0.0};
    double minutes {0.0};
    double seconds {0.0};

    const QString hemisphere = longitude >= 0.0 ? QCoreApplication::translate("Unit", "E") : QCoreApplication::translate("Unit", "W");
    GeographicLib::DMS::Encode(std::abs(longitude), degrees, minutes, seconds);
    return QString("%1° %2' %3\" %4")
           .arg(static_cast<int>(degrees), 3, 10, UnitPrivate::NumberPadding)
           .arg(static_cast<int>(minutes), 2, 10, UnitPrivate::NumberPadding)
           .arg(seconds, 5, 'f', Precision, UnitPrivate::NumberPadding)
           .arg(hemisphere);
}

QString Unit::formatLatLongPositionDMS(double latitude, double longitude) noexcept
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

QString Unit::formatMemory(std::size_t memory) const noexcept
{
    QString size;
    if (memory < 1024) {
        size = QString("%1 bytes").arg(memory);
    } else if (memory < 1024ull * 1024ull) {
        size = QString("%1 KiB").arg(QString::number(static_cast<double>(memory) / 1024.0, 'f', 1));
    } else if (memory < 1024ull * 1024ull * 1024ull) {
        size = QString("%1 MiB").arg(QString::number(static_cast<double>(memory) / (1024.0 * 1024.0), 'f', 2));
    } else if (memory < 1024ull * 1024ull * 1024ull * 1024ull) {
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

QString Unit::formatTime(const QTime &time) const noexcept
{
    return d->locale.toString(time, QLocale::ShortFormat);
}

QString Unit::formatTime(const QDateTime &dateTime) const noexcept
{
    return formatTime(dateTime.time());
}

QString Unit::formatDateTime(const QDateTime &dateTime) const noexcept
{
    return d->locale.toString(dateTime, QLocale::ShortFormat);
}

QString Unit::formatDuration(std::int64_t milliseconds) const noexcept
{
    QString formattedDuration;
    if (milliseconds < MillisecondsPerDay) {
        formattedDuration = formatHHMMSS(milliseconds);
    } else {
        formattedDuration = formatElapsedTime(milliseconds);
    }
    return formattedDuration;
}

QString Unit::formatMonth(int month) const noexcept
{
    return d->locale.monthName(month);
}

QString Unit::formatNumber(std::int64_t number) const noexcept
{
    return d->locale.toString(number);
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

QString Unit::formatTimestamp(std::int64_t timestamp, const QDateTime &startDateTime) const noexcept
{
    QString formattedTimestamp;
    if (timestamp < MillisecondsPerDay) {
        formattedTimestamp = formatHHMMSS(timestamp);
    } else {
        const auto dateTime = startDateTime.addMSecs(timestamp);
        formattedTimestamp = QString("%1 %2").arg(formatDate(dateTime.date()), formatHHMMSS(dateTime.time()));
    }
    return formattedTimestamp;
}

QString Unit::formatElapsedTime(std::int64_t milliseconds) const noexcept
{
    QString elapsedTime;
    const auto duration = std::abs(milliseconds);
    if (duration < MillisecondsPerSecond) {
        elapsedTime = QCoreApplication::translate("Unit", "%1 milliseconds", nullptr, static_cast<int>(milliseconds)).arg(milliseconds);
    } else if (duration < MillisecondsPerMinute) {
        const auto seconds = static_cast<double>(milliseconds) / static_cast<double>(MillisecondsPerSecond);
        elapsedTime = QCoreApplication::translate("Unit", "%1 seconds", nullptr, static_cast<int>(seconds)).arg(QString::number(seconds, 'f', 1));
    } else if (duration < MillisecondsPerHour) {
        const auto minutes = static_cast<double>(milliseconds) / static_cast<double>(MillisecondsPerMinute);
        elapsedTime = QCoreApplication::translate("Unit", "%1 minutes", nullptr, static_cast<int>(minutes)).arg(QString::number(minutes, 'f', 1));
    } else if (duration < MillisecondsPerDay) {
        const auto hours = static_cast<double>(milliseconds) / static_cast<double>(MillisecondsPerHour);
        elapsedTime = QCoreApplication::translate("Unit", "%1 hours", nullptr, static_cast<int>(hours)).arg(QString::number(hours, 'f', 1));
    } else if (duration < MillisecondsPerWeek) {
        const auto days = static_cast<double>(milliseconds) / static_cast<double>(MillisecondsPerDay);
        elapsedTime = QCoreApplication::translate("Unit", "%1 days", nullptr, static_cast<int>(days)).arg(QString::number(days, 'f', 1));
    } else if (duration < MillisecondsPerMonth) {
        const auto weeks = static_cast<double>(milliseconds) / static_cast<double>(MillisecondsPerWeek);
        elapsedTime = QCoreApplication::translate("Unit", "%1 weeks", nullptr, static_cast<int>(weeks)).arg(QString::number(weeks, 'f', 1));
    } else if (duration < MillisecondsPerYear) {
        const auto months = static_cast<double>(milliseconds) / static_cast<double>(MillisecondsPerMonth);
        elapsedTime = QCoreApplication::translate("Unit", "%1 months", nullptr, static_cast<int>(months)).arg(QString::number(months, 'f', 1));
    } else {
        const auto years = static_cast<double>(milliseconds) / static_cast<double>(MillisecondsPerYear);
        elapsedTime = QCoreApplication::translate("Unit", "%1 years", nullptr, static_cast<int>(years)).arg(QString::number(years, 'f', 1));
    }

    return elapsedTime;
}

QString Unit::formatHHMMSS(std::int64_t milliseconds) noexcept
{
    auto time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(milliseconds));
    return formatHHMMSS(time);
}

QString Unit::formatHHMMSS(QTime time) noexcept
{
    return time.toString("hh:mm:ss");
}

QString Unit::formatBoolean(bool value) noexcept
{
    return value ? QCoreApplication::translate("Unit", "Yes") : QCoreApplication::translate("Unit", "No");
}
