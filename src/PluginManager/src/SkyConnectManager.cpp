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
#include <optional>
#include <mutex>

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
#include <Kernel/Settings.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/PositionData.h>
#include <Model/AttitudeData.h>
#include <Connect/SkyConnectIntf.h>
#include <Connect/FlightSimulatorShortcuts.h>
#include <SkyConnectManager.h>

namespace
{
    constexpr const char *ConnectPluginDirectoryName {"Connect"};
    constexpr const char *PluginUuidKey {"uuid"};
    constexpr const char *PluginNameKey {"name"};
    constexpr const char *PluginFlightSimulatorNameKey {"flightSimulator"};
}

struct SkyConnectManagerPrivate
{
    SkyConnectManagerPrivate() noexcept
    {
        pluginsDirectory.cd(File::getPluginDirectoryPath());
    }

    ~SkyConnectManagerPrivate() noexcept
    {
        pluginLoader->unload();
    }

    QDir pluginsDirectory;
    // Key: uuid - value: plugin path
    SkyConnectManager::PluginRegistry pluginRegistry;
    std::vector<SkyConnectManager::Handle> pluginHandles;
    std::unique_ptr<QPluginLoader> pluginLoader {std::make_unique<QPluginLoader>()};
    QUuid currentPluginUuid;

    static inline std::once_flag onceFlag;
    static inline std::unique_ptr<SkyConnectManager> instance;
};

// PUBLIC

SkyConnectManager &SkyConnectManager::getInstance() noexcept
{
    std::call_once(SkyConnectManagerPrivate::onceFlag, []() {
        SkyConnectManagerPrivate::instance = std::unique_ptr<SkyConnectManager>(new SkyConnectManager());
    });
    return *SkyConnectManagerPrivate::instance;
}

void SkyConnectManager::destroyInstance() noexcept
{
    if (SkyConnectManagerPrivate::instance != nullptr) {
        SkyConnectManagerPrivate::instance.reset();
    }
}

const std::vector<SkyConnectManager::Handle> &SkyConnectManager::initialisePlugins() noexcept
{
    initialisePluginRegistry(::ConnectPluginDirectoryName);
    initialisePlugin();
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

void SkyConnectManager::storeSettings() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().storeSettings(d->currentPluginUuid);
    }
}

void SkyConnectManager::restoreSettings() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().restoreSettings(d->currentPluginUuid);
    }
}

std::optional<std::reference_wrapper<SkyConnectIntf>> SkyConnectManager::getCurrentSkyConnect() const noexcept
{
    QObject *plugin = d->pluginLoader->instance();
    if (plugin != nullptr) {
        return std::optional<std::reference_wrapper<SkyConnectIntf>>{*(qobject_cast<SkyConnectIntf *>(plugin))};
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

std::optional<std::unique_ptr<OptionWidgetIntf>> SkyConnectManager::createOptionWidget() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        return skyConnect->get().createOptionWidget();
    }
    return {};
}

void SkyConnectManager::tryConnectAndSetup() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().tryConnectAndSetup();
    }
}

int SkyConnectManager::getRemainingReconnectTime() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        return skyConnect->get().getRemainingReconnectTime();
    } else {
        return -1;
    }
}

bool SkyConnectManager::setUserAircraftInitialPosition(const InitialPosition &initialPosition) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().setUserAircraftInitialPosition(initialPosition) : false;
}

bool SkyConnectManager::setUserAircraftPositionAndAttitude(const PositionData &positionData, const AttitudeData &attitudeData) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().setUserAircraftPositionAndAttitude(positionData, attitudeData) : false;
}

bool SkyConnectManager::freezeUserAircraft(bool enable) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().freezeUserAircraft(enable) : false;
}

bool SkyConnectManager::sendSimulationEvent(SkyConnectIntf::SimulationEvent event, float arg1) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().sendSimulationEvent(event, arg1) : false;
}

SkyConnectIntf::ReplayMode SkyConnectManager::getReplayMode() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().getReplayMode() : SkyConnectIntf::ReplayMode::Normal;
}

void SkyConnectManager::setReplayMode(SkyConnectIntf::ReplayMode replayMode) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().setReplayMode(replayMode);
    }
}

