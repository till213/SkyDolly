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
#include <QString>
#include <QStringView>
#include <QRegularExpression>

#include <Kernel/Enum.h>
#include <Kernel/Version.h>
#include <Kernel/SampleRate.h>
#include <Kernel/Settings.h>
#include <Kernel/File.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include "Export.h"

namespace
{
    // Precision of exported double GNSS coordinate values
    // https://rapidlasso.com/2019/05/06/how-many-decimal-digits-for-storing-longitude-latitude/
    // https://xkcd.com/2170/
    constexpr int CoordinatePrecision = 6;

    // Precision of general number (altitude, heading, ...)
    constexpr int NumberPrecision = 2;
}

// PUBLIC

QString Export::suggestFilePath(const Flight &flight, QStringView suffix) noexcept
{
    // https://www.codeproject.com/tips/758861/removing-characters-which-are-not-allowed-in-windo
    static const QRegularExpression illegalInFileName = QRegularExpression("[\\\\/:*?""<>|]");
    QString suggestedFileName;
    const Settings &settings = Settings::getInstance();

    const QString &title = flight.getTitle();
    if (title.isNull()) {
        if (flight.count() > 0) {
            const Aircraft &aircraft = flight.getUserAircraft();
            suggestedFileName = aircraft.getAircraftInfo().aircraftType.type;
        } else {
            suggestedFileName = Version::getApplicationName();
        }
    } else {
        suggestedFileName = title;
    }

    suggestedFileName = suggestedFileName.replace(illegalInFileName, "_");
    return settings.getExportPath() + "/" + File::ensureSuffix(suggestedFileName, suffix);
}

QString Export::formatCoordinate(double coordinate) noexcept
{
    return QString::number(coordinate, 'f', CoordinatePrecision);
}

QString Export::formatNumber(double number) noexcept
{
    return QString::number(number, 'f', NumberPrecision);
}

void Export::resamplePositionDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod, std::back_insert_iterator<std::vector<PositionData>> backInsertIterator) noexcept
{
    // Position data
    Position &position = aircraft.getPosition();
    if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
        const std::int64_t duration = position.getLast().timestamp;
        const std::int64_t deltaTime = Enum::toUnderlyingType(resamplingPeriod);
        std::int64_t timestamp = 0;
        while (timestamp <= duration) {
            const PositionData &positionData = position.interpolate(timestamp, TimeVariableData::Access::Export);
            if (!positionData.isNull()) {
                backInsertIterator = positionData;
            }
            timestamp += deltaTime;
        }
    } else {
        // Original data requested
        std::copy(position.begin(), position.end(), backInsertIterator);
    }
}
