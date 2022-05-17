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
#include <cmath>

#include <QIODevice>
#include <QChar>
#include <QString>
#include <QStringBuilder>

#include <Kernel/Convert.h>
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
    inline const QString TimestampColumn = QStringLiteral("Timestamp");
    inline const QString UtcColumn = QStringLiteral("UTC");
    inline const QString Latitude = QStringLiteral("Latitude");
    inline const QString Longitude = QStringLiteral("Longitude");
    inline const QString AltitudeColumn = QStringLiteral("Altitude");
    inline const QString SpeedColumn = QStringLiteral("Speed");
    inline const QString PitchColumn = QStringLiteral("Pitch");
    inline const QString BankColumn = QStringLiteral("Bank");
    inline const QString HeadingColumn = QStringLiteral("Heading");
}

class PositionAndVelocityCsvWriterPrivate
{
public:
    PositionAndVelocityCsvWriterPrivate(const CsvExportSettings &thePluginSettings) noexcept
        : pluginSettings(thePluginSettings)
    {}

    const CsvExportSettings &pluginSettings;

    static inline const QString FileExtension {QStringLiteral("csv")};
};

// PUBLIC

PositionAndAttitudeCsvWriter::PositionAndAttitudeCsvWriter(const CsvExportSettings &pluginSettings) noexcept
    : d(std::make_unique<PositionAndVelocityCsvWriterPrivate>(pluginSettings))
{
#ifdef DEBUG
    qDebug("PositionAndVelocityCsvWriter::PositionAndVelocityCsvWriter: CREATED");
#endif
}

PositionAndAttitudeCsvWriter::~PositionAndAttitudeCsvWriter() noexcept
{
#ifdef DEBUG
    qDebug("PositionAndVelocityCsvWriter::~PositionAndVelocityCsvWriter: DELETED");
#endif
}

bool PositionAndAttitudeCsvWriter::write(const Flight &flight, const Aircraft &aircraft, QIODevice &io) noexcept
{
    io.setTextModeEnabled(true);
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
        std::vector<PositionData> interpolatedPositionData;
        Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod(), std::back_inserter(interpolatedPositionData));
        for (PositionData &positionData : interpolatedPositionData) {
            if (!positionData.isNull()) {
                const QDateTime dateTimeUtc = startDateTimeUtc.addMSecs(positionData.timestamp);
                const QString csv = QString::number(positionData.timestamp) % CsvConst::CommaSep %
                                    dateTimeUtc.toString(Qt::ISODate) % CsvConst::CommaSep %
                                    Export::formatCoordinate(positionData.latitude) % CsvConst::CommaSep %
                                    Export::formatCoordinate(positionData.longitude) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.altitude))) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.velocityBodyZ))) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.pitch))) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.bank))) % CsvConst::CommaSep %
                                    QString::number(static_cast<int>(std::round(positionData.heading))) % CsvConst::Ln;
                ok = io.write(csv.toUtf8());
            }
            if (!ok) {
                break;
            }
        }
    }

    return ok;
}
