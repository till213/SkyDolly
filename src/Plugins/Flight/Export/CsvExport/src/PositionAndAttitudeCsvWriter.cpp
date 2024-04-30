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
#include <Model/FlightData.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <PluginManager/Csv.h>
#include <PluginManager/Export.h>
#include "CsvExportSettings.h"
#include "PositionAndAttitudeCsvWriter.h"

namespace
{
    constexpr const char *TimestampColumn {"Timestamp"};
    constexpr const char *UtcColumn {"UTC"};
    constexpr const char *Latitude {"Latitude"};
    constexpr const char *Longitude {"Longitude"};
    constexpr const char *AltitudeColumn {"Altitude"};
    constexpr const char *SpeedColumn {"Speed"};
    constexpr const char *PitchColumn {"Pitch"};
    constexpr const char *BankColumn {"Bank"};
    constexpr const char *HeadingColumn {"Heading"};
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

PositionAndAttitudeCsvWriter::~PositionAndAttitudeCsvWriter() = default;

bool PositionAndAttitudeCsvWriter::write(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept
{
    QString csv = QString::fromLatin1(::TimestampColumn) % Csv::CommaSep %
                                      ::UtcColumn % Csv::CommaSep %
                                      ::Latitude % Csv::CommaSep %
                                      ::Longitude % Csv::CommaSep %
                                      ::AltitudeColumn % Csv::CommaSep %
                                      ::SpeedColumn % Csv::CommaSep %
                                      ::PitchColumn % Csv::CommaSep %
                                      ::BankColumn % Csv::CommaSep %
                                      ::HeadingColumn % Csv::Ln;

    bool ok = io.write(csv.toUtf8());
    if (ok) {
        const QDateTime startDateTimeUtc = flightData.getAircraftStartZuluTime(aircraft);
        const std::vector<PositionData> interpolatedPositionData = Export::resamplePositionDataForExport(aircraft, d->pluginSettings.getResamplingPeriod());
        for (const PositionData &positionData : interpolatedPositionData) {
            const QDateTime dateTimeUtc = startDateTimeUtc.addMSecs(positionData.timestamp);
            const QString csv = QString::number(positionData.timestamp) % Csv::CommaSep %
                                dateTimeUtc.toString(Qt::ISODate) % Csv::CommaSep %
                                Export::formatCoordinate(positionData.latitude) % Csv::CommaSep %
                                Export::formatCoordinate(positionData.longitude) % Csv::CommaSep %
                                QString::number(static_cast<int>(std::round(positionData.altitude))) % Csv::CommaSep %
                                QString::number(static_cast<int>(std::round(positionData.velocityBodyZ))) % Csv::CommaSep %
                                QString::number(static_cast<int>(std::round(positionData.pitch))) % Csv::CommaSep %
                                QString::number(static_cast<int>(std::round(positionData.bank))) % Csv::CommaSep %
                                QString::number(static_cast<int>(std::round(positionData.trueHeading))) % Csv::Ln;
            ok = io.write(csv.toUtf8());
            if (!ok) {
                break;
            }
        }
    }

    return ok;
}
