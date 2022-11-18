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
#include <cstdint>
#include <cmath>
#include <vector>

#include <QIODevice>
#include <QChar>
#include <QString>
#include <QStringBuilder>

#include <Kernel/Convert.h>
#include <Kernel/Unit.h>
#include <Kernel/Enum.h>
#include <Kernel/SampleRate.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <PluginManager/CsvConst.h>
#include <PluginManager/Export.h>
#include "CsvExportSettings.h"
#include "PositionAndAttitudeCsvWriter.h"

namespace
{
    const QString TimestampColumn = QStringLiteral("Timestamp");
    const QString UtcColumn = QStringLiteral("UTC");
    const QString Latitude = QStringLiteral("Latitude");
    const QString Longitude = QStringLiteral("Longitude");
    const QString AltitudeColumn = QStringLiteral("Altitude");
    const QString SpeedColumn = QStringLiteral("Speed");
    const QString PitchColumn = QStringLiteral("Pitch");
    const QString BankColumn = QStringLiteral("Bank");
    const QString HeadingColumn = QStringLiteral("Heading");
}

struct PositionAndAttitudeCsvWriterPrivate
{
    PositionAndAttitudeCsvWriterPrivate(const CsvExportSettings &thePluginSettings) noexcept
        : pluginSettings(thePluginSettings)
    {}

    const CsvExportSettings &pluginSettings;
};

// PUBLIC

PositionAndAttitudeCsvWriter::PositionAndAttitudeCsvWriter(const CsvExportSettings &pluginSettings) noexcept
    : d(std::make_unique<PositionAndAttitudeCsvWriterPrivate>(pluginSettings))
{}

PositionAndAttitudeCsvWriter::~PositionAndAttitudeCsvWriter() noexcept = default;

bool PositionAndAttitudeCsvWriter::write(const Flight &flight, const Aircraft &aircraft, QIODevice &io) noexcept
{
    QString csv = QString(::TimestampColumn % CsvConst::CommaSep %
                          ::UtcColumn % CsvConst::CommaSep %
                          ::Latitude % CsvConst::CommaSep %
                          ::Longitude % CsvConst::CommaSep %
                          ::AltitudeColumn % CsvConst::CommaSep %
                          ::SpeedColumn % CsvConst::CommaSep %
                          ::PitchColumn % CsvConst::CommaSep %
                          ::BankColumn % CsvConst::CommaSep %
                          ::HeadingColumn % CsvConst::Ln
                          );

    bool ok = io.write(csv.toUtf8());
    if (ok) {
        const QDateTime startDateTimeUtc = flight.getAircraftStartZuluTime(aircraft);
        const std::vector<PositionData> interpolatedPositionData = Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
        for (const PositionData &positionData : interpolatedPositionData) {
            if (!positionData.isNull()) {
                const QDateTime dateTimeUtc = startDateTimeUtc.addMSecs(positionData.timestamp);
                const QString csv = QString::number(positionData.timestamp) % CsvConst::CommaSep %
                                    dateTimeUtc.toString(Qt::ISODate) % CsvConst::CommaSep %
                                    Unit::formatCoordinate(positionData.latitude) % CsvConst::CommaSep %
                                    Unit::formatCoordinate(positionData.longitude) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.altitude))) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.velocityBodyZ))) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.pitch))) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.bank))) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.trueHeading))) % CsvConst::Ln;
                ok = io.write(csv.toUtf8());
            }
            if (!ok) {
                break;
            }
        }
    }

    return ok;
}
