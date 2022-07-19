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

#include <memory>
#include <exception>

#include <GeographicLib/Geoid.hpp>

#include "KernelLib.h"

/*!
 * Unit converter.
 *
 * Useful links:
 * - https://www.convertunits.com/
 * - https://www.metric-conversions.org/length/feet-to-meters.htm
 * - https://www.omnicalculator.com/conversion/coordinates-converter#how-to-convert-latitude-and-longitude-coordinates
 */
class KERNEL_API Convert final
{
public:

    Convert() noexcept;
    ~Convert() noexcept;

    /*!
     * Converts the \c height height above WGS84 reference ellipsoid to height above the earth
     * gravity model (EGM) geoid according to the installed EGM data file. The conversion is
     * essentially evaluating the so-called undulation at position \c latitude, \c longitude
     * and adjusting the given \c height accordingly.
     *
     * If no EGM data file is available (not installed, not readable) then simply
     * \c meters is returned.
     *
     * Also refer to https://gisgeography.com/geoid-mean-sea-level/
     *
     * \param height
     *        the altitude to convert [meters]
     * \param latitude
     *        the latitude of the position
     * \param longitude
     *        the longitude of the position
     * \return the converted altitude in meters above the EGM geoid; or \c height if no EGM data
     *         file could be read
     */
    inline double wgs84ToEgmGeoid(double height, double latitude, double longitude) noexcept
    {
        // In meters
        double heightAboveGeoid {0.0};
        if (m_egm != nullptr) {
            try {
                // Convert height above WGS84 ellipsoid (HAE) tp height above EGM geoid [meters]
                heightAboveGeoid = m_egm->ConvertHeight(latitude, longitude, height, GeographicLib::Geoid::ELLIPSOIDTOGEOID);
            }
            catch (const std::exception &ex) {
                heightAboveGeoid = height;
#ifdef DEBUG
                qDebug("Convert::wgs84ToEgmGeoid: caught exception: %s", ex.what());
#endif
            }
        } else {
            heightAboveGeoid = height;
        }
        return heightAboveGeoid;
    }

    /*!
     * Converts the \c height height above the EGM geoid to height above the WGS84 reference
     * ellipsoid according to the installed EGM data file. The conversion is
     * essentially evaluating the so-called undulation at position \c latitude, \c longitude
     * and adjusting the given \c height accordingly.
     *
     * If no EGM data file is available (not installed, not readable) then simply
     * \c meters is returned.
     *
     * Also refer to https://gisgeography.com/wgs84-world-geodetic-system/
     *
     * \param height
     *        the altitude to convert [meters]
     * \param latitude
     *        the latitude of the position
     * \param longitude
     *        the longitude of the position
     * \return the converted altitude in meters above the WGS84 reference ellipsoid; or \c height
     *         if no EGM data file could be read
     */
    inline double egmToWgs84Ellipsoid(double height, double latitude, double longitude) noexcept
    {
        // In meters
        double heightAboveEllipsoid;
        if (m_egm != nullptr) {
            try {
                // Convert height above EGM geoid to height above WGS84 ellipsoid (HAE) [meters]
                heightAboveEllipsoid = m_egm->ConvertHeight(latitude, longitude, height, GeographicLib::Geoid::GEOIDTOELLIPSOID);
            }
            catch (const std::exception &ex) {
                heightAboveEllipsoid = height;
#ifdef DEBUG
                qDebug("Convert::egmToWgs84Ellipsoid: caught exception: %s", ex.what());
#endif
            }
        } else {
            heightAboveEllipsoid = height;
        }
        return heightAboveEllipsoid;
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

private:
    std::unique_ptr<GeographicLib::Geoid> m_egm;
};

#endif // CONVERT_H
