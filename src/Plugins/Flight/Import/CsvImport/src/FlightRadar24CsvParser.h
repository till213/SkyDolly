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
#ifndef FLIGHTRADAR24CSVPARSER_H
#define FLIGHTRADAR24CSVPARSER_H

#include <memory.h>

class QIODevice;
class QDateTime;
class QString;

#include <Kernel/CsvParser.h>
#include <Model/PositionData.h>
#include "CsvParserIntf.h"

struct FlightData;
struct FlightRadar24CsvParserPrivate;

class FlightRadar24CsvParser : public CsvParserIntf
{
public:
    FlightRadar24CsvParser() noexcept;
    ~FlightRadar24CsvParser() override;

    bool parse(QIODevice &io, FlightData &flightData) noexcept override;
private:
    std::unique_ptr<FlightRadar24CsvParserPrivate> d;

    bool validateHeaders() noexcept;
    inline PositionData parsePosition(const CsvParser::Row &row, QDateTime &firstDateTimeUtc, QString &flightNumber, bool &ok) const noexcept;
};

#endif // FLIGHTRADAR24CSVPARSER_H
