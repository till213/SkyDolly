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
#ifndef GPXPARSER_H
#define GPXPARSER_H

#include <memory>
#include <vector>

#include <QDateTime>
#include <QString>

class QXmlStreamReader;
class QDateTime;

struct FlightData;
class GpxImportSettings;
struct GpxParserPrivate;

class GpxParser final
{
public:
    GpxParser(QXmlStreamReader &xmlStreamReader, const GpxImportSettings &pluginSettings) noexcept;
    GpxParser(const GpxParser &rhs) = delete;
    GpxParser(GpxParser &&rhs) = delete;
    GpxParser &operator=(const GpxParser &rhs) = delete;
    GpxParser &operator=(GpxParser &&rhs) = delete;
    ~GpxParser();

    std::vector<FlightData> parse() noexcept;

private:
    const std::unique_ptr<GpxParserPrivate> d;

    std::vector<FlightData> parseGPX() noexcept;
    void parseMetadata(FlightData &flightData) noexcept;
    void parseWaypoint(FlightData &flightData) noexcept;
    void parseRoute(FlightData &flightData) noexcept;
    void parseRoutePoint(FlightData &flightData) noexcept;
    void parseTrack(FlightData &flightData) noexcept;
    void parseTrackSegment(FlightData &flightData) noexcept;
    inline void parseTrackPoint(FlightData &flightData) noexcept;
    inline bool parseWaypointType(double &latitude, double &longitude, double &altitude, QString &identifier, QDateTime &dateTime) noexcept;
};

#endif // GPXPARSER_H
