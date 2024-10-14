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
#ifndef FLIGHTRECORDERCSVPARSER_H
#define FLIGHTRECORDERCSVPARSER_H

#include <memory>

#include <QByteArray>

class QIODevice;
class QDateTime;
class QString;

#include <Kernel/CsvParser.h>
#include "CsvParserIntf.h"

struct FlightData;
struct EngineData;
struct AircraftHandleData;
struct FlightRecorderCsvParserPrivate;

class FlightRecorderCsvParser : public CsvParserIntf
{
public:
    FlightRecorderCsvParser() noexcept;
    FlightRecorderCsvParser(const FlightRecorderCsvParser &rhs) = delete;
    FlightRecorderCsvParser(FlightRecorderCsvParser &&rhs) = delete;
    FlightRecorderCsvParser &operator=(const FlightRecorderCsvParser &rhs) = delete;
    FlightRecorderCsvParser &operator=(FlightRecorderCsvParser &&rhs) = delete;
    ~FlightRecorderCsvParser() override;

    FlightData parse(QIODevice &io, bool &ok) noexcept override;

private:
    const std::unique_ptr<FlightRecorderCsvParserPrivate> d;

    bool validateHeaders() noexcept;
    bool parseRow(const CsvParser::Row &row, FlightData &flightData) noexcept;
    inline void initEngineDefaultValues(EngineData &engineData) noexcept;
    inline void initAircraftHandleDefaultValues(AircraftHandleData &aircraftHandle) noexcept;
};

#endif // FLIGHTRECORDERCSVPARSER_H
