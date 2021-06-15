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
#include <utility>
#include <cmath>

#include <QtGlobal>

/*!
 * Mathematical functions for interpolation.
 */
namespace SkyMath {

    /*! The minimal position value, such that value 0 is exaclty in the middle of the entire range. */
    inline constexpr double PositionMin16 = static_cast<double>(-std::numeric_limits<qint16>::max());
    /*! The maximum position value, such that value 0 is exaclty in the middle of the entire range. */
    inline constexpr double PositionMax16 = static_cast<double>( std::numeric_limits<qint16>::max());
    /*! The range (number of values) for position values. */
    inline constexpr double PositionRange16 = PositionMax16 - PositionMin16;

    /*! The minimal percent value. */
    inline constexpr double PercentMin8 = static_cast<double>(std::numeric_limits<quint8>::min());
    /*! The maximum percent value. */
    inline constexpr double PercentMax8 = static_cast<double>(std::numeric_limits<quint8>::max());
    /*! The range (number of values) for percent values. */
    inline constexpr double PercentRange8 = PercentMax8;

    // Average earth radius [metres]
    inline constexpr double EarthRadius = 6378137.0;

    inline double degreesToRadians(double degree) {
        return degree * M_PI / 180.0;
    };

    inline double radiansToDegrees(double radians) {
        return radians * 180.0 / M_PI;
    };

    inline double feetToMeters(double feet) {
        return feet / 3.2808;
    };

    inline double metersToFeet(double meters) {
        return meters * 3.2808;
    };

    /*!
     * Returns the sign of \c val.
     *
     * \return -1 if \c val is a negative value; 0 for \c val
     *         0; +1 else
     */
    template <typename T>
    int sgn(T val) noexcept
    {
        return (T(0) < val) - (val < T(0));
    }

    /*!
     * Normalises the value \c y1 by comparing its sign with
     * the \em previous value \c y0, which come from a
     * "+/- modulo 180" domain (value in [-180, 180[).
     *
     * - if \c y0 and \c y1 have the same sign then the normalised
     *   value is simply \c y1
     * - if the sign is different then the difference between \c y1 and \c y0
     *   is taken into account:
     *     - if the difference is smaller or equal to 180 the the normalised
     *       value is still \c y1
     *     - if the difference is larger than 180 then \c y1 is "wrapped across
     *       the modulo boundary", by subtracting it from 360 and assigning
     *       the same sign as \c y0
     *
     * Examples:
     * y  | a) | b)  | c)  | d)  | e)
     * ---|----|-----|-----|-----|-----
     * y0 | 10 | 160 | 170 | -20 | -170
     * y1 | 20 | 170 | -20 | -10 |   20
     * yn | 20 | 170 | 340 | -10 | -340
     *
     * Or in other words: the normalisation basically removes the "modulo operation"
     * and the domain is extended beyond the -180 and +180 boundaries. So a series
     * or samples values like 165, 175, -175, -165 becomes 165, 175, 185, 195, and
     * the normalised values are then suitable for interpolation.
     */
    template <typename T>
    T normalise180(T y0, T y1) noexcept
    {
        T y1n;
        T s0 = sgn(y0);
        if (sgn(y1) != s0) {
            T diff = qAbs(y1 - y0);
            if (diff > T(180)) {
                y1n = s0 * (T(360) - qAbs(y1));
            } else {
                y1n = y1;
            }
        } else {
            y1n = y1;
        }

        return y1n;
    }

    /*!
     * Interpolates between \c y1 and \c y2 and the support values \c y0 and
     * \c y3 using Hermite (cubic) interpolation.
     *
     * Also refer to: http://paulbourke.net/miscellaneous/interpolation/
     *
     * \param y0
     *        first support value
     * \param y1
     *        first interpolation value
     * \param y2
     *        second interpolation value
     * \param y3
     *        second support value
     * \param mu
     *        interpolation factor in [0.0, 1.0]
     * \param tension
     *        1 is high, 0 normal, -1 is low
     * \param bias
     *        0 is even; positive values create a bias towards the first segment;
     *        negative values create a bias towards the second segment
     */
    template <typename T>
    T interpolateHermite(
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
     * Also refer to \c #interpolateHermite().
     */
    template <typename T>
    T interpolateHermite180(
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

    /*!
     * Interpolates circular values in a range of [0, 360[ using Hermite
     * (cubic) interpolation.
     *
     * Also refer to \c #interpolateHermite().
     */
    template <typename T>
    T interpolateHermite360(
        T y0, T y1, T y2, T y3,
        T mu,
        T tension = T(0),
        T bias = T(0)) noexcept
    {
        return interpolateHermite180(y0 - T(180), y1 - T(180), y2 - T(180), y3 - T(180), mu, tension, bias) + T(180);
    }

    /*!
     * Interpolates between \c p1 and \c p2 and using linear interpolation.
     *
     * \param p1
     *        the first interpolation point
     * \param p2
     *        the second interpolation point
     * \param mu
     *        the interpolation factor in [0.0, 1.0]
     */
    template <typename T, typename U>
    T interpolateLinear(T p1, T p2, U mu) noexcept
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

    /*!
     * sinphi2 = sinphi1⋅cosδ + cosphi1⋅sinδ⋅costheta
     * tanΔlambda = sintheta⋅sinδ⋅cosphi1 / cosδ−sinphi1⋅sinphi2
     *
     * \sa mathforum.org/library/drmath/view/52049.html for derivation
     * \sa https://www.movable-type.co.uk/scripts/latlong.html
     */
    inline std::pair<double, double> relativePosition(std::pair<double, double> position, double altitude, double bearing, double distance) noexcept
    {
        std::pair<double, double> destination;

        const double radius = EarthRadius + altitude;

        // Angular distance [Radians]
        const double delta = distance / radius;
        const double theta = degreesToRadians(bearing);

        const double phi1 = degreesToRadians(position.first);
        const double lambda1 = degreesToRadians(position.second);

        const double sinphi2 = std::sin(phi1) * std::cos(delta) + std::cos(phi1) * std::sin(delta) * std::cos(theta);
        const double phi2 = std::asin(sinphi2);
        const double y = std::sin(theta) * std::sin(delta) * std::cos(phi1);
        const double x = std::cos(delta) - std::sin(phi1) * sinphi2;
        const double lambda2 = lambda1 + std::atan2(y, x);

        destination.first = radiansToDegrees(phi2);
        destination.second = radiansToDegrees(lambda2);

        return destination;
    }

} // namespace

#endif // SKYMATH_H
