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

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QTimeZone>
#include <QWidget>

#include <Kernel/Const.h>
#include <Kernel/Settings.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Persistence/Service/LogbookService.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/AircraftService.h>
#include <Persistence/Migration.h>
#include "SdLogImportSettings.h"
#include "SdLogImportPlugin.h"

struct SdlogImportPluginPrivate
{
    std::unique_ptr<LogbookService> logbookService {std::make_unique<LogbookService>(Const::ImportConnectionName)};
    std::unique_ptr<DatabaseService> databaseService {std::make_unique<DatabaseService>(Const::ImportConnectionName)};
    std::unique_ptr<FlightService> flightService {std::make_unique<FlightService>(Const::ImportConnectionName)};

    SdLogImportSettings pluginSettings;

    static inline const QString FileExtension {Const::LogbookExtension};
};

// PUBLIC

SdlogImportPlugin::SdlogImportPlugin() noexcept
    : d(std::make_unique<SdlogImportPluginPrivate>())
{}

SdlogImportPlugin::~SdlogImportPlugin() = default;

// PROTECTED

FlightImportPluginBaseSettings &SdlogImportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString SdlogImportPlugin::getFileExtension() const noexcept
{
    return SdlogImportPluginPrivate::FileExtension;
}

QString SdlogImportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Sky Dolly logbook (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> SdlogImportPlugin::createOptionWidget() const noexcept
{
    return nullptr;
}

std::vector<FlightData> SdlogImportPlugin::importSelectedFlights(QIODevice &io, bool &ok) noexcept
{
    std::vector<FlightData> flights;
    ok = false;
    // Only file-based SQLite databases supported
    auto *file = qobject_cast<QFile *>(&io);
    if (file != nullptr) {
        const QFileInfo fileInfo {*file};
        ok = d->databaseService->connectAndMigrate(fileInfo.absoluteFilePath(), Migration::Milestone::Schema);
        if (ok) {
            const std::vector<std::int64_t> flightIds = d->logbookService->getFlightIds({}, &ok);
            // We expect at least one flight to be imported (note that zero flights in a logbook
            // is a valid state, so the logbook service would return ok = true)
            ok = flightIds.size() > 0;
            if (ok) {
                flights.reserve(flightIds.size());
                for (const auto flightId : flightIds) {
                    FlightData flightData;
                    ok = d->flightService->importFlightData(flightId, flightData);
                    flights.push_back(std::move(flightData));
                }
            }
        }
    }
    return flights;
}

FlightAugmentation::Procedures SdlogImportPlugin::getAugmentationProcedures() const noexcept
{
    return FlightAugmentation::Procedure::None;
}

FlightAugmentation::Aspects SdlogImportPlugin::getAugmentationAspects() const noexcept
{
    return FlightAugmentation::Aspect::None;
}