void SkyConnectManager::startRecording(SkyConnectIntf::RecordingMode recordingMode, const InitialPosition &initialPosition) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().startRecording(recordingMode, initialPosition);
    }
}

void SkyConnectManager::stopRecording() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
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

void SkyConnectManager::startReplay(bool skipToStart, const InitialPosition &initialPosition) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().startReplay(skipToStart, initialPosition);
    }
}

void SkyConnectManager::stopReplay() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().stopReplay();
    }
}

bool SkyConnectManager::isReplaying() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isReplaying() : false;
}

bool SkyConnectManager::isInReplayState() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isInReplayState() : false;
}

bool SkyConnectManager::isActive() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isActive() : false;
}

void SkyConnectManager::stop() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().stop();
    }
}

void SkyConnectManager::setPaused(SkyConnectIntf::Initiator initiator, bool enable) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().setPaused(initiator, enable);
    }
}

bool SkyConnectManager::isPaused() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isPaused() : false;
}

bool SkyConnectManager::isRecordingPaused() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isRecordingPaused() : false;
}

void SkyConnectManager::skipToBegin() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipToBegin();
    }
}

void SkyConnectManager::skipBackward() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipBackward();
    }
}

void SkyConnectManager::skipForward() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipForward();
    }
}

void SkyConnectManager::skipToEnd() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipToEnd();
    }
}

void SkyConnectManager::seek(std::int64_t timestamp, SkyConnectIntf::SeekMode seekMode) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().seek(timestamp, seekMode);
    }
}

Connect::State SkyConnectManager::getState() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().getState() : Connect::State::Disconnected;
}

bool SkyConnectManager::isConnected() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isConnected() : false;
}

bool SkyConnectManager::isIdle() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().isIdle() : true;
}

float SkyConnectManager::getReplaySpeedFactor() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().getReplaySpeedFactor() : 1.0f;
}

void SkyConnectManager::setReplaySpeedFactor(float factor) noexcept
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
    return skyConnect ? skyConnect->get().isEndReached() : false;
}

bool SkyConnectManager::requestLocation() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().requestLocation() : false;
}

bool SkyConnectManager::requestSimulationRate() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().requestSimulationRate() : false;
}

bool SkyConnectManager::requestTimeZoneInfo() const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().requestTimeZoneInfo() : false;
}

bool SkyConnectManager::sendZuluDateTime(QDateTime dateTime) const noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = getCurrentSkyConnect();
    return skyConnect ? skyConnect->get().sendZuluDateTime(std::move(dateTime)) : false;
}

// PUBLIC SLOTS

bool SkyConnectManager::tryAndSetCurrentSkyConnect(const QUuid &uuid) noexcept
{
    bool ok {true};

    if (d->currentPluginUuid != uuid) {

        if (d->pluginRegistry.contains(uuid)) {

            if (getCurrentSkyConnect().has_value()) {
                unloadCurrentPlugin();
            }

            const QString pluginPath = d->pluginRegistry[uuid];
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
                connect(skyPlugin, &SkyConnectIntf::timeZoneInfoReceived,
                        this, &SkyConnectManager::timeZoneInfoReceived);
                connect(skyPlugin, &SkyConnectIntf::simulationRateReceived,
                        this, &SkyConnectManager::simulationRateReceived);
                connect(skyPlugin, &SkyConnectIntf::actionActivated,
                        this, &SkyConnectManager::actionActivated);

                // Flight
                const auto &logbook = Logbook::getInstance();
                const auto &flight = logbook.getCurrentFlight();
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
                restoreSettings();
                emit connectionChanged(skyPlugin);
                ok = true;
            } else {
                // Not a valid SkyConnect plugin
                d->pluginLoader->unload();
                ok = false;
            }
            Settings::getInstance().setSkyConnectPluginUuid(d->currentPluginUuid);
        } else {
            // Invalid uuid: no such plugin in registry
            ok = false;
        }
    }
    return ok;
}

// PRIVATE

SkyConnectManager::SkyConnectManager() noexcept
    : d {std::make_unique<SkyConnectManagerPrivate>()}
{
    frenchConnection();
}

SkyConnectManager::~SkyConnectManager()
{
#ifdef DEBUG
    qDebug() << "SkyConnectManager::~SkyConnectManager: DELETED";
#endif
    if (getCurrentSkyConnect().has_value()) {
        // Unload the current plugin
        unloadCurrentPlugin();
    }
}

