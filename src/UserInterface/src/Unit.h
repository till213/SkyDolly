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
#ifndef UNIT_H
#define UNIT_H

#include <memory>

#include <QString>

class UnitPrivate;

class Unit
{
public:
    Unit();
    ~Unit();

    QString formatLatitude(double latitude) noexcept;
    QString formatLongitude(double longitude) noexcept;
    QString formatFeet(double feet) noexcept;
    QString formatCelcius(double temperature) noexcept;
    QString formatPressureInHPa(double pressure) noexcept;
    QString formatVisibility(double metres) noexcept;

    /*!
     * Returns a formatted string for \c degrees [0, 360].
     *
     * @return a formatted string for \c degrees, including unit (°)
     */
    QString formatDegrees(double degrees) noexcept;

    QString formatVelocityInFeet(double velocity) noexcept;
    QString formatVelocityInRadians(double velocity) noexcept;

    QString formatPosition(qint16 position) noexcept;
    QString formatPercent(quint8 percent) noexcept;

    QString formatKnots(double velocity) noexcept;

private:
    std::unique_ptr<UnitPrivate> d;
    static inline void dd2dms(double dd, int &degrees, int &minutes, double &seconds) noexcept;
};

#endif // UNIT_H
