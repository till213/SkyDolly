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
#include <QDir>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QMap>

#include "SkyConnectIntf.h"
#include "SkyManager.h"

namespace
{
    constexpr char ConnectPluginDirectoryName[] = "connect";
#if defined(Q_OS_MAC)
    constexpr char PluginDirectoryName[] = "PlugIns";
#else
    constexpr char PluginDirectoryName[] = "plugins";
#endif
     constexpr char PluginUuidKey[] = "uuid";
     constexpr char PluginNameKey[] = "name";
}

class SkyManagerPrivate
{
public:

    SkyManagerPrivate(QObject *parent) noexcept
        : pluginLoader(new QPluginLoader(parent))
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

    ~SkyManagerPrivate() noexcept
    {}

    QDir pluginsDirectoryPath;
    // Plugin UUID / plugin path
    QMap<QUuid, QString> pluginRegistry;
    QPluginLoader *pluginLoader;

    static SkyManager *instance;
};

SkyManager *SkyManagerPrivate::instance = nullptr;

// PUBLIC

SkyManager &SkyManager::getInstance() noexcept
{
    if (SkyManagerPrivate::instance == nullptr) {
        SkyManagerPrivate::instance = new SkyManager();
    }
    return *SkyManagerPrivate::instance;
}

void SkyManager::destroyInstance() noexcept
{
    if (SkyManagerPrivate::instance != nullptr) {
        delete SkyManagerPrivate::instance;
        SkyManagerPrivate::instance = nullptr;
    }
}

std::vector<SkyManager::Handle> SkyManager::enumeratePlugins() noexcept
{
    return enumeratePlugins(ConnectPluginDirectoryName, d->pluginRegistry);
}

SkyConnectIntf *SkyManager::getCurrentSkyConnect() const noexcept
{
    QObject *plugin = d->pluginLoader->instance();
    return qobject_cast<SkyConnectIntf *>(plugin);
}

bool SkyManager::setCurrentSkyConnect(const QUuid &uuid) noexcept
{
    bool ok;
    if (d->pluginRegistry.contains(uuid)) {
        // Unload the previous plugin (if any)
        d->pluginLoader->unload();
        const QString pluginPath = d->pluginRegistry.value(uuid);
        d->pluginLoader->setFileName(pluginPath);
        QObject *plugin = d->pluginLoader->instance();
        SkyConnectIntf *skyPlugin = qobject_cast<SkyConnectIntf *>(plugin);
        if (skyPlugin != nullptr) {
            connect(skyPlugin, &SkyConnectIntf::timestampChanged,
                    this, &SkyManager::timestampChanged);
            connect(skyPlugin, &SkyConnectIntf::stateChanged,
                    this, &SkyManager::stateChanged);
            connect(skyPlugin, &SkyConnectIntf::recordingStopped,
                    this, &SkyManager::recordingStopped);
            ok = true;
        } else {
            // Not a valid SkyConnect plugin
            d->pluginLoader->unload();
            ok = false;
        }
    } else {
        ok = false;
    }
    return ok;
}

// PROTECTED

SkyManager::~SkyManager() noexcept
{
#ifdef DEBUG
    qDebug("SkyManager::~SkyManager: DELETED");
#endif
}

// PRIVATE

SkyManager::SkyManager() noexcept
    : d(std::make_unique<SkyManagerPrivate>(this))
{
#ifdef DEBUG
    qDebug("SkyManager::SkyManager: CREATED");
#endif
}

std::vector<SkyManager::Handle> SkyManager::enumeratePlugins(const QString &pluginDirectoryName, QMap<QUuid, QString> &pluginRegistry) noexcept
{
    std::vector<SkyManager::Handle> pluginHandles;
    pluginRegistry.clear();
    if (d->pluginsDirectoryPath.exists(pluginDirectoryName)) {
        d->pluginsDirectoryPath.cd(pluginDirectoryName);
        const QStringList entryList = d->pluginsDirectoryPath.entryList(QDir::Files);
        for (const QString &fileName : entryList) {
            const QString pluginPath = d->pluginsDirectoryPath.absoluteFilePath(fileName);
            QPluginLoader loader(pluginPath);

            const QJsonObject metaData = loader.metaData();
            if (!metaData.isEmpty()) {
                const QJsonObject pluginMetaData = metaData.value("MetaData").toObject();
                const QUuid uuid = pluginMetaData.value(PluginUuidKey).toString();
                const QString pluginName = pluginMetaData.value(PluginNameKey).toString();
                const Handle handle = {uuid, pluginName};
                pluginHandles.push_back(handle);
                pluginRegistry.insert(uuid, pluginPath);
            }
        }
        d->pluginsDirectoryPath.cdUp();
    }

    return pluginHandles;
}
