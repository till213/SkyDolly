/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#ifndef EXPORT_H
#define EXPORT_H

#include <vector>

#include <QStringView>
#include <QString>

#include <Kernel/Unit.h>
#include <Kernel/SampleRate.h>
#include "PluginManagerLib.h"

class Flight;
class Aircraft;
struct PositionData;
struct EngineData;
struct PrimaryFlightControlData;
struct SecondaryFlightControlData;
struct AircraftHandleData;
struct LightData;

/*!
 * Common export functionality.
 */
class PLUGINMANAGER_API Export
{
public:

    /*! Precision of general number (altitude, heading, ...). */
    static constexpr int NumberPrecision = 2;

    /*!
     * Returns a file path based on the title of the \c flight with the suggested file name having the given \c extension.
     *
     * \param flight
     *        the Flight from which the suggested file path is derived
     * \param extension
     *        the desired file extension (e.g. \e kml or \e csv)
     * \return the file path having a suggested file name based on the given \c flight
     */
    static QString suggestFlightFilePath(const Flight &flight, QStringView extension) noexcept;

    /*!
     * Returns a file path for location export with the suggested file name having the given \c extension.
     *
     * \param extension
     *        the desired file extension (e.g. \e kml or \e csv)
     * \return the file path having a suggested file name for location export
     */
    static QString suggestLocationFilePath(QStringView extension) noexcept;

    /*!
     * Formats the GNSS \c coordinate (latitude or longitude) with the appropriate decimal point precision.
     *
     * \param coordinate
     *        the coordinate to be formatted
     * \return the text representation of \c coordinate
     * \sa formatLatitude
     * \sa formatLongitude
     */
    static inline QString formatCoordinate(double coordinate) noexcept
    {
        // Note: coordinates are always formatted with a decimal point
        return Unit::formatCoordinate(coordinate);
    }

    /*!
     * Formats the general \c number (e.g. altitude or heading) with the appropriate decimal point precision.
     *
     * Note that in case the \c number should be displayed as text to a user, according to local settings, then
     * Unit::formatNumber should be used instead.
     *
     * \param number
     *        the number to be formatted as QString
     * \return the text representation of \c number
     */
    static inline QString formatNumber(double number) noexcept
    {
        return QString::number(number, 'f', NumberPrecision);
    }

    static std::vector<PositionData> resamplePositionDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept;
    static std::vector<EngineData> resampleEngineDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept;
    static std::vector<PrimaryFlightControlData> resamplePrimaryFlightControlDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept;
    static std::vector<SecondaryFlightControlData> resampleSecondaryFlightControlDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept;
    static std::vector<AircraftHandleData> resampleAircraftHandleDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept;
    static std::vector<LightData> resampleLightDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept;

};

#endif // EXPORT_H
