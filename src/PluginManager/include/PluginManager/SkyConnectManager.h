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
#ifndef SKYCONNECTMANAGER_H
#define SKYCONNECTMANAGER_H

#include <memory>
#include <vector>
#include <utility>
#include <optional>
#include <cstdint>
#include <unordered_map>

#include <QObject>
#include <QDateTime>

class QString;
class QUuid;

#include <Kernel/Settings.h>
#include <Kernel/FlightSimulator.h>
#include <Kernel/QUuidHasher.h>
#include <Model/TimeVariableData.h>
#include <Model/InitialPosition.h>
#include <Model/Location.h>
#include <Model/TimeZoneInfo.h>
#include "Connect/Connect.h"
#include "Connect/SkyConnectIntf.h"
#include "OptionWidgetIntf.h"
#include "PluginManagerLib.h"

struct FlightSimulatorShortcuts;
struct SkyConnectManagerPrivate;
struct PositionData;
struct AttitudeData;

/// \todo Gradually implement all methods from the SkyConnectIntf and then finally inherit from it
class PLUGINMANAGER_API SkyConnectManager final : public QObject
{
    Q_OBJECT
public:
    SkyConnectManager(const SkyConnectManager &rhs) = delete;
    SkyConnectManager(SkyConnectManager &&rhs) = delete;
    SkyConnectManager &operator=(const SkyConnectManager &rhs) = delete;
    SkyConnectManager &operator=(SkyConnectManager &&rhs) = delete;

    static SkyConnectManager &getInstance() noexcept;
    static void destroyInstance() noexcept;

    /*!
     * The plugin name and the flight simulator it supports.
     */
    using SkyConnectPlugin = struct {
        QString name;
        FlightSimulator::Id flightSimulatorId;
    };

    /*!
     * The plugin UUID and the plugin name and capabilities (flight simulator).
     */
    using Handle = std::pair<QUuid, SkyConnectPlugin>;

    /*!
     * \brief initialisePlugins
     * \return
     * \sa connectionChanged
     */
    const std::vector<Handle> &initialisePlugins() noexcept;
    const std::vector<Handle> &availablePlugins() const noexcept;
    bool hasPlugins() const noexcept;

    /*!
     * Stores the settings of the currently loaded plugin (if any).
     */
    void storeSettings() const noexcept;

    /*!
     * Restores the settings of the currently loaded plugin (if any).
     */
    void restoreSettings() const noexcept;

    /*!
     * Returns the current connection.
     *
     * \return the current connection, if any plugin is available
     * \sa tryAndSetCurrentSkyConnect
     */
    std::optional<std::reference_wrapper<SkyConnectIntf>> getCurrentSkyConnect() const noexcept;
    std::optional<QString> getCurrentSkyConnectPluginName() const noexcept;

    /*!
     * Creates the option widget (if any).
     *
     * \return the optional option widget; no value if no connect plugin is loaded,
     *         or the connect plugin does not have specific options
     */
    std::optional<std::unique_ptr<OptionWidgetIntf>> createOptionWidget() const noexcept;

    /*!
     * Tries to connect with the flight simulator and to setup the shortcuts. If the connection
     * fails the SkyConnectManager will periodically retry to connect.
     *
     * This method can be repeatedly called, in case connect plugin-specific settings such as
     * shortcuts have changed.
     */
    void tryConnectAndSetup() noexcept;

    int getRemainingReconnectTime() const noexcept;

    bool setUserAircraftInitialPosition(const InitialPosition &initialPosition) noexcept;
    bool setUserAircraftPositionAndAttitude(const PositionData &positionData, const AttitudeData &attitudeData) noexcept;
    bool freezeUserAircraft(bool enable) noexcept;
    bool sendSimulationEvent(SkyConnectIntf::SimulationEvent event, float arg1 = 0.0f) noexcept;

    SkyConnectIntf::ReplayMode getReplayMode() const noexcept;
    void setReplayMode(SkyConnectIntf::ReplayMode replayMode) noexcept;

    void startRecording(SkyConnectIntf::RecordingMode recordingMode, const InitialPosition &initialPosition = InitialPosition()) noexcept;
    void stopRecording() noexcept;
    bool isRecording() const noexcept;
    bool isInRecordingState() const noexcept;

    void startReplay(bool skipToStart, const InitialPosition &initialPosition = InitialPosition()) noexcept;
    void stopReplay() noexcept;
    bool isReplaying() const noexcept;
    bool isInReplayState() const noexcept;

