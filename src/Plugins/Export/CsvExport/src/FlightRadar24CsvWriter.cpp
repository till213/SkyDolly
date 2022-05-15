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

#include <QIODevice>
#include <QChar>
#include <QString>
#include <QStringBuilder>

#include <Kernel/Enum.h>
#include <Kernel/SampleRate.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <PluginManager/CsvConst.h>
#include <PluginManager/Export.h>
#include "CsvExportSettings.h"
#include "FlightRadar24CsvWriter.h"

namespace
{
    inline const QString TimestampColumn = QStringLiteral("Timestamp");
    inline const QString UtcColumn = QStringLiteral("UTC");
    inline const QString CallsignColumn = QStringLiteral("Callsign");
    inline const QString PositionColumn = QStringLiteral("Position");
    inline const QString AltitudeColumn = QStringLiteral("Altitude");
    inline const QString SpeedColumn = QStringLiteral("Speed");
    inline const QString DirectionColumn = QStringLiteral("Direction");
}

class FlightRadar24CsvWriterPrivate
{
public:
    FlightRadar24CsvWriterPrivate(const CsvExportSettings &thePluginSettings) noexcept
        : pluginSettings(thePluginSettings)
    {}

    const CsvExportSettings &pluginSettings;

    static inline const QString FileExtension {QStringLiteral("csv")};
};

// PUBLIC

FlightRadar24CsvWriter::FlightRadar24CsvWriter(const CsvExportSettings &pluginSettings) noexcept
    : d(std::make_unique<FlightRadar24CsvWriterPrivate>(pluginSettings))
{
#ifdef DEBUG
    qDebug("FlightRadar24CsvWriter::FlightRadar24CsvWriter: CREATED");
#endif
}

FlightRadar24CsvWriter::~FlightRadar24CsvWriter() noexcept
{
#ifdef DEBUG
    qDebug("FlightRadar24CsvWriter::~FlightRadar24CsvWriter: DELETED");
#endif
}

bool FlightRadar24CsvWriter::write(const Flight &flight, const Aircraft &aircraft, QIODevice &io) noexcept
{
    io.setTextModeEnabled(true);
    QString csv = QString(::TimestampColumn % CsvConst::CommaSep %
                          ::UtcColumn % CsvConst::CommaSep %
                          ::CallsignColumn % CsvConst::CommaSep %
                          ::PositionColumn % CsvConst::CommaSep %
                          ::AltitudeColumn % CsvConst::CommaSep %
                          ::SpeedColumn % CsvConst::CommaSep %
                          ::DirectionColumn % CsvConst::Ln
                          );

    bool ok = io.write(csv.toUtf8());
    if (ok) {
        const QDateTime startDateTimeUtc = flight.getAircraftStartZuluTime(aircraft);
        const QString callSign = aircraft.getAircraftInfo().flightNumber;
        std::vector<PositionData> interpolatedPositionData;
        Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod(), std::back_inserter(interpolatedPositionData));
        for (PositionData &positionData : interpolatedPositionData) {
            if (!positionData.isNull()) {
                const QDateTime dateTimeUtc = startDateTimeUtc.addMSecs(positionData.timestamp);
                const std::int64_t secsSinceEpoch = dateTimeUtc.toSecsSinceEpoch();
                const QString csv = QString::number(secsSinceEpoch) % CsvConst::CommaSep %
                                    dateTimeUtc.toString(Qt::ISODate) % CsvConst::CommaSep %
                                    callSign % CsvConst::CommaSep %
                                    formatPosition(positionData) % CsvConst::CommaSep %
                                    QString::number(qRound(positionData.altitude)) % CsvConst::CommaSep %
                                    QString::number(qRound(positionData.velocityBodyZ)) % CsvConst::CommaSep %
                                    QString::number(qRound(positionData.heading)) % CsvConst::Ln;
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
    return CsvConst::DoubleQuote % Export::formatCoordinate(positionData.latitude) % "," % Export::formatCoordinate(positionData.longitude) % CsvConst::DoubleQuote;
}
