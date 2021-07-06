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

#include "ExportIntf.h"
#include "PluginManager.h"

namespace
{
    constexpr char ExportDirectoryName[] = "export";
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
    QMap<QUuid, QString> exportPlugins;
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

std::vector<PluginManager::Handle> PluginManager::enumerateExportPlugins() const noexcept
{
    std::vector<PluginManager::Handle> exportPlugins;
    d->exportPlugins.clear();
    if (d->pluginsDirectoryPath.exists(ExportDirectoryName)) {
        d->pluginsDirectoryPath.cd(ExportDirectoryName);
        const QStringList entryList = d->pluginsDirectoryPath.entryList(QDir::Files);
        for (const QString &fileName : entryList) {
            QPluginLoader loader(d->pluginsDirectoryPath.absoluteFilePath(fileName));

            QJsonObject metaData = loader.metaData();
            if (!metaData.isEmpty()) {
                QUuid pluginUuid = metaData.value(PluginUuidKey).toString();
                QString pluginName = metaData.value(PluginNameKey).toString();
                Handle handle = {pluginUuid, pluginName};
                exportPlugins.push_back(handle);
                d->exportPlugins.insert(pluginUuid, pluginName);
            }
        }

        d->pluginsDirectoryPath.cdUp();
    }

    return exportPlugins;
}

bool PluginManager::exportData(const QUuid pluginUuid) const noexcept
{
    bool ok;
    if (d->exportPlugins.contains(pluginUuid)) {
        const QString pluginPath = d->exportPlugins.value(pluginUuid);
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
{}

// PRIVATE

PluginManager::PluginManager() noexcept
    : d(std::make_unique<PluginManagerPrivate>())
{

}