    /*!
     * Returns \c true in case the SkyConnect connection is \e active, that is either
     * a replay or recording (including paused states) is taking place.
     *
     * \return \c true if the SkyConnect connection is \e active; \c false else
     * \sa isInRecordingState
     * \sa isInReplayState
     */
    bool isActive() const noexcept;

    void stop() noexcept;

    void setPaused(SkyConnectIntf::Initiator initiator, bool enable) noexcept;
    bool isPaused() const noexcept;
    bool isRecordingPaused() const noexcept;

    void skipToBegin() noexcept;
    void skipBackward() noexcept;
    void skipForward() noexcept;
    void skipToEnd() noexcept;
    void seek(std::int64_t timestamp, SkyConnectIntf::SeekMode seekMode) noexcept;

    Connect::State getState() const noexcept;
    virtual bool isConnected() const noexcept;
    virtual bool isIdle() const noexcept;

    float getReplaySpeedFactor() const noexcept;
    void setReplaySpeedFactor(float factor) noexcept;
    std::int64_t getCurrentTimestamp() const noexcept;
    bool isAtEnd() const noexcept;

    bool requestLocation() const noexcept;
    bool requestSimulationRate() const noexcept;
    bool requestTimeZoneInfo() const noexcept;
    bool sendZuluDateTime(QDateTime dateTime) const noexcept;

    using PluginRegistry = std::unordered_map<QUuid, QString, QUuidHasher>;

public slots:
    bool tryAndSetCurrentSkyConnect(const QUuid &uuid) noexcept;

signals:
    /*!
     * Emitted whenver the connection plugin has changed.
     *
     * \param skyConnect
     *        the current skyConnect plugin
     */
    void connectionChanged(SkyConnectIntf *skyConnect);

    /*!
     * Relay of the SkyConnectIntf#timestampChanged signal.
     *
     * \param timestamp
     *        the current timestamp
     * \param access
     *        the way the current timestamp was accessed
     * \sa SkyConnectIntf#timestampChanged
     */
    void timestampChanged(std::int64_t timestamp, TimeVariableData::Access access);

    /*!
     * Relay of the SkyConnectIntf#stateChanged signal.
     *
     * \param state
     *        the current connection state
     * \sa SkyConnectIntf#stateChanged
     */
    void stateChanged(Connect::State state);

    /*!
     * Relay of the SkyConnectIntf#replayModeChanged signal.
     *
     * \param replayMode
     *        the current replay mode
     * \sa SkyConnectIntf#replayModeChanged
     */
    void replayModeChanged(SkyConnectIntf::ReplayMode replayMode);

    /*!
     * Relay of the SkyConnectIntf#recordingStarted signal.
     *
     * \sa SkyConnectIntf#recordingStarted
     */
    void recordingStarted();

    /*!
     * Relay of the SkyConnectIntf#recordingStopped signal.
     *
     * \sa SkyConnectIntf#recordingStopped
     */
    void recordingStopped();

    /*!
     * Relay of the SkyConnectIntf#locationReceived signal.
     *
     * \sa SkyConnectIntf#locationReceived
     */
    void locationReceived(Location location);

    /*!
     * Relay of the SkyConnectIntf#timeZoneInfoReceived signal.
     *
     * \sa SkyConnectIntf#timeZoneInfoReceived
     */
    void timeZoneInfoReceived(TimeZoneInfo timeZoneInfo);

    /*!
     * Relay of the SkyConnectIntf#simulationRateReceived signal.
     *
     * \sa SkyConnectIntf#simulationRateReceived
     */
    void simulationRateReceived(float rate);

    /*!
     * Relay of the SkyConnectIntf#actionActivated signal.
     *
     * \sa SkyConnectIntf#actionActivated
     */
    void actionActivated(FlightSimulatorShortcuts::Action action);

private:
    const std::unique_ptr<SkyConnectManagerPrivate> d;

    SkyConnectManager() noexcept;
    friend std::unique_ptr<SkyConnectManager>::deleter_type;
    ~SkyConnectManager() override;

    void frenchConnection() noexcept;
    void initialisePluginRegistry(const QString &pluginDirectoryName) noexcept;
    void initialisePlugin() noexcept;
    void unloadCurrentPlugin() noexcept;
};

#endif // SKYCONNECTMANAGER_H
