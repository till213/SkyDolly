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
#ifndef SKYDOLLYCSVPARSER_H
#define SKYDOLLYCSVPARSER_H

#include <cstdint>

#include <QByteArray>
#include <QList>

class QIODevice;
class QDateTime;
class QString;

#include <Kernel/CsvParser.h>
#include <Model/PositionData.h>
#include "CsvParserIntf.h"

class Flight;
class Aircraft;
class Engine;
class PrimaryFlightControl;
class SecondaryFlightControl;
class AircraftHandle;
class Light;

class SkyDollyCsvParser : public CsvParserIntf
{
public:
    bool parse(QIODevice &io, QDateTime &firstDateTimeUtc, QString &flightNumber, Flight &flight) noexcept override;

private:
    std::int64_t m_timestampDelta {0};
    inline bool parseRow(const CsvParser::Row &row, Aircraft &aircraft) noexcept;
    inline bool importPositionData(const CsvParser::Row &row, Aircraft &aircraft) noexcept;
    inline bool importEngineData(const CsvParser::Row &row, Engine &engine) noexcept;
    inline bool importPrimaryFlightControlData(const CsvParser::Row &row, PrimaryFlightControl &primaryFlightControl) noexcept;
    inline bool importSecondaryFlightControlData(const CsvParser::Row &row, SecondaryFlightControl &secondaryFlightControl) noexcept;
    inline bool importAircraftHandleData(const CsvParser::Row &row, AircraftHandle &aircraftHandle) noexcept;
    inline bool importLightData(const CsvParser::Row &row, Light &light) noexcept;
};

#endif // SKYDOLLYCSVPARSER_H
