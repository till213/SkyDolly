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
#ifndef CONVERT_H
#define CONVERT_H

#include <memory>
#include <exception>

#include <QtMath>
#ifdef DEBUG
#include <QDebug>
#endif

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
    Convert(const Convert &rhs) = delete;
    Convert(Convert &&rhs) noexcept;
    Convert &operator=(const Convert &rhs) = delete;
    Convert &operator=(Convert &&rhs) noexcept;
    ~Convert();

    /*!
     * Converts the \p height above WGS84 reference ellipsoid to height above the earth
     * gravity model (EGM) geoid according to the installed EGM data file. The conversion is
     * essentially evaluating the so-called undulation at position \p latitude, \p longitude
     * and adjusting the given \p height accordingly.
     *
     * If no EGM data file is available (not installed, not readable) then simply
     * \p meters is returned.
     *
     * Also refer to https://gisgeography.com/geoid-mean-sea-level/ and
     * https://www.mathworks.com/help/map/ellipsoid-geoid-and-orthometric-height.html
     *
     * \param height
     *        the altitude to convert [meters]
     * \param latitude
     *        the latitude of the position
     * \param longitude
     *        the longitude of the position
     * \return the converted altitude in meters above the EGM geoid; or \p height if no EGM data
     *         file could be read
     */
    inline double ellipsoidToGeoidHeight(double height, double latitude, double longitude) noexcept
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
                qDebug() << "Convert::wgs84ToEgmGeoid: caught exception:" << ex.what();
#endif
            }
        } else {
            heightAboveGeoid = height;
        }
        return heightAboveGeoid;
    }

    /*!
     * Converts the \p height above the EGM geoid to height above the WGS84 reference
     * ellipsoid according to the installed EGM data file. The conversion is
     * essentially evaluating the so-called undulation at position \p latitude, \p longitude
     * and adjusting the given \p height accordingly.
     *
     * When setting the \p height to 0 the so-called geoid height is returned.
     *
     * If no EGM data file is available (not installed, not readable) then simply
     * \p height is returned.
     *
     * Also refer to https://gisgeography.com/wgs84-world-geodetic-system/ and
     * https://www.mathworks.com/help/map/ellipsoid-geoid-and-orthometric-height.html
     *
     * \param height
     *        the altitude to convert [meters]
     * \param latitude
     *        the latitude of the position
     * \param longitude
     *        the longitude of the position
     * \return the converted altitude in meters above the WGS84 reference ellipsoid; or \p height
     *         if no EGM data file could be read
     */
    inline double geoidToEllipsoidHeight(double height, double latitude, double longitude) noexcept
    {
        // In meters
        double heightAboveEllipsoid {height};
        if (m_egm != nullptr) {
            try {
                // Convert height above EGM geoid to height above WGS84 ellipsoid (HAE) [meters]
                heightAboveEllipsoid = m_egm->ConvertHeight(latitude, longitude, height, GeographicLib::Geoid::GEOIDTOELLIPSOID);
            }
            catch (const std::exception &ex) {
                heightAboveEllipsoid = height;
#ifdef DEBUG
                qDebug() << "Convert::egmToWgs84Ellipsoid: caught exception:" << ex.what();
#endif
            }
        }
        return heightAboveEllipsoid;
    }

    static constexpr double degreesToRadians(double degree) noexcept {
        return degree * M_PI / 180.0;
    };

    /*!
     * Converts the \p radians to degrees.
     *
     * \param radians
     *        the radians to convert
     * \return the converted degrees
     */
    static constexpr double radiansToDegrees(double radians) noexcept {
        return radians * 180.0 / M_PI;
    };

    /*!
     * Converts the \p feet to meters.
     *
     * \return the \p feet converted to meters
     * \sa https://www.convertunits.com/from/feet/to/meter
     */
    static constexpr double feetToMeters(double feet) noexcept
    {
        return feet * 0.3048;
    }

    /*!
     * Converts the \p meters to feet.
     *
     * \return the \p meters converted to feet
     * \sa https://www.convertunits.com/from/meter/to/feet
     */
    static constexpr double metersToFeet(double meters) noexcept
    {
        return meters * 3.28083989501312;
    }

    /*!
     * Converts the \p feetPerSecond to knots.
     *
     * \param feetPerSecond
     *        the feet per second to convert
     * \return the converted knots
     * \sa https://www.convertunits.com/from/feet/second/to/knots
     */
    static constexpr double feetPerSecondToKnots(double feetPerSecond) noexcept
    {
        return feetPerSecond * 0.5924838012959;
    }

    /*!
     * Converts \p knots to feet per second.
     *
     * \param knots
     *        the knots to convert
     * \return the converted feet per second
     * \sa https://www.convertunits.com/from/knots/to/feet/second/
     */
    static constexpr double knotsToFeetPerSecond(double knots) noexcept
    {
        return knots * 1.6878098571012;
    }

    /*!
     * Converts the \p knots to meters per second.
     *
     * \param knots
     *        the knots to convert
     * \return the converted meters per second
     * \sa https://www.convertunits.com/from/knots/to/metre/second/
     */
    static constexpr double knotsToMetersPerSecond(double knots) noexcept
    {
        return knots * 0.51444444444444;
    }

    /*!
     * Converts the \p metersPerSecond to knots.
     *
     * \param metersPerSecond
     *        the meters per second to convert
     * \return the converted knots
     * \sa https://www.convertunits.com/from/knots/to/metre/second/
     */
    static constexpr double metersPerSecondToKnots(double metersPerSecond) noexcept
    {
        return metersPerSecond * 1.9438444924406;
    }

    /*!
     * Converts the \p feetPerSecond to km/h.
     *
     * \param feetPerSecond
     *        the feet per second to convert
     * \return the converted kilometers per hour
     * \sa https://www.convertunits.com/from/knots/to/kilometre/hour/
     */
    static constexpr double feetPerSecondToKilometersPerHour(double feetPerSecond) noexcept
    {
        return feetPerSecond * 1.09728;
    }

    /*!
     * Converts the \p metersPerSecond to feet/s.
     *
     * \param metersPerSecond
     *        the meters per second to convert
     * \return the converted feet per second
     * \sa https://www.convertunits.com/from/meter/second/to/foot/second
     */
    static constexpr double metersPerSecondToFeetPerSecond(double metersPerSecond) noexcept
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
    static constexpr double trueToIndicatedAirspeed(double trueAirspeed, double altitudeAboveSealevel) noexcept
    {
        const double altitudeFactor = altitudeAboveSealevel / 1000.0;
        return trueAirspeed / (1 + altitudeFactor * 0.02);
    }

private:
    std::unique_ptr<GeographicLib::Geoid> m_egm;
};

#endif // CONVERT_H
