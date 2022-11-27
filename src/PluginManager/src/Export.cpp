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
#include <QString>
#include <QStringView>
#include <QRegularExpression>
#include <QCoreApplication>

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

// PUBLIC

QString Export::suggestFlightFilePath(const Flight &flight, QStringView suffix) noexcept
{
    // https://www.codeproject.com/tips/758861/removing-characters-which-are-not-allowed-in-windo
    static const QRegularExpression illegalInFileName = QRegularExpression(R"([\\/:*?""<>|])");
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

QString Export::suggestLocationFilePath(QStringView suffix) noexcept
{
    QString suggestedFileName {QCoreApplication::translate("Export", "Locations")};

    const Settings &settings = Settings::getInstance();
    return settings.getExportPath() + "/" + File::ensureSuffix(suggestedFileName, suffix);
}

std::vector<PositionData> Export::resamplePositionDataForExport(const Aircraft &aircraft, const SampleRate::ResamplingPeriod resamplingPeriod) noexcept
{
    std::vector<PositionData> interpolatedPositionData;
    // Position data
    Position &position = aircraft.getPosition();
    if (resamplingPeriod != SampleRate::ResamplingPeriod::Original) {
        const std::int64_t duration = position.getLast().timestamp;
        const std::int64_t deltaTime = Enum::underly(resamplingPeriod);
        std::int64_t timestamp = 0;
        while (timestamp <= duration) {
            const PositionData &positionData = position.interpolate(timestamp, TimeVariableData::Access::Export);
            if (!positionData.isNull()) {
                interpolatedPositionData.push_back(positionData);
            }
            timestamp += deltaTime;
        }
    } else {
        // Original data requested
        interpolatedPositionData.reserve(position.count());
        std::copy(position.begin(), position.end(), std::back_inserter(interpolatedPositionData));
    }
    return interpolatedPositionData;
}