void SkyConnectManager::frenchConnection() noexcept
{
    // Settings
    const auto &settings = Settings::getInstance();
    connect(&settings, &Settings::skyConnectPluginUuidChanged,
            this, &SkyConnectManager::tryAndSetCurrentSkyConnect);
}

void SkyConnectManager::initialisePluginRegistry(const QString &pluginDirectoryName) noexcept
{
    d->pluginRegistry.clear();
    if (d->pluginsDirectory.exists(pluginDirectoryName)) {
        d->pluginsDirectory.cd(pluginDirectoryName);
        const QStringList entryList = d->pluginsDirectory.entryList(QDir::Files);
        d->pluginHandles.reserve(entryList.count());
        for (const auto &fileName : entryList) {
            const QString pluginPath = d->pluginsDirectory.absoluteFilePath(fileName);
            d->pluginLoader->setFileName(pluginPath);

            const QJsonObject metaData = d->pluginLoader->metaData();
            if (!metaData.isEmpty()) {
                const QJsonObject pluginMetadata {metaData.value("MetaData").toObject()};
                const QUuid uuid {pluginMetadata.value(PluginUuidKey).toString()};
                const QString pluginName {pluginMetadata.value(PluginNameKey).toString()};
                const QString flightSimulatorName {pluginMetadata.value(PluginFlightSimulatorNameKey).toString()};
                const FlightSimulator::Id flightSimulatorId {FlightSimulator::nameToId(flightSimulatorName)};
                SkyConnectPlugin plugin {pluginName, flightSimulatorId};
                const Handle handle {uuid, plugin};
                d->pluginHandles.push_back(handle);
                d->pluginRegistry[uuid] =pluginPath;
            }
        }
        d->pluginsDirectory.cdUp();
    }
}

void SkyConnectManager::initialisePlugin() noexcept
{
    const auto &settings = Settings::getInstance();
    QUuid uuid = settings.getSkyConnectPluginUuid();
    // Try to load plugin as stored in the settings
    bool ok = !uuid.isNull() && tryAndSetCurrentSkyConnect(uuid);
    if (!ok) {
        // First attempt or not a valid plugin (anymore), so try the other plugins
        if (d->pluginHandles.size() == 1) {
            // There is only one plugin
            uuid = d->pluginHandles.front().first;
            tryAndSetCurrentSkyConnect(uuid);
        } else if (d->pluginHandles.size() > 1) {
            // Check if an actual flight simulator instance is running
            for (auto &plugin : d->pluginHandles) {
                if (FlightSimulator::isRunning(plugin.second.flightSimulatorId)) {
                    uuid = plugin.first;
                    ok = tryAndSetCurrentSkyConnect(uuid);
                }
                if (ok) {
                    break;
                }
            }
            if (!ok) {
                // No instance running (or no valid plugin), so try again and
                // check if the given flight simulator is installed
                for (auto &plugin : d->pluginHandles) {
                    if (FlightSimulator::isInstalled(plugin.second.flightSimulatorId)) {
                        uuid = plugin.first;
                        ok = tryAndSetCurrentSkyConnect(uuid);
                    }
                    if (ok) {
                        break;
                    }
                }
            }
            if (!ok) {
                // Default to the Flight Simulator 2020 plugin
                for (auto &plugin : d->pluginHandles) {
                    if (plugin.second.flightSimulatorId == FlightSimulator::Id::MSFS) {
                        uuid = plugin.first;
                        ok = tryAndSetCurrentSkyConnect(uuid);
                    }
                    if (ok) {
                        break;
                    }
                }
            }
            if (!ok) {
                // Everything failed, so as a last resort try with a generic ("All") plugin
                for (auto &plugin : d->pluginHandles) {
                    if (plugin.second.flightSimulatorId == FlightSimulator::Id::All) {
                        uuid = plugin.first;
                        ok = tryAndSetCurrentSkyConnect(uuid);
                    }
                    if (ok) {
                        break;
                    }
                }
            }
        }
    }
}

void SkyConnectManager::unloadCurrentPlugin() noexcept
{
    storeSettings();
    d->pluginLoader->unload();
    d->currentPluginUuid = QUuid();
}
