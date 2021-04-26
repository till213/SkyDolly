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
#include <cmath>
#include <limits>

#include <QString>
#include <QStringBuilder>

#include "Unit.h"

namespace {
    constexpr int Precision = 2;
    // https://en.wikipedia.org/wiki/Visibility - Fog, mist, haze, and freezing drizzle
    constexpr int Fog = 1000; // In metres
    constexpr int Mist = 2000; // In metres
    constexpr int Haze = 5000; // In metres
}

// PUBLIC

QString Unit::formatLatitude(double latitude) noexcept
{
    int degrees;
    int minutes;
    double seconds;

    dd2dms(latitude, degrees, minutes, seconds);

    QString hemisphere = latitude >= 0.0 ? QT_TRANSLATE_NOOP("Unit", "N") : QT_TRANSLATE_NOOP("Unit", "S");
    return QString::number(degrees) % "° " % QString::number(minutes) % "' " % QString::number(seconds, 'f', Precision) % "'' " % " " % hemisphere;
}


QString Unit::formatLongitude(double longitude) noexcept
{
    int degrees;
    int minutes = 0;
    double seconds = 0;

    dd2dms(longitude, degrees, minutes, seconds);
    QString hemisphere = longitude >= 0.0 ? QT_TRANSLATE_NOOP("Unit", "E") : QT_TRANSLATE_NOOP("Unit", "W");
    return QString::number(degrees) % "° " % QString::number(minutes) % "' " % QString::number(seconds, 'f', Precision) % "'' " % " " % hemisphere;
}

QString Unit::formatFeet(double feet) noexcept
{
    return QString::number(feet, 'f', Precision) % " ft";
}

QString Unit::formatCelcius(double temperature) noexcept
{
    return QString::number(temperature, 'f', Precision) % " °C";
}

QString Unit::formatPressureInHPa(double pressure) noexcept
{
    return QString::number(pressure, 'f', Precision) % " hPa";
}

QString Unit::formatVisibility(double metres) noexcept
{
    QString visibility;
    if (metres < Fog) {
        visibility = QT_TRANSLATE_NOOP("Unit", "Fog (< 3,300 ft)");
    } else if (metres < Mist) {
        visibility = QT_TRANSLATE_NOOP("Unit", "Mist (< 1.2 mi)");
    } else if (metres < Haze) {
        visibility = QT_TRANSLATE_NOOP("Unit", "Haze (< 3.1 mi)");
    } else {
        visibility = QT_TRANSLATE_NOOP("Unit", "Clear (>= 3.1 mi)");
    }
    return visibility;
}

QString Unit::formatDegrees(double velocity) noexcept
{
    return QString::number(velocity, 'f', Precision) % " °";
}

QString Unit::formatVelocityInFeet(double velocity) noexcept
{
    return QString::number(velocity, 'f', Precision) % " ft/s";
}

QString Unit::formatVelocityInRadians(double velocity) noexcept
{
    return QString::number(velocity, 'f', Precision) % " rad/s";
}

QString Unit::formatPosition(qint16 position) noexcept
{
    return QString::number(position / static_cast<double>(std::numeric_limits<qint16>::max()) * 100.0, 'f', Precision) % " %";
}

QString Unit::formatPercent(quint8 percent) noexcept
{
    return QString::number(percent / static_cast<double>(std::numeric_limits<quint8>::max()) * 100.0, 'f', Precision) % " %";
}

QString Unit::formatKnots(double velocity) noexcept
{
    return QString::number(velocity, 'f', Precision) % " knots";
}

QString Unit::formatMemory(qint64 memory) noexcept
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

// PRIVATE

// https://www.omnicalculator.com/conversion/coordinates-converter#how-to-convert-latitude-and-longitude-coordinates
void Unit::dd2dms(double dd, int &degrees, int &minutes, double &seconds) noexcept
{
    double absDegrees = std::abs(dd);
    degrees = static_cast<int>(absDegrees);
    double v = (absDegrees - static_cast<double>(degrees)) * 60.0;
    minutes = static_cast<int>(v);
    seconds = (v - static_cast<double>(minutes)) * 60.0;
}
