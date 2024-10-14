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
#ifndef POSITIONDATA_H
#define POSITIONDATA_H

#include "TimeVariableData.h"
#include "ModelLib.h"

struct MODEL_API PositionData final : public TimeVariableData
{
    // Position
    double latitude {0.0};
    double longitude {0.0};
    // Above mean sea level (MSL) altitude ("true altitude")
    double altitude {0.0};
    // Indicated pressure altitude (analytical purposes only)
    double indicatedAltitude {0.0};
    // Indicated altitude with the altimeter calibrated to current sea level pressure (analytical purposes only)
    double calibratedIndicatedAltitude {0.0};
    // Standard pressure altitude, that is at a 1013.25 hPa (1 atmosphere) setting (analytical purposes only)
    double pressureAltitude {0.0};

    /*!
     * Initialises the \p latitude, \p longitude and \p altitude, but not the pressure related altitudes
     * (that remain at 0.0).
     *
     * \param latitude
     *        the latitude [degrees]
     * \param longitude
     *        the longitude [degrees]
     * \param altitude
     *        the altitude [feet]
     */
    explicit PositionData(double latitude = 0.0, double longitude = 0.0, double altitude = 0.0) noexcept;

    /*!
     * Initialises all altitude values (true, indicated, pressure) to \p altitude.
     *
     * This is not quite correct, but useful for import formats that only provide one given
     * altitude value, as an approximation.
     *
     * \param altitude
     *        the common altitude value for initialisation of true, indicated and pressure altitudes [feet]
     */
    inline void initialiseCommonAltitude(double altitude)
    {
        this->altitude = altitude;
        indicatedAltitude = altitude;
        calibratedIndicatedAltitude = altitude;
        pressureAltitude = altitude;
    }
};

#endif // POSITIONDATA_H
