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
#ifndef FLIGHTRADAR24CSVPARSER_H
#define FLIGHTRADAR24CSVPARSER_H

#include <memory.h>
#include <utility>

class QIODevice;
class QDateTime;
class QString;

#include <Kernel/CsvParser.h>
#include <Model/PositionData.h>
#include <Model/AttitudeData.h>
#include "CsvParserIntf.h"

struct FlightData;
struct FlightRadar24CsvParserPrivate;

class FlightRadar24CsvParser : public CsvParserIntf
{
public:
    FlightRadar24CsvParser() noexcept;
    FlightRadar24CsvParser(const FlightRadar24CsvParser &rhs) = delete;
    FlightRadar24CsvParser(FlightRadar24CsvParser &&rhs) = delete;
    FlightRadar24CsvParser &operator=(const FlightRadar24CsvParser &rhs) = delete;
    FlightRadar24CsvParser &operator=(FlightRadar24CsvParser &&rhs) = delete;
    ~FlightRadar24CsvParser() override;

    FlightData parse(QIODevice &io, bool &ok) noexcept override;
private:
    const std::unique_ptr<FlightRadar24CsvParserPrivate> d;

    bool validateHeaders() const noexcept;
    inline std::pair<PositionData, AttitudeData> parsePosition(const CsvParser::Row &row, QDateTime &firstDateTimeUtc, QString &flightNumber, bool &ok) const noexcept;
};

#endif // FLIGHTRADAR24CSVPARSER_H
