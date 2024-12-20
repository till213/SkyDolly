/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

#include <QIODevice>
#include <QFile>
#include <QStringBuilder>
#include <QString>
#include <QDateTime>

#include <Kernel/Const.h>
#include <Kernel/Settings.h>
#include <Model/FlightData.h>
#include <Model/Aircraft.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/AircraftService.h>
#include <Persistence/Migration.h>
#include <PluginManager/Export.h>
#include "SdLogExportSettings.h"
#include "SdLogExportPlugin.h"

struct SdLogExportPluginPrivate
{
    std::unique_ptr<DatabaseService> databaseService {std::make_unique<DatabaseService>(Const::ExportConnectionName)};
    std::unique_ptr<FlightService> flightService {std::make_unique<FlightService>(Const::ExportConnectionName)};
    SdLogExportSettings pluginSettings;

    static inline const QString FileExtension {Const::LogbookExtension};
};

// PUBLIC

SdLogExportPlugin::SdLogExportPlugin() noexcept
    : d {std::make_unique<SdLogExportPluginPrivate>()}
{}

SdLogExportPlugin::~SdLogExportPlugin() = default;

// PROTECTED

FlightExportPluginBaseSettings &SdLogExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString SdLogExportPlugin::getFileExtension() const noexcept
{
    return SdLogExportPluginPrivate::FileExtension;
}

QString SdLogExportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Sky Dolly logbook (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> SdLogExportPlugin::createOptionWidget() const noexcept
{
    // No custom settings yet
    return nullptr;
}

bool SdLogExportPlugin::exportFlightData(const FlightData &flightData, QIODevice &io) const noexcept
{
    bool ok {true};
    auto *file = qobject_cast<QFile *>(&io);
    if (file != nullptr) {
        const QFileInfo fileInfo {*file};
        ok = d->databaseService->connect(fileInfo.absoluteFilePath());
        if (ok) {
            d->databaseService->migrate(Migration::Milestone::Schema);
        }
        if (ok) {
            ok = d->flightService->exportFlightData(flightData);
        }
    } else {
        // We only support file-based SQLite databases
        ok = false;
    }
    return ok;
}

bool SdLogExportPlugin::exportAircraft([[maybe_unused]] const FlightData &flightData, [[maybe_unused]] const Aircraft &aircraft, [[maybe_unused]] QIODevice &io) const noexcept
{
    bool ok {false};
    // Currently not supported
    return ok;
}
