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
#include <memory>
#include <cmath>
#include <limits>
#include <chrono>
#include <cstdint>

#include <QCoreApplication>
#include <QString>
#include <QStringBuilder>
#include <QLocale>
#include <QDateTime>
#include <QDate>
#include <QTime>

#include "Convert.h"
#include "Unit.h"

namespace {
    constexpr int Precision = 2;
    // https://en.wikipedia.org/wiki/Visibility - Fog, mist, haze, and freezing drizzle
    constexpr int Fog = 1000; // In meters
    constexpr int Mist = 2000; // In meters
    constexpr int Haze = 5000; // In meters
}

class UnitPrivate {
public:
    UnitPrivate()
    {}

    QLocale locale;
};

// PUBLIC

Unit::Unit()
    : d(std::make_unique<UnitPrivate>())
{}

Unit::~Unit()
{}

QString Unit::formatLatitude(double latitude) noexcept
{
    int degrees;
    int minutes;
    double seconds;

    Convert::dd2dms(latitude, degrees, minutes, seconds);

    QString hemisphere = latitude >= 0.0 ? QCoreApplication::translate("Unit", "N") : QCoreApplication::translate("Unit", "S");
    return QString::number(degrees) % "° " % QString::number(minutes) % "' " % QString::number(seconds, 'f', Precision) % "'' " % " " % hemisphere;
}

QString Unit::formatLongitude(double longitude) noexcept
{
    int degrees;
    int minutes = 0;
    double seconds = 0.0;

    Convert::dd2dms(longitude, degrees, minutes, seconds);
    QString hemisphere = longitude >= 0.0 ? QCoreApplication::translate("Unit", "E") : QCoreApplication::translate("Unit", "W");
    return QString::number(degrees) % "° " % QString::number(minutes) % "' " % QString::number(seconds, 'f', Precision) % "'' " % " " % hemisphere;
}

QString Unit::formatLatLongPosition(double latitude, double longitude) noexcept
{
    return formatLatitude(latitude) % " " % formatLongitude(longitude);
}

QString Unit::formatFeet(double feet) noexcept
{
    return d->locale.toString(feet, 'f', Precision) % " ft";
}

QString Unit::formatCelcius(double temperature) noexcept
{
    return d->locale.toString(temperature, 'f', Precision) % " °C";
}

QString Unit::formatPressureInHPa(double pressure) noexcept
{
    return d->locale.toString(pressure, 'f', Precision) % " hPa";
}

QString Unit::formatVisibility(double meters) noexcept
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

QString Unit::formatDegrees(double velocity) noexcept
{
    return d->locale.toString(velocity, 'f', Precision) % " °";
}

QString Unit::formatHz(double hz) noexcept
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

QString Unit::formatVelocityInFeetPerSecond(double velocity) noexcept
{
    return d->locale.toString(velocity, 'f', Precision) % " ft/s";
}

QString Unit::formatVelocityInRadians(double velocity) noexcept
{
    return d->locale.toString(velocity, 'f', Precision) % " rad/s";
}

QString Unit::formatPosition(qint16 position) noexcept
{
    return d->locale.toString(position / static_cast<double>(std::numeric_limits<qint16>::max()) * 100.0, 'f', Precision) % " %";
}

QString Unit::formatPercent(quint8 percent) noexcept
{
    return d->locale.toString(percent / static_cast<double>(std::numeric_limits<quint8>::max()) * 100.0, 'f', Precision) % " %";
}

QString Unit::formatKnots(double velocity) noexcept
{
    return d->locale.toString(velocity, 'f', Precision) % " knots";
}

QString Unit::formatElapsedTime(int64_t milliSeconds) noexcept
{
    QString elapsedTime;
    if (qAbs(milliSeconds) < 1000) {
        elapsedTime = QString("%1 ms").arg(milliSeconds);
    } else if (qAbs(milliSeconds) < 1000 * 60) {
        elapsedTime = QString("%1 s").arg(QString::number(static_cast<double>(milliSeconds) / 1000.0, 'f', 1));
    } else if (qAbs(milliSeconds) < 1000 * 60 * 60) {
        elapsedTime = QString("%1 min").arg(QString::number(static_cast<double>(milliSeconds) / (1000.0 * 60.0), 'f', 1));
    } else {
        elapsedTime = QString("%1 hours").arg(QString::number(static_cast<double>(milliSeconds) / (1000.0 * 60.0 * 60), 'f', 1));
    }

    return elapsedTime;
}

QString Unit::formatMemory(int64_t memory) noexcept
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

QString Unit::formatDate(const QDate &date) noexcept
{
    return d->locale.toString(date, QLocale::ShortFormat);
}

QString Unit::formatDate(const QDateTime &dateTime) noexcept
{
    return formatDate(dateTime.date());
}

QString Unit::formatTime(const QDateTime &dateTime) noexcept
{
    QTime time(dateTime.time());
    return d->locale.toString(time, QLocale::ShortFormat);
}

QString Unit::formatDateTime(const QDateTime &dateTime) noexcept
{
    return d->locale.toString(dateTime, QLocale::ShortFormat);
}

QString Unit::formatDuration(const QTime &time) noexcept
{
    return d->locale.toString(time, "hh:mm:s");
}

QString Unit::formatMonth(int month) noexcept
{
    return d->locale.monthName(month);
}

QString Unit::formatNumber(double number, int precision) noexcept
{
    return d->locale.toString(number, 'f', precision);
}

double Unit::toNumber(const QString &value, bool *ok) noexcept
{
    return d->locale.toDouble(value, ok);
}

QString Unit::formatHHMMSS(int64_t msec) noexcept
{
    std::chrono::milliseconds milliseconds {msec};
    std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(milliseconds);
    milliseconds -= seconds;
    std::chrono::minutes minutes = std::chrono::duration_cast<std::chrono::minutes>(seconds);
    seconds -= minutes;
    std::chrono::hours hours = std::chrono::duration_cast<std::chrono::hours>(minutes);
    minutes -= hours;

    QTime time;
    time.setHMS(hours.count(), minutes.count(), seconds.count());
    return time.toString("hh:mm:ss");
}

QString Unit::formatBoolean(bool value) noexcept
{
    return value ? QCoreApplication::translate("Unit", "Yes") : QCoreApplication::translate("Unit", "No");
}
