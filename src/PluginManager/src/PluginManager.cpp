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
#include <mutex>
#include <vector>
#include <unordered_map>

#include <QCoreApplication>
#include <QPluginLoader>
#include <QJsonObject>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QUuid>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/File.h>
#include <Model/Flight.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/LocationService.h>
#include <Flight/FlightImportIntf.h>
#include <Flight/FlightExportIntf.h>
#include <Location/LocationImportIntf.h>
#include <Location/LocationExportIntf.h>
#include "PluginManager.h"

namespace
{
    constexpr const char *FlightDirectoryName {"Flight"};
    constexpr const char *LocationDirectoryName {"Location"};
    constexpr const char *ExportDirectoryName {"Export"};
    constexpr const char *ImportDirectoryName {"Import"};
    constexpr const char *PluginUuidKey {"uuid"};
    constexpr const char *PluginNameKey {"name"};
}

struct PluginManagerPrivate
{
    PluginManagerPrivate() noexcept
    {
        pluginsDirectory.cd(File::getPluginDirectoryPath());
    }

    ~PluginManagerPrivate() = default;

    QWidget *parentWidget {nullptr};
    std::unique_ptr<LocationService> locationService {std::make_unique<LocationService>()};
    QDir pluginsDirectory;
    // Key: uuid - value: plugin path
    PluginManager::PluginRegistry flightImportPluginRegistry;
    PluginManager::PluginRegistry flightExportPluginRegistry;
    PluginManager::PluginRegistry locationImportPluginRegistry;
    PluginManager::PluginRegistry locationExportPluginRegistry;

    static inline std::once_flag onceFlag;
    static inline PluginManager *instance;
};

// PUBLIC

PluginManager &PluginManager::getInstance() noexcept
{
    std::call_once(PluginManagerPrivate::onceFlag, []() {
        PluginManagerPrivate::instance = new PluginManager();
    });
    return *PluginManagerPrivate::instance;
}

void PluginManager::destroyInstance() noexcept
{
    if (PluginManagerPrivate::instance != nullptr) {
        delete PluginManagerPrivate::instance;
        PluginManagerPrivate::instance = nullptr;
    }
}

void PluginManager::initialise(QWidget *parentWidget) noexcept
{
    d->parentWidget = parentWidget;
}

std::vector<PluginManager::Handle> PluginManager::initialiseFlightImportPlugins() noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    if (d->pluginsDirectory.exists(QString::fromLatin1(::FlightDirectoryName))) {
        d->pluginsDirectory.cd(QString::fromLatin1(::FlightDirectoryName));
        pluginHandles = enumeratePlugins(QString::fromLatin1(::ImportDirectoryName), d->flightImportPluginRegistry);
        d->pluginsDirectory.cdUp();
    }
    return pluginHandles;
}

std::vector<PluginManager::Handle> PluginManager::initialiseFlightExportPlugins() noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    if (d->pluginsDirectory.exists(QString::fromLatin1(::FlightDirectoryName))) {
        d->pluginsDirectory.cd(QString::fromLatin1(::FlightDirectoryName));
        pluginHandles = enumeratePlugins(QString::fromLatin1(::ExportDirectoryName), d->flightExportPluginRegistry);
        d->pluginsDirectory.cdUp();
    }
    return pluginHandles;
}

std::vector<PluginManager::Handle> PluginManager::initialiseLocationImportPlugins() noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    if (d->pluginsDirectory.exists(QString::fromLatin1(::LocationDirectoryName))) {
        d->pluginsDirectory.cd(QString::fromLatin1(::LocationDirectoryName));
        pluginHandles = enumeratePlugins(QString::fromLatin1(::ImportDirectoryName), d->locationImportPluginRegistry);
        d->pluginsDirectory.cdUp();
    }
    return pluginHandles;
}

std::vector<PluginManager::Handle> PluginManager::initialiseLocationExportPlugins() noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    if (d->pluginsDirectory.exists(QString::fromLatin1(::LocationDirectoryName))) {
        d->pluginsDirectory.cd(QString::fromLatin1(::LocationDirectoryName));
        pluginHandles = enumeratePlugins(QString::fromLatin1(::ExportDirectoryName), d->locationExportPluginRegistry);
        d->pluginsDirectory.cdUp();
    }
    return pluginHandles;
}

bool PluginManager::importFlights(const QUuid &pluginUuid, Flight &flight) const noexcept
{
    bool ok {false};
    if (d->flightImportPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath {d->flightImportPluginRegistry[pluginUuid]};
        QPluginLoader loader {pluginPath};
        QObject *plugin = loader.instance();
        auto *importPlugin = qobject_cast<FlightImportIntf *>(plugin);
        if (importPlugin != nullptr) {
            importPlugin->setParentWidget(d->parentWidget);
            importPlugin->restoreSettings(pluginUuid);
            ok = importPlugin->importFlights(flight);
            importPlugin->storeSettings(pluginUuid);
        }
        loader.unload();
    }
    return ok;
}

