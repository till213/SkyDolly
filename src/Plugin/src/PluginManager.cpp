/**
 * Sky Dolly - The black sheep for your flight recordings
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

#include <QCoreApplication>
#include <QPluginLoader>
#include <QJsonObject>
#include <QUuid>
#include <QMap>
#include <QDir>
#include <QStringList>

#include "../../Persistence/src/Service/FlightService.h"
#include "ExportIntf.h"
#include "ImportIntf.h"
#include "PluginManager.h"

namespace
{
    constexpr char ExportDirectoryName[] = "export";
    constexpr char ImportDirectoryName[] = "import";
#if defined(Q_OS_MAC)
    constexpr char PluginDirectoryName[] = "PlugIns";
#else
    constexpr char PluginDirectoryName[] = "plugins";
#endif
}

class PluginManagerPrivate
{
public:
    PluginManagerPrivate() noexcept
    {
        pluginsDirectoryPath = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
        if (pluginsDirectoryPath.dirName() == "MacOS") {
            // Navigate up the app bundle structure, into the Contents folder
            pluginsDirectoryPath.cdUp();
        }
#endif
        pluginsDirectoryPath.cd(PluginDirectoryName);
    }

    ~PluginManagerPrivate() noexcept
    {}

    QDir pluginsDirectoryPath;
    // UUID / plugin path
    QMap<QUuid, QString> exportPluginRegistry;
    QMap<QUuid, QString> importPluginRegistry;
    static PluginManager *instance;
};

PluginManager *PluginManagerPrivate::instance = nullptr;

// PUBLIC

PluginManager &PluginManager::getInstance() noexcept
{
    if (PluginManagerPrivate::instance == nullptr) {
        PluginManagerPrivate::instance = new PluginManager();
    }
    return *PluginManagerPrivate::instance;
}

void PluginManager::destroyInstance() noexcept
{
    if (PluginManagerPrivate::instance != nullptr) {
        delete PluginManagerPrivate::instance;
        PluginManagerPrivate::instance = nullptr;
    }
}

std::vector<PluginManager::Handle> PluginManager::enumerateExportPlugins() noexcept
{
    return enumeratePlugins(ExportDirectoryName, d->exportPluginRegistry);
}

std::vector<PluginManager::Handle> PluginManager::enumerateImportPlugins() noexcept
{
    return enumeratePlugins(ImportDirectoryName, d->importPluginRegistry);
}

bool PluginManager::importData(const QUuid pluginUuid, FlightService &flightService) const noexcept
{
    bool ok;
    if (d->importPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath = d->importPluginRegistry.value(pluginUuid);
        QPluginLoader loader(pluginPath);
        QObject *plugin = loader.instance();
        ImportIntf *importPlugin = qobject_cast<ImportIntf *>(plugin);
        if (importPlugin != nullptr) {
            ok = importPlugin->importData(flightService);
        } else {
            ok = false;
        }
        loader.unload();
    } else {
        ok = false;
    }
    return ok;
}

bool PluginManager::exportData(const QUuid pluginUuid) const noexcept
{
    bool ok;
    if (d->exportPluginRegistry.contains(pluginUuid)) {
        const QString pluginPath = d->exportPluginRegistry.value(pluginUuid);
        QPluginLoader loader(pluginPath);
        QObject *plugin = loader.instance();
        ExportIntf *exportPlugin = qobject_cast<ExportIntf *>(plugin);
        if (exportPlugin != nullptr) {
            ok = exportPlugin->exportData();
        } else {
            ok = false;
        }
        loader.unload();
    } else {
        ok = false;
    }
    return ok;
}

// PROTECTED

PluginManager::~PluginManager() noexcept
{
#ifdef DEBUG
    qDebug("PluginManager::~PluginManager: DELETED");
#endif
}

// PRIVATE

PluginManager::PluginManager() noexcept
    : d(std::make_unique<PluginManagerPrivate>())
{
#ifdef DEBUG
    qDebug("PluginManager::PluginManager: CREATED");
#endif
}

std::vector<PluginManager::Handle> PluginManager::enumeratePlugins(const QString &pluginDirectoryName, QMap<QUuid, QString> &pluginRegistry) noexcept
{
    std::vector<PluginManager::Handle> pluginHandles;
    pluginRegistry.clear();
    if (d->pluginsDirectoryPath.exists(pluginDirectoryName)) {
        d->pluginsDirectoryPath.cd(pluginDirectoryName);
        const QStringList entryList = d->pluginsDirectoryPath.entryList(QDir::Files);
        for (const QString &fileName : entryList) {
            const QString pluginPath = d->pluginsDirectoryPath.absoluteFilePath(fileName);
            QPluginLoader loader(pluginPath);

            QJsonObject metaData = loader.metaData();
            if (!metaData.isEmpty()) {
                QJsonObject pluginMetaData = metaData.value("MetaData").toObject();
                QUuid pluginUuid = pluginMetaData.value(PluginUuidKey).toString();
                QString pluginName = pluginMetaData.value(PluginNameKey).toString();
                Handle handle = {pluginUuid, pluginName};
                pluginHandles.push_back(handle);
                pluginRegistry.insert(pluginUuid, pluginPath);
            }
        }
        d->pluginsDirectoryPath.cdUp();
    }

    return pluginHandles;
}
