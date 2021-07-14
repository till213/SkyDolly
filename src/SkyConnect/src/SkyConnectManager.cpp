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

#include "../../Kernel/src/Settings.h"
#include "SkyConnectIntf.h"
#include "SkyConnectManager.h"

namespace
{
    constexpr char ConnectPluginDirectoryName[] = "Connect";
#if defined(Q_OS_MAC)
    constexpr char PluginDirectoryName[] = "PlugIns";
#else
    constexpr char PluginDirectoryName[] = "Plugins";
#endif
    constexpr char PluginUuidKey[] = "uuid";
    constexpr char PluginNameKey[] = "name";
    constexpr char PluginFlightSimulatorNameKey[] = "flightSimulator";
}

class skyConnectManagerPrivate
{
public:

    skyConnectManagerPrivate(QObject *parent) noexcept
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

    ~skyConnectManagerPrivate() noexcept
    {
        pluginLoader->unload();
    }

    QDir pluginsDirectoryPath;
    // Plugin UUID / plugin path
    QMap<QUuid, QString> pluginRegistry;
    std::vector<SkyConnectManager::Handle> pluginHandles;
    QPluginLoader *pluginLoader;

    static SkyConnectManager *instance;
};

SkyConnectManager *skyConnectManagerPrivate::instance = nullptr;

// PUBLIC

SkyConnectManager &SkyConnectManager::getInstance() noexcept
{
    if (skyConnectManagerPrivate::instance == nullptr) {
        skyConnectManagerPrivate::instance = new SkyConnectManager();
    }
    return *skyConnectManagerPrivate::instance;
}

void SkyConnectManager::destroyInstance() noexcept
{
    if (skyConnectManagerPrivate::instance != nullptr) {
        delete skyConnectManagerPrivate::instance;
        skyConnectManagerPrivate::instance = nullptr;
    }
}

const std::vector<SkyConnectManager::Handle> &SkyConnectManager::initialisePlugins() noexcept
{
    initialisePlugins(ConnectPluginDirectoryName);
    return availablePlugins();
}

const std::vector<SkyConnectManager::Handle> &SkyConnectManager::availablePlugins() const noexcept
{
    return d->pluginHandles;
}

std::optional<std::reference_wrapper<SkyConnectIntf>> SkyConnectManager::getCurrentSkyConnect() const noexcept
{
    QObject *plugin = d->pluginLoader->instance();
    if (plugin != nullptr) {
        return std::optional<std::reference_wrapper<SkyConnectIntf>>{*(static_cast<SkyConnectIntf *>(plugin))};
    } else {
        return {};
    }
}

bool SkyConnectManager::tryAndSetCurrentSkyConnect(const QUuid &uuid) noexcept
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
                    this, &SkyConnectManager::timestampChanged);
            connect(skyPlugin, &SkyConnectIntf::stateChanged,
                    this, &SkyConnectManager::stateChanged);
            connect(skyPlugin, &SkyConnectIntf::recordingStopped,
                    this, &SkyConnectManager::recordingStopped);
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

SkyConnectManager::~SkyConnectManager() noexcept
{
#ifdef DEBUG
    qDebug("SkyConnectManager::~SkyConnectManager: DELETED");
#endif
}

// PRIVATE

SkyConnectManager::SkyConnectManager() noexcept
    : d(std::make_unique<skyConnectManagerPrivate>(this))
{
    frenchConnection();
#ifdef DEBUG
    qDebug("SkyConnectManager::SkyConnectManager: CREATED");
#endif
}

void SkyConnectManager::frenchConnection() noexcept
{
    connect(&Settings::getInstance(), &Settings::skyConnectPluginUuidChanged,
            this, &SkyConnectManager::tryAndSetCurrentSkyConnect);
}

void SkyConnectManager::initialisePlugins(const QString &pluginDirectoryName) noexcept
{
    d->pluginRegistry.clear();
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
                const QString flightSimulatorName = pluginMetaData.value(PluginFlightSimulatorNameKey).toString();
                const FlightSimulator::Id flightSimulatorId = FlightSimulator::nameToId(flightSimulatorName);
                SkyConnectPlugin plugin = {pluginName, flightSimulatorId};
                const Handle handle = {uuid, plugin};
                d->pluginHandles.push_back(handle);
                d->pluginRegistry.insert(uuid, pluginPath);
            }
        }
        d->pluginsDirectoryPath.cdUp();
    }
}
