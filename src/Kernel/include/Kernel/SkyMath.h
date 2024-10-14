/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

#include <algorithm>
#include <limits>
#include <utility>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <cassert>

#include <GeographicLib/Geodesic.hpp>

#include <QtGlobal>
#include <QDateTime>
#include <QTimeZone>
#ifdef DEBUG
#include <QDebug>
#endif

#include "Convert.h"

/*!
 * Mathematical functions for interpolation and geodesic math.
 *
 * Useful links:
 * - https://tools.timodenk.com/cubic-spline-interpolation
 * - https://www.wikihow.com/Write-Latitude-and-Longitude
 */
namespace SkyMath
{
    // Latitude, longitude [degrees]
    using Coordinate = std::pair<double, double>;

    /*! The minimal position value, such that value 0 is exaclty in the middle of the entire range. */
    constexpr double PositionMin16 = static_cast<double>(-std::numeric_limits<std::int16_t>::max());
    /*! The maximum position value, such that value 0 is exaclty in the middle of the entire range. */
    constexpr double PositionMax16 = static_cast<double>( std::numeric_limits<std::int16_t>::max());
    /*! The range (number of values) for position values. */
    constexpr double PositionRange16 = PositionMax16 - PositionMin16;

    /*! The minimal percent value. */
    constexpr double PercentMin8 = static_cast<double>(std::numeric_limits<std::uint8_t>::min());
    /*! The maximum percent value. */
    constexpr double PercentMax8 = static_cast<double>(std::numeric_limits<std::uint8_t>::max());
    /*! The range (number of values) for percent values. */
    constexpr double PercentRange8 = PercentMax8;

    /*! Average earth radius [meters] */
    constexpr double EarthRadius = 6378137.0;

    /*! Default threshold beyond with two coordinates are considered to be different [meters] */
    constexpr double DefaultDistanceThreshold = 50.0;

