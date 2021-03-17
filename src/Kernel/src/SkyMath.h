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
#ifndef SKYMATH_H
#define SKYMATH_H

#include <limits>

#include <QtGlobal>

namespace SkyMath {

    // Make the position range symmetric, such that 0 is exactly the middle value
    constexpr double PositionMin16 = static_cast<double>(-std::numeric_limits<qint16>::max());
    constexpr double PositionMax16 = static_cast<double>( std::numeric_limits<qint16>::max());
    constexpr double PositionRange16 = PositionMax16 - PositionMin16;

    constexpr double PercentMin8 = static_cast<double>(std::numeric_limits<quint8>::min());
    constexpr double PercentMax8 = static_cast<double>(std::numeric_limits<quint8>::max());
    constexpr double PercentRange8 = PercentMax8;

    template <typename T> int sgn(T val) noexcept
    {
        return (T(0) < val) - (val < T(0));
    }

    template <typename T> T normalise180(T y0, T y1) noexcept
    {
        T y1n;
        T s = sgn(y0);
        if (sgn(y1) != s) {
            T diff = qAbs(y1 - y0);
            if (diff > T(180)) {
                y1n = s * (T(360) - qAbs(y1));
            } else {
                y1n = y1;
            }
        } else {
            y1n = y1;
        }

        return y1n;
    }

    /*!
     * Interpolates between \c y1 and \c y2 and the support values y0 and
     * y3 using Hermite (cubic) interpolation.
     *
     *  Tension: 1 is high, 0 normal, -1 is low
     *  Bias: 0 is even,
     *        positive is towards first segment,
     *        negative towards the other
     *
     * Also refer to: http://paulbourke.net/miscellaneous/interpolation/
     */
    template <typename T> T interpolateHermite(
        T y0, T y1, T y2, T y3,
        T mu,
        T tension = T(0),
        T bias = T(0)) noexcept
    {
        T m0, m1, mu2, mu3;
        T a0, a1, a2, a3;

        mu2 = mu * mu;
        mu3 = mu2 * mu;
        m0  = (y1 - y0) * (T(1) + bias) * (T(1) - tension) / T(2);
        m0 += (y2 - y1) * (T(1) - bias) * (T(1) - tension) / T(2);
        m1  = (y2 - y1) * (T(1) + bias) * (T(1) - tension) / T(2);
        m1 += (y3 - y2) * (T(1) - bias) * (T(1) - tension) / T(2);

        a0 =  T(2) * mu3 - T(3) * mu2 + T(1);
        a1 =         mu3 - T(2) * mu2 + mu;
        a2 =         mu3 -        mu2;
        a3 = -T(2) * mu3 + T(3) * mu2;

        return (a0 * y1 + a1 * m0 + a2 * m1 + a3 * y2);
    }

    /*!
     * Interpolates circular values in a range of [-180, 180[ using Hermite
     * (cubic) interpolation.
     *
     * Also refer to \c interpolateHermite180.
     */
    template <typename T> T interpolateHermite180(
        T y0, T y1, T y2, T y3,
        T mu,
        T tension = T(0),
        T bias = T(0)) noexcept
    {
        T y0n, y1n, y2n, y3n;

        // Normalise sample points y0, y1, y2 and y3
        y0n = y0;
        y1n = normalise180(y0, y1);
        y2n = normalise180(y1n, y2);
        y3n = normalise180(y2n, y3);

        T v = interpolateHermite(y0n, y1n, y2n, y3n, mu, tension, bias);
        if (v < - T(180)) {
           v += T(360);
        } else if (v >= T(180)) {
           v -= T(360);
        }
        return v;
    }

    template <typename T> T interpolateHermite360(
        T y0, T y1, T y2, T y3,
        T mu,
        T tension = T(0),
        T bias = T(0)) noexcept
    {
        return interpolateHermite180(y0 - T(180), y1 - T(180), y2 - T(180), y3 - T(180), mu, tension, bias) + T(180);
    }

    template <typename T, typename U> T interpolateLinear(T p1, T p2, U mu) noexcept
    {
        if (std::is_integral<T>::value) {
            return p1 + qRound(mu * (U(p2) - U(p1)));
        } else {
            return p1 + mu * (U(p2) - U(p1));
        }
    }

    /*!
     * Maps the \c position value to a discrete, signed 16bit value.
     *
     * \param position
     *        the position value in the range [-1.0, 1.0]
     * \return the mapped discrete, signed 16bit value [PositionMin16, PositionMax16] (note: symmetric range)
     */
    inline qint16 fromPosition(double position) noexcept
    {
        return static_cast<qint16>(qRound(PositionMin16 + ((position + 1.0) * PositionRange16) / 2.0));
    }

    /*!
     * Maps the \c position16 value to a double value.
     *
     * \param position16
     *        the discrete signed 16bit position value in the range [PositionMin16, PositionMax16] (note: symmetric range)
     * \return the position mapped onto a double value [-1.0, 1.0]
     */
    inline double toPosition(qint16 position16) noexcept
    {
        return 2.0 * (static_cast<double>(position16) - PositionMin16) / PositionRange16 - 1.0;
    }

    /*!
     * Maps the \c percent value to a discrete, unsigned 8bit value.
     *
     * \param percent
     *        the percent value in the range [0.0, 100.0]
     * \return the mapped discrete, unsigned 8bit value [0, PercentMax8]
     */
    inline quint8 fromPercent(double percent) noexcept
    {
        return static_cast<quint8>(qRound(percent * PercentRange8 / 100.0));
    }

    /*!
     * Maps the \c percent8 value to a double value.
     *
     * \param percent8
     *        the discrete unsigned 8bit percent value in the range [0, PercentMax8]
     * \return the percent mapped onto a double value [0, 100.0]
     */
    inline double toPercent(quint8 percent8) noexcept
    {
        return static_cast<double>(100.0 * percent8 / PercentRange8);
    }

} // namespace

#endif // SKYMATH_H
