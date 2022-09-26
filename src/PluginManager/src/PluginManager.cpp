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
#include "ExportIntf.h"
#include "ImportIntf.h"
#include "PluginManager.h"

namespace
{
    constexpr char ExportDirectoryName[] = "Export";
    constexpr char ImportDirectoryName[] = "Import";
#if defined(Q_OS_MAC)
    constexpr char PluginDirectoryName[] = "PlugIns";
#else
    constexpr char PluginDirectoryName[] = "Plugins";
#endif
    constexpr char PluginUuidKey[] = "uuid";
    constexpr char PluginNameKey[] = "name";
}

struct PluginManagerPrivate
{
    PluginManagerPrivate() noexcept
        : parentWidget(nullptr)
    {
        pluginsDirectory = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
        if (pluginsDirectory.dirName() == "MacOS") {
            // Navigate up the app bundle structure, into the Contents folder
            pluginsDirectory.cdUp();
        }
#endif
        pluginsDirectory.cd(PluginDirectoryName);
    }

    ~PluginManagerPrivate() noexcept
    {}

    QWidget *parentWidget;
    QDir pluginsDirectory;
    // Plugin UUID / plugin path
    QMap<QUuid, QString> exportPluginRegistry;
    QMap<QUuid, QString> importPluginRegistry;

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

std::vector<PluginManager::Handle> PluginManager::initialiseExportPlugins() noexcept
{
    return enumeratePlugins(::ExportDirectoryName, d->exportPluginRegistry);
}

std::vector<PluginManager::Handle> PluginManager::initialiseImportPlugins() noexcept
{
    return enumeratePlugins(::ImportDirectoryName, d->importPluginRegistry);
}

bool PluginManager::importFlight(const QUuid &pluginUuid, FlightService &flightService, Flight &flight) const noexcept
{
    bool ok {false};
    if (d->importPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath = d->importPluginRegistry.value(pluginUuid);
        QPluginLoader loader(pluginPath);
        const QObject *plugin = loader.instance();
        ImportIntf *importPlugin = qobject_cast<ImportIntf *>(plugin);
        if (importPlugin != nullptr) {
            importPlugin->setParentWidget(d->parentWidget);
            importPlugin->restoreSettings(pluginUuid);
            ok = importPlugin->importFlight(flightService, flight);
            importPlugin->storeSettings(pluginUuid);
        } else {
            ok = false;
        }
        loader.unload();
    }
    return ok;
}

bool PluginManager::exportFlight(const Flight &flight, const QUuid &pluginUuid) const noexcept
{
    bool ok {false};
    if (d->exportPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath = d->exportPluginRegistry.value(pluginUuid);
        QPluginLoader loader(pluginPath);
        QObject *plugin = loader.instance();
        ExportIntf *exportPlugin = qobject_cast<ExportIntf *>(plugin);
        if (exportPlugin != nullptr) {
            exportPlugin->setParentWidget(d->parentWidget);
            exportPlugin->restoreSettings(pluginUuid);
            ok = exportPlugin->exportFlight(flight);
            exportPlugin->storeSettings(pluginUuid);
        } else {
            ok = false;
        }
        loader.unload();
    }
    return ok;
}

// PROTECTED

PluginManager::~PluginManager() noexcept
{
#ifdef DEBUG
    qDebug() << "PluginManager::~PluginManager: DELETED";
#endif
}

// PRIVATE

PluginManager::PluginManager() noexcept
    : d(std::make_unique<PluginManagerPrivate>())
{
#ifdef DEBUG
    qDebug() << "PluginManager::PluginManager: CREATED";
#endif
}

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
                const QJsonObject pluginMetadata = metaData.value("MetaData").toObject();
                const QUuid uuid = pluginMetadata.value(::PluginUuidKey).toString();
                const QString pluginName = pluginMetadata.value(::PluginNameKey).toString();
                const Handle handle = {uuid, pluginName};
                pluginHandles.push_back(handle);
                pluginRegistry.insert(uuid, pluginPath);
            }
        }
        d->pluginsDirectory.cdUp();
    }

    return pluginHandles;
}
