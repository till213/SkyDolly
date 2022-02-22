/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <memory>
#include <cstdint>
#include <limits>
#include <unordered_map>

#include <QByteArray>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QTimeZone>
#include <QFile>
#include <QFileInfo>

#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Model/src/SimVar.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Persistence/src/CSVConst.h"
#include "FlightRecorderCSVParser.h"

namespace
{
    constexpr char Separator = ',';

    // Column names
    constexpr char Milliseconds[] = "Milliseconds";
    constexpr char Latitude[] = "Latitude";
    constexpr char Longitude[] = "Longitude";
    constexpr char Altitude[] = "Altitude";
    constexpr char Pitch[] = "Pitch";
    constexpr char Bank[] = "Bank";
    constexpr char TrueHeading[] = "TrueHeading";

    constexpr int InvalidIndex = std::numeric_limits<int>::max();
}

class FlightRecorderCSVParserPrivate
{
public:
    FlightRecorderCSVParserPrivate()
    {
        firstDateTimeUtc.setTimeZone(QTimeZone::utc());
    }

    std::unordered_map<QByteArray, int> columnIndexes;
    QDateTime firstDateTimeUtc;
};

// PUBLIC

FlightRecorderCSVParser::FlightRecorderCSVParser() noexcept
    : d(std::make_unique<FlightRecorderCSVParserPrivate>())
{
#ifdef DEBUG
    qDebug("FlightRecorderCSVParser::~FlightRecorderCSVParser: CREATED");
#endif
}

FlightRecorderCSVParser::~FlightRecorderCSVParser() noexcept
{
#ifdef DEBUG
    qDebug("FlightRecorderCSVParser::~FlightRecorderCSVParser: DELETED");
#endif
}

bool FlightRecorderCSVParser::parse(QFile &file, QDateTime &firstDateTimeUtc, QString &flightNumber) noexcept
{
    firstDateTimeUtc = QFileInfo(file).birthTime().toUTC();
    bool ok = parseHeader(file);
    if (ok) {
        ok = parseData(file);
    }

    return ok;
}

// PRIVATE

bool FlightRecorderCSVParser::parseHeader(QFile &file) noexcept
{
    // Headers
    const QByteArray header = file.readLine();
    bool ok = !header.isNull();

    if (ok) {
        QList<QByteArray> headers = header.split(::Separator);
        ok = headers.count() > 0;
        if (ok) {
            int index = 0;
            for (const QByteArray &header : headers) {
                d->columnIndexes[header] = index;
                ++index;
            }
        }
    }

    return ok;
}

bool FlightRecorderCSVParser::parseData(QFile &file) noexcept
{
    int latitudeIndex = InvalidIndex;
    int longitudeIndex = InvalidIndex;
    int altitdueIndex = InvalidIndex;
    int pitchIndex = InvalidIndex;
    int bankIndex = InvalidIndex;
    int trueHeadingIndex = InvalidIndex;
    bool ok;
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Aircraft &aircraft = flight.getUserAircraft();
    Position &position = aircraft.getPosition();

    QByteArray data = file.readLine();
    // At least one data row expected
    ok = !data.isNull();
    bool firstRow = true;
    std::int64_t timestampDelta = 0;
    while (ok && !data.isNull()) {

        QList<QByteArray> values = data.split(::Separator);
        PositionData positionData;

        importTimestamp(values, firstRow, positionData.timestamp, timestampDelta);
        firstRow = false;

        // Position
        importValue(values, ::Latitude, latitudeIndex, positionData.latitude);
        importValue(values, ::Longitude, longitudeIndex, positionData.longitude);
        importValue(values, ::Altitude, altitdueIndex, positionData.altitude);
        importValue(values, ::Pitch, pitchIndex, positionData.pitch);
        importValue(values, ::Bank, bankIndex, positionData.bank);
        importValue(values, ::TrueHeading, trueHeadingIndex, positionData.heading);

        if (ok) {
            position.upsertLast(positionData);
        }

        data = file.readLine();
    }

    return ok;
}

inline bool FlightRecorderCSVParser::importTimestamp(const QList<QByteArray> &values, bool firstRow, std::int64_t &timestamp, std::int64_t &timestampDelta) noexcept
{
    static int timestampIndex = ::InvalidIndex;
    bool ok = true;;
    if (timestampIndex == ::InvalidIndex) {
        auto it = d->columnIndexes.find(::Milliseconds);
        if (it != d->columnIndexes.end()) {
            timestampIndex = it->second;
        } else {
            // No timestamp column
            ok = false;
        }
    }
    if (ok && timestampIndex < values.count()) {
        timestamp = values.at(timestampIndex).toLongLong(&ok);
        if (ok) {
            if (!firstRow) {
                timestamp += timestampDelta;
            } else {
                // The first timestamp must be 0, so shift all timestamps by
                // the timestamp delta, derived from the first timestamp
                // (which is usually 0 already)
                timestampDelta = -timestamp;
                timestamp = 0.0;
            }
        }
    }
    return ok;
}

template <typename T>
inline bool FlightRecorderCSVParser::importValue(const QList<QByteArray> &values, const char *name, int &index, T &value) noexcept
{
    bool ok = true;;
    if (index == ::InvalidIndex) {
        auto it = d->columnIndexes.find(name);
        if (it != d->columnIndexes.end()) {
            index = it->second;
        } else {
            // No timestamp column
            ok = false;
        }
    }
    if (ok && index < values.count()) {
        // @todo call proper toXYZ method based on type T
        value = values.at(index).toDouble(&ok);
    }
    return ok;
}
