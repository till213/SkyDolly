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
#include <cstdint>
#include <utility>

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
#include <Model/Attitude.h>
#include <Model/AttitudeData.h>
#include <PluginManager/Csv.h>
#include "FlightRadar24CsvParser.h"

namespace Header
{
    constexpr const char *FlightRadar24Csv {"Timestamp,UTC,Callsign,Position,Altitude,Speed,Direction"};

    // Column names (also add them to FlightRadar24CsvParserPrivate::HeaderNames, for validation)
    constexpr const char *Timestamp {"Timestamp"};
    constexpr const char *Utc {"UTC"};
    constexpr const char *Callsign {"Callsign"};
    constexpr const char *Position {"Position"};
    constexpr const char *Altitude {"Altitude"};
    constexpr const char *Speed {"Speed"};
    constexpr const char *Direction {"Direction"};
}

struct FlightRadar24CsvParserPrivate
{
    FlightRadar24CsvParserPrivate()
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::UTC);
    }

    QDateTime firstDateTimeUtc;
    CsvParser::Headers headers;

    static constexpr std::array<const char *, 8> HeaderNames {
        Header::Timestamp,
        Header::Utc,
        Header::Callsign,
        Header::Altitude,
        Header::Position,
        Header::Altitude,
        Header::Speed,
        Header::Direction
    };
};

// PUBLIC

FlightRadar24CsvParser::FlightRadar24CsvParser() noexcept
    : d {std::make_unique<FlightRadar24CsvParserPrivate>()}
{}

FlightRadar24CsvParser::~FlightRadar24CsvParser() = default;

FlightData FlightRadar24CsvParser::parse(QIODevice &io, bool &ok) noexcept
{
    FlightData flightData;
    QDateTime firstDateTimeUtc;
    QString flightNumber;

    firstDateTimeUtc.setTimeZone(QTimeZone::UTC);

    CsvParser csvParser;
    QTextStream textStream(&io);
    textStream.setEncoding(QStringConverter::Utf8);
    CsvParser::Rows rows = csvParser.parse(textStream, Header::FlightRadar24Csv);
    d->headers = csvParser.getHeaders();
    ok = validateHeaders();
    if (ok) {
        ok = CsvParser::validate(rows, d->headers.size());
    }
    if (ok) {
        auto &aircraft = flightData.addUserAircraft();
        auto &position = aircraft.getPosition();
        auto &attitude = aircraft.getAttitude();
        position.reserve(rows.size());
        for (const auto &row : rows) {
            const auto positionAndAttitude = parsePosition(row, firstDateTimeUtc, flightNumber, ok);
            if (ok) {
                position.upsertLast(positionAndAttitude.first);
                attitude.upsertLast(positionAndAttitude.second);
            } else {
                break;
            }
        }
        if (ok) {
            flightData.creationTime = firstDateTimeUtc;
            flightData.flightNumber = flightNumber;
            FlightCondition &flightCondition = flightData.flightCondition;
            flightCondition.setStartZuluDateTime(firstDateTimeUtc);
            flightCondition.setStartZuluDateTime(firstDateTimeUtc.toLocalTime());
        }
    }
    return flightData;
}

// PRIVATE

bool FlightRadar24CsvParser::validateHeaders() const noexcept
{
    bool ok {true};
    for (const auto val : d->HeaderNames) {
        ok = d->headers.contains(val);
        if (!ok) {
            break;
        }
    }
    return ok;
}

inline std::pair<PositionData, AttitudeData> FlightRadar24CsvParser::parsePosition(const CsvParser::Row &row, QDateTime &firstDateTimeUtc, QString &flightNumber, bool &ok) const noexcept
{
    PositionData positionData;
    AttitudeData attitudeData;
    std::int64_t timestamp {0};
    QDateTime currentDateTimeUtc;
    currentDateTimeUtc.setTimeZone(QTimeZone::UTC);

    ok = true;
    // In seconds after 1970-01-01 UTC
    const std::int64_t unixTimestamp = row.at(d->headers.at(Header::Timestamp)).toLongLong(&ok);
    if (ok) {
        if (firstDateTimeUtc.isNull()) {
            firstDateTimeUtc.setSecsSinceEpoch(unixTimestamp);
            currentDateTimeUtc = firstDateTimeUtc;
            timestamp = 0;
            flightNumber = row.at(d->headers.at(Header::Callsign));
        } else {
            currentDateTimeUtc.setSecsSinceEpoch(unixTimestamp);
            timestamp = firstDateTimeUtc.msecsTo(currentDateTimeUtc);
        }
    }
    if (ok) {
        positionData.timestamp = timestamp;
        const QString &position = row.at(d->headers.at(Header::Position));
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
        const auto altitude = row.at(d->headers.at(Header::Altitude)).toDouble(&ok);
        if (ok) {
            positionData.initialiseCommonAltitude(altitude);
            // FlightRadar24 encodes "on ground" with an altitude of 0
            attitudeData.onGround = altitude == 0 ? true : false;
        }
    }
    if (ok) {
        attitudeData.timestamp = timestamp;
        attitudeData.velocityBodyZ = row.at(d->headers.at(Header::Speed)).toDouble(&ok);
    }
    if (ok) {
        attitudeData.trueHeading = row.at(d->headers.at(Header::Direction)).toDouble(&ok);
    }
    return std::make_pair(positionData, attitudeData);
}
