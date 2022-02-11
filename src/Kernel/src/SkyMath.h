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
#ifndef SKYMATH_H
#define SKYMATH_H

#include <limits>
#include <utility>
#include <cmath>

#include <QtGlobal>

/*!
 * Mathematical functions for interpolation.
 *
 * Useful links:
 * - https://tools.timodenk.com/cubic-spline-interpolation
 * - https://www.wikihow.com/Write-Latitude-and-Longitude
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

    // Average earth radius [meters]
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

    template <typename T>
    T interpolateCatmullRom(
        T y0, T y1, T y2, T y3,
        T mu) noexcept
    {
        T a0, a1, a2, a3, mu2;

        mu2 = mu*mu;
        a0 = -0.5 * y0 + 1.5 * y1 - 1.5 * y2 + 0.5 * y3;
        a1 = y0 - 2.5 * y1 + 2 * y2 - 0.5 * y3;
        a2 = -0.5 * y0 + 0.5 * y2;
        a3 = y1;

#ifdef DEBUG
        qDebug("interpolateCatmullRom: mu: %f", mu);
#endif

        return (a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
    }

    /*!
     * Interpolates circular values in a range of [-180, 180[ using Hermite
     * (cubic) interpolation.
     *
     * \sa #interpolateHermite
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
     * \sa #interpolateHermite
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
     * Calculates the spherical distance [meters] of two points by using the ‘haversine’ formula to calculate
     * the great-circle distance between these two points.
     *
     * a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
     * c = 2 ⋅ atan2(√a, √(1−a))
     * d = R ⋅ c
     *
     * \param startPosition
     *        the <latitude, longitude> of the start position [degrees]
     * \param endPosition
     *        the <latitude, longitude> of the end position [degrees]
     * \param averageAltitude
     *        the average altitude of the two points [meters]
     * \return the spherical distance between the points [meters]
     * \sa https://www.movable-type.co.uk/scripts/latlong.html
     */
    inline double sphericalDistance(std::pair<double, double> startPosition, std::pair<double, double> endPosition, double averageAltitude) noexcept
    {
        const double radius = EarthRadius + averageAltitude;
        // phi,  lambda in radians
        const double phi1 = startPosition.first * M_PI / 180.0;
        const double phi2 = endPosition.first * M_PI / 180.0;
        const double deltaPhi = (endPosition.first - startPosition.first) * M_PI / 180.0;
        const double deltaLambda = (endPosition.second - startPosition.second) * M_PI / 180.0;

        const double sinDeltaPhi = std::sin(deltaPhi / 2.0);
        const double sinDeltaLambda = std::sin(deltaLambda / 2.0);
        const double a = sinDeltaPhi * sinDeltaPhi +
                         std::cos(phi1) * std::cos(phi2) *
                         sinDeltaLambda * sinDeltaLambda;
        const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

        return radius * c;
    }

    /*!
     * Calculates the spherical distance between the \c startPoint and the \c endPoint and the
     * velocity [meters per second] it takes to travel that distance, taking the timestamps \c startTimestamp
     * and \c endTimestamp into account.
     *
     * \param startPosition
     *        the <latitude, longitude> of the start position [degrees]
     * \param startTimestamp
     *        the timestamp of the start point [milliseconds]
     * \param endPosition
     *        the <latitude, longitude> of the end position [degrees]
     * \param endTimestamp
     *        the timestamp of the end point [milliseconds]
     * \param averageAltitude
     *        the average altitude of the two points [meters]
     * \return the distance (first value) and required speed [m/s] (second value)
     * \sa https://www.movable-type.co.uk/scripts/latlong.html
     */
    inline std::pair<double, double> distanceAndVelocity(std::pair<double, double> startPosition, qint64 startTimestamp,
                                                         std::pair<double, double> endPosition, qint64 endTimestamp,
                                                         double averageAltitude) noexcept
    {
        const double distance = sphericalDistance(startPosition, endPosition, averageAltitude);
        const double deltaT = (endTimestamp - startTimestamp) / 1000.0;

        return std::pair(distance, distance / deltaT);
    }

    /*!
     * Calculates the initial bearing required to get from \c startPosition
     * to \c endPosition.
     *
     * θ = atan2(sin Δλ ⋅ cos φ2 , cos φ1 ⋅ sin φ2 − sin φ1 ⋅ cos φ2 ⋅ cos Δλ)
     *
     * \param startPosition
     *        the <latitude, longitude> of the start position [degrees]
     * \param endPosition
     *        the <latitude, longitude> of the end position [degrees]
     * \return the initial bearing [degrees]
     * \sa https://www.movable-type.co.uk/scripts/latlong.html
     */
    inline double initialBearing(std::pair<double, double> startPosition, std::pair<double, double> endPosition) noexcept
    {
        const double phi1 = startPosition.first * M_PI / 180.0;
        const double lambda1 = startPosition.second * M_PI / 180.0;
        const double phi2 = endPosition.first * M_PI / 180.0;
        const double lambda2 = endPosition.second * M_PI / 180.0;

        const double y = std::sin(lambda2 - lambda1) * std::cos(phi2);
        const double x = std::cos(phi1) * std::sin(phi2) -
                         std::sin(phi1) * std::cos(phi2) * std::cos(lambda2 - lambda1);
        const double theta = std::atan2(y, x);
        // In degrees, converted to [0.0, 360.0[
        return std::fmod(theta * 180.0 / M_PI + 360.0, 360.0);
    }

    /*!
     * Approximates the pitch angle [degrees] by assuming a straight distance line
     * and delta altitude, that is a triangle defined by \c sphericalDistance
     * and orthogonal \c deltaAltitude (both in [meters]). The estimated elevation
     * (pitch) angle should be exact enough for short distances.
     *
     * Note that we assume that the aircraft is not flying "upside down", so the
     * maximum estimated pitch angles are in [-90, 90] degrees. Also, we also assume
     * that the aircraft is never "perfectly flying straight up" (or down), or in
     * other words: if the \c sphericalDistance is 0.0 then the resulting pitch angle
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
    inline double approximatePitch(double sphericalDistance, double deltaAltitude) noexcept
    {
        double pitch;
        if (!qFuzzyIsNull(deltaAltitude)) {
            if (sphericalDistance > 0.0) {
                pitch = std::atan(deltaAltitude / sphericalDistance);
            } else {
                // Mathematically the angle would be +/- 90 degrees, but when no
                // distance is travelled we assume that the aircraft is stationary,
                // or in other words: level (0.0 degrees pitch) on the ground
                pitch = 0.0;
            }
        } else {
            // Level flight
            pitch = 0.0;
        }
        return pitch * 180.0 / M_PI;
    }

    /*!
     * Calculates the shortest heading change to get from the \c currentHeading to
     * the \c targetHeading. All headings are in degrees.
     *
     * The following convention is applied when turning exaclty 180 degrees:
     * - If the \c currentHeading is < \c targetHeading then a right turn (-180.0)
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
    inline double headingChange(double currentHeading, double targetHeading) noexcept
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
     * Approximates the required bank angle required for the given \c headingChange.
     * The maximum bank angle is limited by \c maxBankAngle and required for turns
     * of \c maxBankAngleForHeadingChange degrees.
     *
     * \param headingChange
     *        the desired heading change [-180, 180] [degrees]; negative values
     *         correspond to clockwise ("right") turn; positive values
     *         correspond to anti-clockwise ("left") turn
     * \param maxBankAngleForHeadingChange
     *        the heading change (absolute value: [0, 180]) [degrees] for which the \c maxBankAngle
     *        is required
     * \param maxBankAngle
     *        the maximum bank angle (absolute value: [0, 90]) the aircraft typically can do
     *        (in either direction)
     * \return the approximate bank angle required for the heading change; negative values
     *         for "right turns"; positive values for "left turns"
     * \sa headingChange
     */
    inline double bankAngle(double headingChange, double maxBankAngleForHeadingChange, double maxBankAngle) noexcept
    {
        return qMin((std::abs(headingChange) / maxBankAngleForHeadingChange) * maxBankAngle, maxBankAngle) * SkyMath::sgn(headingChange);
    }

    /*!
     * Returns the relative position from the starting \c position at altitude \c altitude,
     * given the \c bearing and \c distance.
     *
     * sinphi2    = sinphi1⋅cosδ + cosphi1⋅sinδ⋅costheta
     * tanΔlambda = sintheta⋅sinδ⋅cosphi1 / cosδ−sinphi1⋅sinphi2
     *
     * \param position
     *        the <latitude, longitude> of the position [degrees]
     * \param altitude
     *        the altitude above sea level [meters]
     * \param bearing
     *        the bearing of the destination point [degrees]
     * \param distance
     *        the distance to the destination point [meters]
     * \return the <latitude, longitude> of the relative position [degrees]
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
