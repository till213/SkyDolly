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
#include <cstdint>

#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QTimeZone>
#include <QIODevice>
#include <QFileInfo>
#include <QTextStream>
#include <QStringConverter>

#include <Kernel/Convert.h>
#include <Kernel/Enum.h>
#include <Kernel/CsvParser.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <PluginManager/Csv.h>
#include "FlightRadar24CsvParser.h"

namespace
{
    constexpr const char *FlightRadar24CsvHeader {"Timestamp,UTC,Callsign,Position,Altitude,Speed,Direction"};

    enum struct Index
    {
        UnixTimestamp = 0,
        Callsign,
        Position,
        Altitude,
        Speed,
        Heading,
        // Last index
        Count
    };
}

// PUBLIC

bool FlightRadar24CsvParser::parse(QIODevice &io, QDateTime &firstDateTimeUtc, QString &flightNumber, FlightData &flightData) noexcept
{
    Aircraft aircraft;
    Position &position = aircraft.getPosition();
    firstDateTimeUtc.setTimeZone(QTimeZone::utc());

    CsvParser csvParser;
    QTextStream textStream(&io);
    textStream.setEncoding(QStringConverter::Utf8);
    CsvParser::Rows rows = csvParser.parse(textStream, ::FlightRadar24CsvHeader);
    bool ok = CsvParser::validate(rows, Enum::underly(::Index::Count));
    if (ok) {
        position.reserve(rows.size());
        for (const auto &row : rows) {
            const PositionData positionData = parsePosition(row, firstDateTimeUtc, flightNumber, ok);
            if (ok) {
                position.upsertLast(positionData);
            } else {
                break;
            }
        }
    }
    if (ok) {
        flightData.aircraft.push_back(std::move(aircraft));
    }
    return ok;
}

inline PositionData FlightRadar24CsvParser::parsePosition(const CsvParser::Row &row, QDateTime &firstDateTimeUtc, QString &flightNumber, bool &ok) noexcept
{
    PositionData positionData;
    std::int64_t timestamp {0};
    QDateTime currentDateTimeUtc;
    currentDateTimeUtc.setTimeZone(QTimeZone::utc());

    ok = true;
    // In seconds after 1970-01-01 UTC
    const std::int64_t unixTimestamp = row.at(Enum::underly(::Index::UnixTimestamp)).toLongLong(&ok);
    if (ok) {
        if (firstDateTimeUtc.isNull()) {
            firstDateTimeUtc.setSecsSinceEpoch(unixTimestamp);
            currentDateTimeUtc = firstDateTimeUtc;
            timestamp = 0;
            flightNumber = row.at(Enum::underly(::Index::Callsign));
        } else {
            currentDateTimeUtc.setSecsSinceEpoch(unixTimestamp);
            timestamp = firstDateTimeUtc.msecsTo(currentDateTimeUtc);
        }
    }
    if (ok) {
        positionData.timestamp = timestamp;
        const QString &position = row.at(Enum::underly(::Index::Position));
        const QStringList coordinates = position.split(',');
        if (coordinates.size() == 2) {
            positionData.latitude = coordinates.first().toDouble(&ok);
            if (ok) {
                positionData.longitude = coordinates.last().toDouble(&ok);
            }
        } else {
            ok = false;
        }
    }
    if (ok) {
        positionData.altitude = row.at(Enum::underly(::Index::Altitude)).toDouble(&ok);
        positionData.indicatedAltitude = positionData.altitude;
    }
    if (ok) {
        positionData.velocityBodyZ = row.at(Enum::underly(::Index::Speed)).toDouble(&ok);
    }
    if (ok) {
        positionData.trueHeading = row.at(Enum::underly(::Index::Heading)).toDouble(&ok);
    }
    return positionData;
}
