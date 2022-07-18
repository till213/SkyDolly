/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <optional>

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

#include <Kernel/Settings.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
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

struct skyConnectManagerPrivate
{
    skyConnectManagerPrivate(QObject *parent) noexcept
        : pluginLoader(new QPluginLoader(parent))
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

    ~skyConnectManagerPrivate() noexcept
    {
        pluginLoader->unload();
    }

    QDir pluginsDirectory;
    // Plugin UUID / plugin path
    QMap<QUuid, QString> pluginRegistry;
    std::vector<SkyConnectManager::Handle> pluginHandles;
    QPluginLoader *pluginLoader;
    QUuid currentPluginUuid;

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

bool SkyConnectManager::hasPlugins() const noexcept
{
    return d->pluginRegistry.size() > 0;
}

std::optional<std::reference_wrapper<SkyConnectIntf>> SkyConnectManager::getCurrentSkyConnect() const noexcept
{
    QObject *plugin = d->pluginLoader->instance();
    if (plugin != nullptr) {
        return std::optional<std::reference_wrapper<SkyConnectIntf>>{*(dynamic_cast<SkyConnectIntf *>(plugin))};
    } else {
        return {};
    }
}

std::optional<QString> SkyConnectManager::getCurrentSkyConnectPluginName() const noexcept
{
    if (d->currentPluginUuid != QUuid()) {
        for (auto &handle : d->pluginHandles) {
            if (d->currentPluginUuid == handle.first) {
                return std::optional<QString>{handle.second.name};
            }
        }
    }
    return {};
}

bool SkyConnectManager::setUserAircraftInitialPosition(const InitialPosition &initialPosition) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().setUserAircraftInitialPosition(initialPosition) : false;
}

bool SkyConnectManager::setUserAircraftPosition(const PositionData & positionData) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().setUserAircraftPosition(positionData) : false;
}

bool SkyConnectManager::freezeUserAircraft(bool enable) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().freezeUserAircraft(enable) : false;
}

SkyConnectIntf::ReplayMode SkyConnectManager::getReplayMode() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().getReplayMode() : SkyConnectIntf::ReplayMode::Normal;
}

void SkyConnectManager::setReplayMode(SkyConnectIntf::ReplayMode replayMode) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().setReplayMode(replayMode);
    }
}

void SkyConnectManager::startRecording(SkyConnectIntf::RecordingMode recordingMode, const InitialPosition &initialPosition) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().startRecording(recordingMode, initialPosition);
    }
}

void SkyConnectManager::stopRecording() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().stopRecording();
    }
}

bool SkyConnectManager::isRecording() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isRecording() : false;
}

bool SkyConnectManager::isInRecordingState() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isInRecordingState() : false;
}

void SkyConnectManager::startReplay(bool fromStart, const InitialPosition &flyWithFormationPosition) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().startReplay(fromStart, flyWithFormationPosition);
    }
}

void SkyConnectManager::stopReplay() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().stopReplay();
    }
}

bool SkyConnectManager::isReplaying() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isReplaying() : false;
}

bool SkyConnectManager::isInReplayState() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isInReplayState() : false;
}

bool SkyConnectManager::isActive() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isInRecordingState() || skyConnect->get().isInReplayState(): false;
}

void SkyConnectManager::stop() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().stop();
    }
}

void SkyConnectManager::setPaused(bool enable) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().setPaused(enable);
    }
}

bool SkyConnectManager::isPaused() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isPaused() : false;
}

void SkyConnectManager::skipToBegin() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipToBegin();
    }
}

void SkyConnectManager::skipBackward() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipBackward();
    }
}

void SkyConnectManager::skipForward() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipForward();
    }
}

void SkyConnectManager::skipToEnd() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipToEnd();
    }
}

void SkyConnectManager::seek(std::int64_t timestamp) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().seek(timestamp);
    }
}

Connect::State SkyConnectManager::getState() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().getState() : Connect::State::Disconnected;
}

bool SkyConnectManager::isConnected() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isConnected() : false;
}

bool SkyConnectManager::isIdle() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isIdle() : true;
}

double SkyConnectManager::getReplaySpeedFactor() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().getReplaySpeedFactor() : 1.0;
}

void SkyConnectManager::setReplaySpeedFactor(double factor) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().setReplaySpeedFactor(factor);
    }
}

std::int64_t SkyConnectManager::getCurrentTimestamp() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().getCurrentTimestamp() : 0;
}

