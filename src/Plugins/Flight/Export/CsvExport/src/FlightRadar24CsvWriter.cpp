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
#include <memory>
#include <vector>
#include <cstdint>
#include <cmath>

#include <QIODevice>
#include <QChar>
#include <QString>
#include <QStringBuilder>

#include <Kernel/Enum.h>
#include <Kernel/Unit.h>
#include <Kernel/SampleRate.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <PluginManager/Csv.h>
#include <PluginManager/Export.h>
#include "CsvExportSettings.h"
#include "FlightRadar24CsvWriter.h"

namespace
{
    const QString TimestampColumn = QStringLiteral("Timestamp");
    const QString UtcColumn = QStringLiteral("UTC");
    const QString CallsignColumn = QStringLiteral("Callsign");
    const QString PositionColumn = QStringLiteral("Position");
    const QString AltitudeColumn = QStringLiteral("Altitude");
    const QString SpeedColumn = QStringLiteral("Speed");
    const QString DirectionColumn = QStringLiteral("Direction");
}

struct FlightRadar24CsvWriterPrivate
{
    FlightRadar24CsvWriterPrivate(const CsvExportSettings &thePluginSettings) noexcept
        : pluginSettings(thePluginSettings)
    {}

    const CsvExportSettings &pluginSettings;
};

// PUBLIC

FlightRadar24CsvWriter::FlightRadar24CsvWriter(const CsvExportSettings &pluginSettings) noexcept
    : d(std::make_unique<FlightRadar24CsvWriterPrivate>(pluginSettings))
{}

FlightRadar24CsvWriter::~FlightRadar24CsvWriter() = default;

bool FlightRadar24CsvWriter::write(const Flight &flight, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    QString csv = QString(::TimestampColumn % Csv::CommaSep %
                          ::UtcColumn % Csv::CommaSep %
                          ::CallsignColumn % Csv::CommaSep %
                          ::PositionColumn % Csv::CommaSep %
                          ::AltitudeColumn % Csv::CommaSep %
                          ::SpeedColumn % Csv::CommaSep %
                          ::DirectionColumn % Csv::Ln
                          );

    bool ok = io.write(csv.toUtf8());
    if (ok) {
        const QDateTime startDateTimeUtc = flight.getAircraftStartZuluTime(aircraft);
        const QString callSign = aircraft.getAircraftInfo().flightNumber;
        const std::vector<PositionData> interpolatedPositionData = Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
        for (const PositionData &positionData : interpolatedPositionData) {
            if (!positionData.isNull()) {
                const QDateTime dateTimeUtc = startDateTimeUtc.addMSecs(positionData.timestamp);
                const std::int64_t secsSinceEpoch = dateTimeUtc.toSecsSinceEpoch();
                const QString csv = QString::number(secsSinceEpoch) % Csv::CommaSep %
                                    dateTimeUtc.toString(Qt::ISODate) % Csv::CommaSep %
                                    callSign % Csv::CommaSep %
                                    formatPosition(positionData) % Csv::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.altitude))) % Csv::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.velocityBodyZ))) % Csv::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.trueHeading))) % Csv::Ln;
                ok = io.write(csv.toUtf8());
            }
            if (!ok) {
                break;
            }
        }
    }

    return ok;
}

// PRIVATE

inline QString FlightRadar24CsvWriter::formatPosition(const PositionData &positionData) noexcept
{
    return Csv::DoubleQuote % Export::formatCoordinate(positionData.latitude) % "," % Export::formatCoordinate(positionData.longitude) % Csv::DoubleQuote;
}
