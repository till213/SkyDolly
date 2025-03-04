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
#ifndef FLIGHTRADAR24KMLPARSER_H
#define FLIGHTRADAR24KMLPARSER_H

#include <memory>
#include <vector>

#include <QDateTime>
#include <QString>

class QXmlStreamReader;

#include "KmlParserIntf.h"

struct FlightData;
struct FlightRadar24KmlParserPrivate;

class FlightRadar24KmlParser : public KmlParserIntf
{
public:
    FlightRadar24KmlParser() noexcept;
    FlightRadar24KmlParser(const FlightRadar24KmlParser &rhs) = delete;
    FlightRadar24KmlParser(FlightRadar24KmlParser &&rhs) = delete;
    FlightRadar24KmlParser &operator=(const FlightRadar24KmlParser &rhs) = delete;
    FlightRadar24KmlParser &operator=(FlightRadar24KmlParser &&rhs) = delete;
    ~FlightRadar24KmlParser() override;

    std::vector<FlightData> parse(QXmlStreamReader &xmlStreamReader) noexcept override;

private:
    const std::unique_ptr<FlightRadar24KmlParserPrivate> d;

    void parseName(FlightData &flightData) noexcept;
    void parseDocument() noexcept;
    void parseFolder() noexcept;
    void parsePlacemark() noexcept;
    bool parseDescription() noexcept;
    void parseTimestamp() noexcept;
    void parsePoint() noexcept;
};

#endif // FLIGHTRADAR24KMLPARSER_H