bool SkyConnectManager::isAtEnd() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isAtEnd() : false;
}

bool SkyConnectManager::requestInitialPosition() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().requestLocation() : false;
}

// PUBLIC SLOTS

bool SkyConnectManager::tryAndSetCurrentSkyConnect(const QUuid &uuid) noexcept
{
    bool ok {false};
    d->currentPluginUuid = QUuid();
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
            connect(skyPlugin, &SkyConnectIntf::replayModeChanged,
                    this, &SkyConnectManager::replayModeChanged);
            connect(skyPlugin, &SkyConnectIntf::recordingStarted,
                    this, &SkyConnectManager::recordingStarted);
            connect(skyPlugin, &SkyConnectIntf::recordingStopped,
                    this, &SkyConnectManager::recordingStopped);
            connect(skyPlugin, &SkyConnectIntf::locationReceived,
                    this, &SkyConnectManager::locationReceived);

            // Flight
            const Logbook &logbook = Logbook::getInstance();
            const Flight &flight = logbook.getCurrentFlight();
            connect(&flight, &Flight::flightRestored,
                    skyPlugin, &SkyConnectIntf::syncAiObjectsWithFlight);
            connect(&flight, &Flight::cleared,
                    skyPlugin, &SkyConnectIntf::removeAiObjects);
            connect(&flight, &Flight::aircraftAdded,
                    skyPlugin, &SkyConnectIntf::addAiObject);
            connect(&flight, &Flight::aircraftRemoved,
                    skyPlugin, &SkyConnectIntf::removeAiObject);
            connect(&flight, &Flight::userAircraftChanged,
                    skyPlugin, &SkyConnectIntf::updateUserAircraft);
            connect(&flight, &Flight::timeOffsetChanged,
                    skyPlugin, &SkyConnectIntf::onTimeOffsetChanged);
            connect(&flight, &Flight::tailNumberChanged,
                    skyPlugin, &SkyConnectIntf::onTailNumberChanged);
            d->currentPluginUuid = uuid;
            ok = true;
        } else {
            // Not a valid SkyConnect plugin
            d->pluginLoader->unload();
            ok = false;
        }
    }
    return ok;
}

// PROTECTED

SkyConnectManager::~SkyConnectManager() noexcept
{
#ifdef DEBUG
    qDebug() << "SkyConnectManager::~SkyConnectManager: DELETED";
#endif
}

// PRIVATE

SkyConnectManager::SkyConnectManager() noexcept
    : d(std::make_unique<skyConnectManagerPrivate>(this))
{
    frenchConnection();
#ifdef DEBUG
    qDebug() << "SkyConnectManager::SkyConnectManager: CREATED";
#endif
}

void SkyConnectManager::frenchConnection() noexcept
{
    // Settings
    Settings &settings = Settings::getInstance();
    connect(&settings, &Settings::skyConnectPluginUuidChanged,
            this, &SkyConnectManager::tryAndSetCurrentSkyConnect);  
}

void SkyConnectManager::initialisePlugins(const QString &pluginDirectoryName) noexcept
{
    d->pluginRegistry.clear();
    if (d->pluginsDirectory.exists(pluginDirectoryName)) {
        d->pluginsDirectory.cd(pluginDirectoryName);
        const QStringList entryList = d->pluginsDirectory.entryList(QDir::Files);
        d->pluginHandles.reserve(entryList.count());
        for (const QString &fileName : entryList) {
            const QString pluginPath = d->pluginsDirectory.absoluteFilePath(fileName);
            QPluginLoader loader(pluginPath);

            const QJsonObject metaData = loader.metaData();
            if (!metaData.isEmpty()) {
                const QJsonObject pluginMetadata = metaData.value("MetaData").toObject();
                const QUuid uuid = pluginMetadata.value(PluginUuidKey).toString();
                const QString pluginName = pluginMetadata.value(PluginNameKey).toString();
                const QString flightSimulatorName = pluginMetadata.value(PluginFlightSimulatorNameKey).toString();
                const FlightSimulator::Id flightSimulatorId = FlightSimulator::nameToId(flightSimulatorName);
                SkyConnectPlugin plugin = {pluginName, flightSimulatorId};
                const Handle handle = {uuid, plugin};
                d->pluginHandles.push_back(handle);
                d->pluginRegistry.insert(uuid, pluginPath);
            }
        }
        d->pluginsDirectory.cdUp();
    }
}
