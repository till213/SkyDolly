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
#ifndef CONVERT_H
#define CONVERT_H

#include "KernelLib.h"

/*!
 * Unit converter.
 *
 * Useful links:
 * - https://www.convertunits.com/
 * - https://www.metric-conversions.org/length/feet-to-meters.htm
 * - https://www.omnicalculator.com/conversion/coordinates-converter#how-to-convert-latitude-and-longitude-coordinates
 */
class KERNEL_API Convert
{
public:

    /*!
     * Converts a latitude or longitude coordinate given in degrees, minutes and seconds (e.g.
     * as string: 43° 30' 12.34'') to a single decimal value (double).
     *
     * \param degrees
     *        the degrees
     * \param minutes
     *        the minutes [0, 59]
     * \param seconds
     *        the seconds [0.0, 59.99]
     * \return the coordinate as a single fractional value
     */
    static inline double dms2dd(int degrees, int minutes, double seconds) noexcept
    {
        return degrees + static_cast<double>(minutes) / 60.0 + seconds / 3600.0;
    }

    /*!
     * Converts a latitude or longitude coordinate given in degrees and decimal minutes (e.g.
     * as string: 43° 30.1234') to a single decimal value (double).
     *
     * \param degrees
     *        the degrees
     * \param minutes
     *        the decimal minutes [0.0, 59.99]
     * \return the coordinate as a single fractional value
     */
    static inline double dm2dd(int degrees, double minutes) noexcept
    {
        return degrees + minutes / 60.0;
    }

    /*!
     * Converts a latitude or longitude coordinate given as single decimal value (double)
     * to degrees, minutes and seconds (e.g. as string: 43° 30' 12.34'').
     *
     * \param dd
     *        the coordinate as single fractional value
     * \param degrees
     *        the resulting degrees
     * \param minutes
     *        the resulting minutes [0, 59]
     * \param seconds
     *        the resulting seconds [0.0, 59.99]
     */
    static inline void dd2dms(double dd, int &degrees, int &minutes, double &seconds) noexcept
    {
        double absDegrees = std::abs(dd);
        degrees = static_cast<int>(absDegrees);
        double v = (absDegrees - static_cast<double>(degrees)) * 60.0;
        minutes = static_cast<int>(v);
        seconds = (v - static_cast<double>(minutes)) * 60.0;
    }

    /*!
     * Converts a latitude or longitude coordinate given as single decimal value (double)
     * to degrees and minutes (e.g. as string: 43° 30.34').
     *
     * \param dd
     *        the coordinate as single fractional value
     * \param degrees
     *        the resulting degrees
     * \param minutes
     *        the resulting minutes [0, 59.99]
     */
    static inline void dd2dm(double dd, int &degrees, double &minutes) noexcept
    {
        double absDegrees = std::abs(dd);
        degrees = static_cast<int>(absDegrees);
        minutes = (absDegrees - static_cast<double>(degrees)) * 60.0;
    }

    /*!
     * Converts the \c feet to meters.
     *
     * \return the \c feet converted to meters
     * \sa https://www.convertunits.com/from/feet/to/meter
     */
    static inline double feetToMeters(double feet) noexcept
    {
        return feet * 0.3048;
    }

    /*!
     * Converts the \c meters to feet.
     *
     * \return the \c meters converted to feet
     * \sa https://www.convertunits.com/from/meter/to/feet
     */
    static inline double metersToFeet(double meters) noexcept
    {
        return meters * 3.28083989501312;
    }

    /*!
     * Converts the \c feetPerSecond to knots.
     *
     * \param feetPerSecond
     *        the feet per second to convert
     * \return the converted knots
     * \sa https://www.convertunits.com/from/feet/second/to/knots
     */
    static inline double feetPerSecondToKnots(double feetPerSecond) noexcept
    {
        return feetPerSecond * 0.5924838012959;
    }

    /*!
     * Converts \c knots to feet per second.
     *
     * \param knots
     *        the knots to convert
     * \return the converted feet per second
     * \sa https://www.convertunits.com/from/knots/to/feet/second/
     */
    static inline double knotsToFeetPerSecond(double knots) noexcept
    {
        return knots * 1.6878098571012;
    }

    /*!
     * Converts the \c knots to meters per second.
     *
     * \param knots
     *        the knots to convert
     * \return the converted meters per second
     * \sa https://www.convertunits.com/from/knots/to/metre/second/
     */
    static inline double knotsToMetersPerSecond(double knots) noexcept
    {
        return knots * 0.51444444444444;
    }

    /*!
     * Converts the \c metersPerSecond to knots.
     *
     * \param metersPerSecond
     *        the meters per second to convert
     * \return the converted knots
     * \sa https://www.convertunits.com/from/knots/to/metre/second/
     */
    static inline double metersPerSecondToKnots(double metersPerSecond) noexcept
    {
        return metersPerSecond * 1.9438444924406;
    }

    /*!
     * Converts the \c feetPerSecond to km/h.
     *
     * \param feetPerSecond
     *        the feet per second to convert
     * \return the converted kilometers per hour
     * \sa https://www.convertunits.com/from/knots/to/kilometre/hour/
     */
    static inline double feetPerSecondToKilometersPerHour(double feetPerSecond) noexcept
    {
        return feetPerSecond * 1.09728;
    }

    /*!
     * Converts the \c metersPerSecond to feet/s.
     *
     * \param metersPerSecond
     *        the meters per second to convert
     * \return the converted feet per second
     * \sa https://www.convertunits.com/from/meter/second/to/foot/second
     */
    static inline double metersPerSecondToFeetPerSecond(double metersPerSecond) noexcept
    {
        return metersPerSecond * 3.28083989501312;
    }

    /*!
     * This is a "rule of thumb" conversion from true airspeed to indicated airspeed:
     * "The general rule of thumb is that true airspeed is an additional roughly 2% higher
     * than indicated airspeed for each 1,000 feet above sea level."
     *
     * https://www.pilotmall.com/blogs/news/how-to-calculate-true-airspeed-and-what-it-is-guide
     *
     * \param trueAirspeed
     *        the true airspeed [knots or km/h] to be converted to indicated airspeed [knots or km/h]
     * \param altitudeAboveSealevel
     *        the altitude above sea level [feet]
     * \return the estimated indicated airspeed [knots or km/h]
     */
    static inline double trueToIndicatedAirspeed(double trueAirspeed, double altitudeAboveSealevel) noexcept
    {
        const double altitudeFactor = altitudeAboveSealevel / 1000.0;
        return trueAirspeed / (1 + altitudeFactor * 0.02);
    }
};

#endif // CONVERT_H