std::vector<FlightData> PluginManager::importSelectedFlights(const QUuid &pluginUuid, QIODevice &io, bool &ok) const noexcept
{
    std::vector<FlightData> flights;
    if (d->flightImportPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath {d->flightImportPluginRegistry[pluginUuid]};
        QPluginLoader loader {pluginPath};
        QObject *plugin = loader.instance();
        auto *importPlugin = qobject_cast<FlightImportIntf *>(plugin);
        if (importPlugin != nullptr) {
            importPlugin->setParentWidget(d->parentWidget);
            importPlugin->restoreSettings(pluginUuid);
            flights = importPlugin->importSelectedFlights(io, ok);
            importPlugin->storeSettings(pluginUuid);
        }
        loader.unload();
    }
    return flights;
}

bool PluginManager::exportFlight(const Flight &flight, const QUuid &pluginUuid) const noexcept
{
    bool ok {false};
    if (d->flightExportPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath {d->flightExportPluginRegistry[pluginUuid]};
        QPluginLoader loader {pluginPath};
        QObject *plugin = loader.instance();
        auto *exportPlugin = qobject_cast<FlightExportIntf *>(plugin);
        if (exportPlugin != nullptr) {
            exportPlugin->setParentWidget(d->parentWidget);
            exportPlugin->restoreSettings(pluginUuid);
            ok = exportPlugin->exportFlight(flight);
            exportPlugin->storeSettings(pluginUuid);
        }
        loader.unload();
    }
    return ok;
}

bool PluginManager::importLocations(const QUuid &pluginUuid) const noexcept
{
    bool ok {false};
    if (d->locationImportPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath {d->locationImportPluginRegistry[pluginUuid]};
        QPluginLoader loader {pluginPath};
        QObject *plugin = loader.instance();
        auto *importPlugin = qobject_cast<LocationImportIntf *>(plugin);
        if (importPlugin != nullptr) {
            importPlugin->setParentWidget(d->parentWidget);
            importPlugin->restoreSettings(pluginUuid);
            ok = importPlugin->importLocations();
            importPlugin->storeSettings(pluginUuid);
        }
        loader.unload();
    }
    return ok;
}

bool PluginManager::exportLocations(const QUuid &pluginUuid) const noexcept
{
    bool ok {false};
    if (d->locationExportPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath = d->locationExportPluginRegistry[pluginUuid];
        QPluginLoader loader(pluginPath);
        QObject *plugin = loader.instance();
        LocationExportIntf *exportPlugin = qobject_cast<LocationExportIntf *>(plugin);
        if (exportPlugin != nullptr) {
            exportPlugin->setParentWidget(d->parentWidget);
            exportPlugin->restoreSettings(pluginUuid);
            std::vector<Location> locations = d->locationService->getAll(&ok);
            if (ok) {
                ok = exportPlugin->exportLocations(locations);
            }
            exportPlugin->storeSettings(pluginUuid);
        }
        loader.unload();
    }
    return ok;
}

// PRIVATE

PluginManager::PluginManager() noexcept
    : d(std::make_unique<PluginManagerPrivate>())
{}

PluginManager::~PluginManager() = default;

std::vector<PluginManager::Handle> PluginManager::enumeratePlugins(const QString &pluginDirectoryName, PluginRegistry &pluginRegistry) noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    pluginRegistry.clear();
    if (d->pluginsDirectory.exists(pluginDirectoryName)) {
        d->pluginsDirectory.cd(pluginDirectoryName);
        const QStringList entryList = d->pluginsDirectory.entryList(QDir::Files);
        pluginHandles.reserve(entryList.count());
        for (const auto &fileName : entryList) {
            const QString pluginPath = d->pluginsDirectory.absoluteFilePath(fileName);
            QPluginLoader loader(pluginPath);

            const QJsonObject metaData = loader.metaData();
            if (!metaData.isEmpty()) {
                const QJsonObject pluginMetadata{
                    metaData.value(QString::fromLatin1("MetaData")).toObject()};
                const QUuid uuid{
                    pluginMetadata.value(QString::fromLatin1(::PluginUuidKey)).toString()};
                const QString pluginName{
                    pluginMetadata.value(QString::fromLatin1(::PluginNameKey)).toString()};
                const Handle handle {uuid, pluginName};
                pluginHandles.push_back(handle);
                pluginRegistry[uuid] = pluginPath;
            }
        }
        d->pluginsDirectory.cdUp();
    }

    return pluginHandles;
}
