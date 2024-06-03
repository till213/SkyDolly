/**
 * Sky Dolly - The Black Sheep for Your Location Recordings
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

#include <QIODevice>
#include <QFile>
#include <QStringBuilder>
#include <QString>
#include <QDateTime>

#include <Kernel/Const.h>
#include <Kernel/Settings.h>
#include <Model/Location.h>
#include <Model/Aircraft.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/Service/LocationService.h>
#include <Persistence/Service/AircraftService.h>
#include <Persistence/Migration.h>
#include <PluginManager/Export.h>
#include "SdLogLocationExportSettings.h"
#include "SdLogLocationExportPlugin.h"

struct SdLogLocationExportPluginPrivate
{
    std::unique_ptr<DatabaseService> databaseService {std::make_unique<DatabaseService>(Const::ExportConnectionName)};
    std::unique_ptr<LocationService> locationService {std::make_unique<LocationService>(Const::ExportConnectionName)};
    SdLogLocationExportSettings pluginSettings;

    static inline const QString FileExtension {Const::LogbookExtension};
};

// PUBLIC

SdLogLocationExportPlugin::SdLogLocationExportPlugin() noexcept
    : d {std::make_unique<SdLogLocationExportPluginPrivate>()}
{}

SdLogLocationExportPlugin::~SdLogLocationExportPlugin() = default;

// PROTECTED

LocationExportPluginBaseSettings &SdLogLocationExportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString SdLogLocationExportPlugin::getFileExtension() const noexcept
{
    return SdLogLocationExportPluginPrivate::FileExtension;
}

QString SdLogLocationExportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Sky Dolly logbook (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> SdLogLocationExportPlugin::createOptionWidget() const noexcept
{
    // No custom settings yet
    return nullptr;
}

bool SdLogLocationExportPlugin::exportLocations(const std::vector<Location> &locations, QIODevice &io) const noexcept
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
            ok = d->locationService->exportAll(locations);
        }
    } else {
        // We only support file-based SQLite databases
        ok = false;
    }
    return ok;
}
