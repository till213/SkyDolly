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
#ifndef FLIGHTAWAREKMLPARSER_H
#define FLIGHTAWAREKMLPARSER_H

#include <memory>

#include <QDateTime>
#include <QString>

class QXmlStreamReader;

#include "AbstractKmlTrackParser.h"

struct FlightData;
class Aircraft;

class FlightAwareKmlParser : public AbstractKmlTrackParser
{
public:
    FlightAwareKmlParser() noexcept;
    FlightAwareKmlParser(const FlightAwareKmlParser &rhs) = delete;
    FlightAwareKmlParser(FlightAwareKmlParser &&rhs) = delete;
    FlightAwareKmlParser &operator=(const FlightAwareKmlParser &rhs) = delete;
    FlightAwareKmlParser &operator=(FlightAwareKmlParser &&rhs) = delete;
    ~FlightAwareKmlParser() override;

    std::vector<FlightData> parse(QXmlStreamReader &xmlStreamReader) noexcept override;

private:
    void parsePlacemark(FlightData &flightData) noexcept override;
    void parseWaypoint(FlightData &flightData, QString name) noexcept;
    void updateFlightWaypoints(std::vector<FlightData> &flights) noexcept;
    void updateAircraftWaypoints(Aircraft &aircraft) noexcept;
};

#endif // FLIGHTAWAREKMLPARSER_H