    /*!
     * Defines how the aircraft time offset is to be synchronised.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct TimeOffsetSync: std::uint8_t {
        First = 0,
        /*! No synchronisation to be done. */
        None = First,
        /*! Both date and time of the flight creation time are taken into account */
        DateAndTime,
        /*!
         * Only the time is taken into account. For example a flight that was recorded
         * a day before, on the 2023-02-14 10:45:00Z is only considered to be 15 minutes
         * behind of a flight recorded on the 2023-02-15 11:00:00Z (and not a day plus
         * 15 minutes). This is useful when importing e.g. real-world flights that
         * happened on different days, but should still be synchronised "on the same day".
         */
        TimeOnly,
        Last = TimeOnly
    };

    /*!
     * Returns the sign of \p val.
     *
     * \return -1 if \p val is a negative value; 0 for \p val
     *         0; +1 else
     */
    template <typename T>
    inline int constexpr sgn(T val) noexcept
    {
        return (T(0) < val) - (val < T(0));
    }

    /*!
     * Normalises the value \p y1 by comparing its sign with
     * the \e previous value \p y0, which come from a
     * "+/- modulo 180" domain (value in [-180, 180[).
     *
     * - if \p y0 and \p y1 have the same sign then the normalised
     *   value is simply \p y1
     * - if the sign is different then the difference between \p y1 and \p y0
     *   is taken into account:
     *     - if the difference is smaller or equal to 180 the the normalised
     *       value is still \p y1
     *     - if the difference is larger than 180 then \p y1 is "wrapped across
     *       the modulo boundary", by subtracting it from 360 and assigning
     *       the same sign as \p y0
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
    inline T constexpr normalise180(T y0, T y1) noexcept
    {
        T y1n;
        T s0 = sgn(y0);
        if (sgn(y1) != s0) {
            T diff = std::abs(y1 - y0);
            if (diff > T(180)) {
                y1n = s0 * (T(360) - std::abs(y1));
            } else {
                y1n = y1;
            }
        } else {
            y1n = y1;
        }

        return y1n;
    }

    /*!
     * Interpolates between \p y1 and \p y2 and the support values \p y0 and
     * \p y3 using Hermite (cubic) interpolation.
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
    constexpr T interpolateHermite(
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

    template <typename T>
    constexpr T interpolateCatmullRom(
        T y0, T y1, T y2, T y3,
        T mu) noexcept
    {
        T a0, a1, a2, a3, mu2;

        mu2 = mu*mu;
        a0 = -0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3;
        a1 = y0 - 2.5 * y1 + 2 * y2 - 0.5 * y3;
        a2 = -0.5 * y0 + 0.5 * y2;
        a3 = y1;

        return (a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
    }

    /*!
     * Interpolates circular values in a range of [-180, 180[ using Hermite
     * (cubic) interpolation.
     *
     * \sa #interpolateHermite
     */
    template <typename T>
    constexpr T interpolateHermite180(
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
     * \sa #interpolateHermite
     */
    template <typename T>
    constexpr T interpolateHermite360(
        T y0, T y1, T y2, T y3,
        T mu,
        T tension = T(0),
        T bias = T(0)) noexcept
    {
        return interpolateHermite180(y0 - T(180), y1 - T(180), y2 - T(180), y3 - T(180), mu, tension, bias) + T(180);
    }

    /*!
     * Interpolates between \p p1 and \p p2 using linear interpolation.
     *
     * \param p1
     *        the first interpolation point
     * \param p2
     *        the second interpolation point
     * \param mu
     *        the interpolation factor in [0.0, 1.0]
     */
    template <typename T, typename U>
    constexpr T interpolateLinear(T p1, T p2, U mu) noexcept
    {
        if (std::is_integral<T>::value) {
            return p1 + std::round(mu * (U(p2) - U(p1)));
        } else {
            return p1 + mu * (U(p2) - U(p1));
        }
    }

    /*!
     * Maps the \p position value to a discrete, signed 16bit value.
     *
     * \param position
     *        the normalised position value in the range [-1.0, 1.0]; the range is enforced (clamp)
     * \return the mapped discrete, signed 16bit value [PositionMin16, PositionMax16] (note: symmetric range)
     */
    inline std::int16_t fromNormalisedPosition(double position) noexcept
    {
        return static_cast<std::int16_t>(std::round(PositionMin16 + ((std::clamp(position, -1.0, 1.0) + 1.0) * PositionRange16) / 2.0));
    }

    /*!
     * Maps the \p position16 value to a normalised double value.
     *
     * \param position16
     *        the discrete signed 16bit position value in the range [PositionMin16, PositionMax16] (note: symmetric range)
     * \return the position mapped onto a normalised double value [-1.0, 1.0]
     */
    constexpr double toNormalisedPosition(std::int16_t position16) noexcept
    {
        return 2.0 * (static_cast<double>(position16) - PositionMin16) / PositionRange16 - 1.0;
    }

    /*!
     * Maps the \p percent value to a discrete, unsigned 8bit value.
     *
     * \param percent
     *        the percent value in the range [0.0, 100.0]; the range is enforced (clamp)
     * \return the mapped discrete, unsigned 8bit value [0, PercentMax8]
     */
    inline std::uint8_t fromPercent(double percent) noexcept
    {
        return static_cast<std::uint8_t>(std::round(std::clamp(percent, 0.0, 100.0) * PercentRange8 / 100.0));
    }

    /*!
     * Maps the \p percent8 value to a double value.
     *
     * \param percent8
     *        the discrete unsigned 8bit percent value in the range [0, PercentMax8]
     * \return the percent mapped onto a double value [0, 100.0]
     */
    constexpr double toPercent(std::uint8_t percent8) noexcept
    {
        return static_cast<double>(100.0 * percent8 / PercentRange8);
    }

    /*!
     * Calculates the geodesic distance [meters] of two points.
     *
     * \param startPosition
     *        the Coordinate of the start position [degrees]
     * \param endPosition
     *        the Coordinate of the end position [degrees]
     * \return the spherical distance between the points [meters]
     */
    inline double geodesicDistance(Coordinate startPosition, Coordinate endPosition) noexcept
    {
        double distance {0.0};
        try {
            const GeographicLib::Geodesic &geodesic = GeographicLib::Geodesic::WGS84();
            geodesic.Inverse(startPosition.first, startPosition.second, endPosition.first, endPosition.second, distance);
        } catch (const std::exception &ex) {
#ifdef DEBUG
            qDebug() << "SkyMath::sphericalDistance: caught exception:" << ex.what();
#endif
            distance = std::numeric_limits<double>::max();
        }
        return distance;
    }

    /*!
     * Calculates the geodesic distance between the \p startPoint and the \p endPoint and the
     * speed [meters per second] it takes to travel that distance, taking the timestamps \p startTimestamp
     * and \p endTimestamp into account.
     *
     * \param startPosition
     *        the Coordinate of the start position [degrees]
     * \param startTimestamp
     *        the timestamp of the start point [milliseconds]
     * \param endPosition
     *        the Coordinate of the end position [degrees]
     * \param endTimestamp
     *        the timestamp of the end point [milliseconds]
     * \return the distance (first value) and required speed [m/s] (second value)
     * \sa sphericalDistance
     */
    inline std::pair<double, double> distanceAndSpeed(Coordinate startPosition, std::int64_t startTimestamp,
                                                      Coordinate endPosition, std::int64_t endTimestamp) noexcept
    {
        const double distance = geodesicDistance(startPosition, endPosition);
        const double deltaT = static_cast<double>(endTimestamp - startTimestamp) / 1000.0;

        return {distance, distance / deltaT};
    }

    /*!
     * Calculates the initial bearing required to get from \p startPosition
     * to \p endPosition.
     *
     * \param startPosition
     *        the Coordinate of the start position [degrees]
     * \param endPosition
     *        the Coordinate of the end position [degrees]
     * \return the initial bearing [degrees] [0, 360[
     */
    inline double initialBearing(Coordinate startPosition, Coordinate endPosition) noexcept
    {
        double azimuth1 {0.0};
        double azimuth2 {0.0};
        try {
            const GeographicLib::Geodesic &geodesic = GeographicLib::Geodesic::WGS84();
            geodesic.Inverse(startPosition.first, startPosition.second, endPosition.first, endPosition.second, azimuth1, azimuth2);
        } catch (const std::exception &ex) {
#ifdef DEBUG
            qDebug() << "SkyMath::initialBearing: caught exception:" << ex.what();
#endif
            azimuth1 = 0.0;
        }

        // In degrees, converted to [0.0, 360.0[
        return std::fmod(azimuth1 + 360.0, 360.0);
    }

    /*!
     * Approximates the pitch angle [degrees] by assuming a straight distance line
     * and delta altitude, that is a triangle defined by \p sphericalDistance
     * and orthogonal \p deltaAltitude (both in [meters]). The estimated elevation
     * (pitch) angle should be exact enough for short distances.
     *
     * Note that we assume that the aircraft is not flying "upside down", so the
     * maximum estimated pitch angles are in [-90, 90] degrees. Also, we also assume
     * that the aircraft is never "perfectly flying straight up" (or down), or in
     * other words: if the \p sphericalDistance is 0.0 then the resulting pitch angle
     * will also be 0.0. While this is mathematically not quite correct (the pitch
     * angle would either be a perfect +90 or -90 degrees) it better reflects reality
     * when an aircraft is stationary on the ground, but the measured altitude values
     * fluctuate slightly.
     *
     * \param sphericalDistance
     *        the spherical distance [meters]
     * \param deltaAltitude
     *        the difference in altitude [meters]; positive values yield a
     *        positive pitch angle
     * \return the eximated pitch angle [-90, 90] [degrees]
     */
    constexpr double approximatePitch(double sphericalDistance, double deltaAltitude) noexcept
    {
        double pitch {0.0};
        if (!qFuzzyIsNull(deltaAltitude)) {
            if (sphericalDistance > 0.0) {
                pitch = std::atan(deltaAltitude / sphericalDistance);
            } else {
                // Mathematically the angle would be +/- 90 degrees, but when no
                // distance is travelled we assume that the aircraft is stationary,
                // or in other words: level (0.0 degrees pitch) on the ground
                pitch = 0.0;
            }
        }
        return Convert::radiansToDegrees(pitch);
    }

    /*!
     * Calculates the shortest heading change to get from the \p currentHeading to
     * the \p targetHeading. All headings are in degrees.
     *
     * The following convention is applied when turning exaclty 180 degrees:
     * - If the \p currentHeading is < \p targetHeading then a right turn (-180.0)
     *   is done
     * - Otherwise a left turn (180.0) is done
     *
     * This is in analogy with #interpolateHermite360.
     *
     * \param currentHeading
     *        the current heading [0, 360[ [degrees]
     * \param targetHeading
     *        the target heading the first heading [0, 360[ [degrees]
     * \return the required heading change [-180, 180] [degrees]; negative values
     *         correspond to clockwise ("right") turn; positive values
     *         correspond to anti-clockwise ("left") turn
     * \sa #interpolateHermite360
     * \sa https://forum.arduino.cc/t/calculating-heading-distance-and-direction/92144/6
     */
    constexpr double headingChange(double currentHeading, double targetHeading) noexcept
    {
        // The denormalizedHeading is always larger or equal than the targetHeading
        const double denormalizedHeading = currentHeading >= targetHeading ? currentHeading : currentHeading + 360.0;

        // Calculate left turn, will allways be in [0, 360[
        double headingChange = denormalizedHeading - targetHeading;

        // Take the smallest turn
        if (qFuzzyCompare(headingChange, 180.0)) {
            // If the - original (!) - currentHeading was smaller than the target
            // heading then we turn right 180 degrees (negative value), otherwise
            // left (positive value - by convention, which is in analogy how
            // interpolateHermite360 interpolates 180 degree turns)
            headingChange = currentHeading < targetHeading ? -180.0 : +180.0;
        } else if (headingChange > 180.0) {
            // Right turns are negative: -(360 - headingChange)
            headingChange = -360.0 + headingChange;
        }
        return headingChange;
    }

    /*!
     * Approximates the required bank angle required for the given \p headingChange.
     * The maximum bank angle is limited by \p maxBankAngle and required for turns
     * of \p maxBankAngleForHeadingChange degrees.
     *
     * \param headingChange
     *        the desired heading change [-180, 180] [degrees]; negative values
     *         correspond to clockwise ("right") turn; positive values
     *         correspond to anti-clockwise ("left") turn
     * \param maxBankAngleForHeadingChange
     *        the heading change (absolute value: [0, 180]) [degrees] for which the \p maxBankAngle
     *        is required
     * \param maxBankAngle
     *        the maximum bank angle (absolute value: [0, 90]) the aircraft typically can do
     *        (in either direction)
     * \return the approximate bank angle required for the heading change; negative values
     *         for "right turns"; positive values for "left turns"
     * \sa headingChange
     */
    constexpr double bankAngle(double headingChange, double maxBankAngleForHeadingChange, double maxBankAngle) noexcept
    {
        return std::min((std::abs(headingChange) / maxBankAngleForHeadingChange) * maxBankAngle, maxBankAngle) * sgn(headingChange);
    }

    /*!
     * Returns the relative position from the starting \p position,
     * given the \p bearing and geodesic \p distance.
     *
     * sinphi2    = sinphi1⋅cosδ + cosphi1⋅sinδ⋅costheta
     * tanΔlambda = sintheta⋅sinδ⋅cosphi1 / cosδ−sinphi1⋅sinphi2
     *
     * \param position
     *        the Coordinate of the position [degrees]
     * \param bearing
     *        the bearing of the destination point [degrees]
     * \param distance
     *        the geodesic distance to the destination point [meters]
     * \return the Coordinate of the relative position [degrees]
     */
    inline Coordinate relativePosition(Coordinate position, double bearing, double distance) noexcept
    {
        Coordinate destination;
        try {
            const GeographicLib::Geodesic &geodesic = GeographicLib::Geodesic::WGS84();
            geodesic.Direct(position.first, position.second, bearing, distance, destination.first, destination.second);
        } catch (const std::exception &ex) {
#ifdef DEBUG
            qDebug() << "SkyMath::relativePosition: caught exception:" << ex.what();
#endif
            destination.first = 0.0;
            destination.second = 0.0;
        }
        return destination;
    }

    inline bool isSameWaypoint(Coordinate wp1, Coordinate wp2, double threshold = DefaultDistanceThreshold) noexcept
    {
        const double distance = geodesicDistance(wp1, wp2);
        return distance < threshold;
    }

    /*!
     * Calculates the time difference (in milliseconds) between the given
     * \p fromDateTime to \p toDateTime (being possibly in different
     * time zones).
     *
     * - The time difference from the imported creation time to the creation
     *   time of the current flight is calculated
     * - That difference is NEGATIVE if the imported creation time is AFTER
     *   the current creation time (imported date "in the future") and...
     * - ... POSITIVE if the imported creation time is BEFORE the current
     *   creation time (imported date "in the past")
     *
     * So:
     * - If the imported creation time is "in the future", we want to apply a
     *   NEGATIVE time offset to the imported aircraft ("move it into the past"), and...
     * - ... if the imported creation time "is in the past" then we want to apply
     *   a POSITIVE time offset to the imported aircraft ("move it into the future")
     */
    inline std::int64_t calculateTimeOffset(TimeOffsetSync timeOffsetSync, const QDateTime &fromDateTime, const QDateTime &toDateTime) noexcept
    {
        QDateTime toDateTimeUtc = toDateTime.toUTC();
        QDateTime fromDateTimeUtc;;
        switch (timeOffsetSync) {
        case TimeOffsetSync::DateAndTime:
            fromDateTimeUtc = fromDateTime.toUTC();
            break;
        case TimeOffsetSync::TimeOnly:
            // Same date
            fromDateTimeUtc.setDate(toDateTimeUtc.date());
            fromDateTimeUtc.setTime(fromDateTime.toUTC().time());
            fromDateTimeUtc.setTimeZone(QTimeZone::UTC);
            break;
        case TimeOffsetSync::None:
            fromDateTimeUtc = toDateTimeUtc;
            break;
        }
        return fromDateTimeUtc.secsTo(toDateTimeUtc) * 1000;
    }

    /*!
     * Calculates the first \p n-th Fibonacci numbers, starting with 0 for \p n = 1.
     *
     * Note that the value of \p c must be equal to the value of \p N.
     *
     * \param n
     *        the number of Fibonacci numbers to calculate: n must be >= 1
     * \return an array with the first n Fibonacci numbers, starting with 0
     */
    template <std::size_t N>
    consteval std::array<int, N> calculateFibonacci(int n) noexcept
    {
        std::array<int, N> fibonaccis {};
        assert(fibonaccis.size() >= n);
        assert(n > 0);

        // Base cases
        fibonaccis[0] = 0;

        if (n > 1) {
            fibonaccis[1] = 1;
        }

        // Calculate the remaining Fibonacci numbers
        for (int i = 2; i < n; ++i)
        {
            fibonaccis[i] = fibonaccis[i - 1] + fibonaccis[i - 2];
        }

        return fibonaccis;
    }

    /*!
     * Calculates the next power of two value for the given \n.
     *
     * Note: if \p n is already a power of two then \p n is returned.
     * The lowest integer power of two is 1 (2^0), so n = 0 always returns 1.
     *
     * Examples:
     *
     * - n = 15 -> 16
     * - n = 16 -> 16
     * - n = 17 -> 32
     *
     * \param n
     *        a number for which to calculate the next power of two value
     * \return the next power of two value, including \p n (if \p n is
     *         already a power of two)
     */
    inline std::uint32_t nextPowerOfTwo(std::uint32_t n) noexcept
    {
        if (n != 0) {
            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            n++;
        } else {
            return 1;
        }
        return n;
    }

    /*!
     * Calculates the previous power of two value for the given \n.
     *
     * Note: if \p n is already a power of two then \p n is returned.
     * The lowest integer power of two is 1 (2^0), so n = 0 always returns 1.
     *
     * Examples:
     *
     * - n = 17 -> 16
     * - n = 16 -> 16
     * - n = 15 -> 8
     *
     * \param n
     *        a number for which to calculate the previous power of two value
     * \return the previous power of two value, including \p n (if \p n is
     *         already a power of two)
     */
    inline std::uint32_t previousPowerOfTwo(std::uint32_t n) noexcept
    {
        if (n != 0) {
            n = n | (n >> 1);
            n = n | (n >> 2);
            n = n | (n >> 4);
            n = n | (n >> 8);
        } else {
            return 1;
        }
        return n - (n >> 1);
    }

} // namespace

#endif // SKYMATH_H
