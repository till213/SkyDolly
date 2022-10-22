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
#include <QList>
#include <QString>
#include <QDateTime>
#include <QTimeZone>
#include <QIODevice>
#include <QFileInfo>
#include <QRegularExpression>

#include <Kernel/Convert.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include "FlightRadar24CsvParser.h"

namespace
{
    constexpr const char *FlightRadar24CSVPattern {R"(^(\d*),(?:\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z),([\w]*|[\d]*),\"([-]?\d{1,2}.\d+),([-]?\d{1,3}.\d+)\",(\d+),(\d+),(\d+))"};
    constexpr int UnixTimestampIndex = 1;
    constexpr int CallsignIndex = 2;
    constexpr int LatitudeIndex = 3;
    constexpr int LongitudeIndex = 4;
    constexpr int AltitudeIndex = 5;
    constexpr int SpeedIndex = 6;
    constexpr int HeadingIndex = 7;
}

// PUBLIC

bool FlightRadar24CsvParser::parse(QIODevice &io, QDateTime &firstDateTimeUtc, QString &flightNumber, Flight &flight) noexcept
{
    Aircraft &aircraft = flight.getUserAircraft();
    static const QRegularExpression regexp(::FlightRadar24CSVPattern);

    // Headers
    const QByteArray header = io.readLine();
    bool ok = !header.isNull();

    firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    QDateTime currentDateTimeUtc;
    currentDateTimeUtc.setTimeZone(QTimeZone::utc());
    std::int64_t timestamp;

    Position &position = aircraft.getPosition();

    // CSV data
    QByteArray data = io.readLine();
    while (ok && !data.isNull()) {
        const QRegularExpressionMatch match = regexp.match(data);
        if (match.hasMatch()) {

            PositionData positionData;

            // In seconds after 1970-01-01 UTC
            const std::int64_t unixTimestamp = match.capturedView(::UnixTimestampIndex).toLongLong(&ok);
            if (ok) {
                if (firstDateTimeUtc.isNull()) {
                    firstDateTimeUtc.setSecsSinceEpoch(unixTimestamp);
                    currentDateTimeUtc = firstDateTimeUtc;
                    timestamp = 0;
                    flightNumber = match.captured(::CallsignIndex);
                } else {
                    currentDateTimeUtc.setSecsSinceEpoch(unixTimestamp);
                    timestamp = firstDateTimeUtc.msecsTo(currentDateTimeUtc);
                }
            }
            if (ok) {
                positionData.timestamp = timestamp;
                positionData.latitude = match.capturedView(::LatitudeIndex).toDouble(&ok);
            }
            if (ok) {
                positionData.longitude = match.capturedView(::LongitudeIndex).toDouble(&ok);
            }
            if (ok) {
                positionData.altitude = match.capturedView(::AltitudeIndex).toDouble(&ok);
                positionData.indicatedAltitude = positionData.altitude;
            }
            if (ok) {
                positionData.velocityBodyZ = match.capturedView(::SpeedIndex).toDouble(&ok);
            }
            if (ok) {
                positionData.trueHeading = match.capturedView(::HeadingIndex).toDouble(&ok);
            }
            if (ok) {
                position.upsertLast(positionData);
            }

        }
        data = io.readLine();
    }

    return ok;
}
