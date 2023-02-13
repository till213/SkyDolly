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

#include <QCoreApplication>
#include <QPluginLoader>
#include <QJsonObject>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QMap>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/Flight.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/LocationService.h>
#include "FlightImportIntf.h"
#include "FlightExportIntf.h"
#include "LocationImportIntf.h"
#include "LocationExportIntf.h"
#include "PluginManager.h"

namespace
{
    constexpr const char *FlightDirectoryName {"Flight"};
    constexpr const char *LocationDirectoryName {"Location"};
    constexpr const char *ExportDirectoryName {"Export"};
    constexpr const char *ImportDirectoryName {"Import"};
#if defined(Q_OS_MAC)
    constexpr const char *PluginDirectoryName {"PlugIns"};
#else
    constexpr const char *PluginDirectoryName {"Plugins"};
#endif
    constexpr const char *PluginUuidKey {"uuid"};
    constexpr const char *PluginNameKey {"name"};
}

struct PluginManagerPrivate
{
    PluginManagerPrivate() noexcept
    {
        pluginsDirectory.setPath(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
        if (pluginsDirectory.dirName() == "MacOS") {
            // Navigate up the app bundle structure, into the Contents folder
            pluginsDirectory.cdUp();
        }
#endif
        pluginsDirectory.cd(PluginDirectoryName);
    }

    ~PluginManagerPrivate() = default;

    QWidget *parentWidget {nullptr};
    std::unique_ptr<LocationService> locationService {std::make_unique<LocationService>()};
    QDir pluginsDirectory;
    // Plugin UUID / plugin path
    QMap<QUuid, QString> flightImportPluginRegistry;
    QMap<QUuid, QString> flightExportPluginRegistry;
    QMap<QUuid, QString> locationImportPluginRegistry;
    QMap<QUuid, QString> locationExportPluginRegistry;

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
    if (d->pluginsDirectory.exists(::FlightDirectoryName)) {
        d->pluginsDirectory.cd(::FlightDirectoryName);
        pluginHandles = enumeratePlugins(::ImportDirectoryName, d->flightImportPluginRegistry);
        d->pluginsDirectory.cdUp();
    }
    return pluginHandles;
}

std::vector<PluginManager::Handle> PluginManager::initialiseFlightExportPlugins() noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    if (d->pluginsDirectory.exists(::FlightDirectoryName)) {
        d->pluginsDirectory.cd(::FlightDirectoryName);
        pluginHandles = enumeratePlugins(::ExportDirectoryName, d->flightExportPluginRegistry);
        d->pluginsDirectory.cdUp();
    }
    return pluginHandles;
}

std::vector<PluginManager::Handle> PluginManager::initialiseLocationImportPlugins() noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    if (d->pluginsDirectory.exists(::LocationDirectoryName)) {
        d->pluginsDirectory.cd(::LocationDirectoryName);
        pluginHandles = enumeratePlugins(::ImportDirectoryName, d->locationImportPluginRegistry);
        d->pluginsDirectory.cdUp();
    }
    return pluginHandles;
}

std::vector<PluginManager::Handle> PluginManager::initialiseLocationExportPlugins() noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    if (d->pluginsDirectory.exists(::LocationDirectoryName)) {
        d->pluginsDirectory.cd(::LocationDirectoryName);
        pluginHandles = enumeratePlugins(::ExportDirectoryName, d->locationExportPluginRegistry);
        d->pluginsDirectory.cdUp();
    }
    return pluginHandles;
}

bool PluginManager::importFlight(const QUuid &pluginUuid, Flight &flight) const noexcept
{
    bool ok {false};
    if (d->flightImportPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath {d->flightImportPluginRegistry.value(pluginUuid)};
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

bool PluginManager::exportFlight(const Flight &flight, const QUuid &pluginUuid) const noexcept
{
    bool ok {false};
    if (d->flightExportPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath {d->flightExportPluginRegistry.value(pluginUuid)};
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
        const QString pluginPath {d->locationImportPluginRegistry.value(pluginUuid)};
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
        const QString pluginPath = d->locationExportPluginRegistry.value(pluginUuid);
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

std::vector<PluginManager::Handle> PluginManager::enumeratePlugins(const QString &pluginDirectoryName, QMap<QUuid, QString> &pluginRegistry) noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    pluginRegistry.clear();
    if (d->pluginsDirectory.exists(pluginDirectoryName)) {
        d->pluginsDirectory.cd(pluginDirectoryName);
        const QStringList entryList = d->pluginsDirectory.entryList(QDir::Files);
        pluginHandles.reserve(entryList.count());
        for (const QString &fileName : entryList) {
            const QString pluginPath = d->pluginsDirectory.absoluteFilePath(fileName);
            QPluginLoader loader(pluginPath);

            const QJsonObject metaData = loader.metaData();
            if (!metaData.isEmpty()) {
                const QJsonObject pluginMetadata {metaData.value("MetaData").toObject()};
                const QUuid uuid {pluginMetadata.value(::PluginUuidKey).toString()};
                const QString pluginName {pluginMetadata.value(::PluginNameKey).toString()};
                const Handle handle {uuid, pluginName};
                pluginHandles.push_back(handle);
                pluginRegistry.insert(uuid, pluginPath);
            }
        }
        d->pluginsDirectory.cdUp();
    }

    return pluginHandles;
}
