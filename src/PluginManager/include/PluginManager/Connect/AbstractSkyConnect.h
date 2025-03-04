/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef ABSTRACTSKYCONNECT_H
#define ABSTRACTSKYCONNECT_H

#include <memory>
#include <optional>
#include <cstdint>

#include <QObject>
#include <QDateTime>

class QWidget;

#include <Kernel/SampleRate.h>
#include <Kernel/Settings.h>
#include <Model/InitialPosition.h>
#include "SkyConnectIntf.h"
#include "ConnectPluginBaseSettings.h"
#include "Connect.h"
#include "../PluginBase.h"
#include "../OptionWidgetIntf.h"
#include "../PluginManagerLib.h"

class Flight;
class Aircraft;
struct FlightSimulatorShortcuts;
class ConnectPluginBaseSettings;
struct AbstractSkyConnectPrivate;

class PLUGINMANAGER_API AbstractSkyConnect : public SkyConnectIntf, public PluginBase
{
    Q_OBJECT
public:
    enum struct AircraftSelection: std::uint8_t {
        All,
        UserAircraft
    };

    AbstractSkyConnect(QObject *parent = nullptr) noexcept;
    AbstractSkyConnect(const AbstractSkyConnect &rhs) = delete;
    AbstractSkyConnect(AbstractSkyConnect &&rhs) = delete;
    AbstractSkyConnect &operator=(const AbstractSkyConnect &rhs) = delete;
    AbstractSkyConnect &operator=(AbstractSkyConnect &&rhs) = delete;
    ~AbstractSkyConnect() override;
    
    void tryConnectAndSetup() noexcept override;
    void disconnect() noexcept override;
    int getRemainingReconnectTime() const noexcept override;

    bool setUserAircraftInitialPosition(const InitialPosition &initialPosition) noexcept override;
    bool freezeUserAircraft(bool enable) const noexcept override;
    bool sendSimulationEvent(SimulationEvent event, float arg1) noexcept override;

    ReplayMode getReplayMode() const noexcept override;
    void setReplayMode(ReplayMode replayMode) noexcept override;

    void startRecording(RecordingMode recordingMode, const InitialPosition &initialPosition = InitialPosition()) noexcept override;
    void stopRecording() noexcept override;
    bool isRecording() const noexcept override;
    bool isInRecordingState() const noexcept override;

    void startReplay(bool skipToStart, const InitialPosition &initialPosition = InitialPosition()) noexcept override;
    void stopReplay() noexcept override;
    bool isReplaying() const noexcept override;
    bool isInReplayState() const noexcept override;

    void stop() noexcept override;

    void setPaused(Initiator initiator, bool enable) noexcept override;
    bool isPaused() const noexcept override;
    bool isRecordingPaused() const noexcept override;

    void skipToBegin() noexcept override;
    void skipBackward() noexcept override;
    void skipForward() noexcept override;
    void skipToEnd() noexcept override;
    void seek(std::int64_t timestamp, SeekMode seekMode) noexcept override;
    void onEndReached() noexcept override;

    Connect::State getState() const noexcept override;
    bool isConnected() const noexcept override;
    bool isIdle() const noexcept override;
    bool isActive() const noexcept override;

    std::int64_t getCurrentTimestamp() const noexcept override;
    bool isEndReached() const noexcept override;

    float getReplaySpeedFactor() const noexcept override;
    void setReplaySpeedFactor(float factor) noexcept override;

    bool requestLocation() noexcept override;
    bool requestSimulationRate() noexcept override;
    bool requestTimeZoneInfo() noexcept override;
    bool sendZuluDateTime(QDateTime dateTime) noexcept override;

    void storeSettings(const QUuid &pluginUuid) const noexcept final
    {
        PluginBase::storeSettings(pluginUuid);
    }

    void restoreSettings(const QUuid &pluginUuid) noexcept final
    {
        PluginBase::restoreSettings(pluginUuid);
    }

    std::optional<std::unique_ptr<OptionWidgetIntf>> createOptionWidget() const noexcept final;

public slots:
    void addAiObject(const Aircraft &aircraft) noexcept override;
    void removeAiObjects() noexcept override;
    void removeAiObject(std::int64_t removedAircraftId) noexcept override;
    void syncAiObjectsWithFlight() noexcept override;
    void updateUserAircraft(int newUserAircraftIndex, int previousUserAircraftIndex) noexcept override;
    void onTimeOffsetChanged() noexcept override;
    void onTailNumberChanged(const Aircraft &aircraft) noexcept override;

protected:
    void setState(Connect::State state) noexcept override;
    Flight &getCurrentFlight() const;
    void setCurrentTimestamp(std::int64_t timestamp) noexcept;

    bool isElapsedTimerRunning() const noexcept;   
    void startElapsedTimer() const noexcept;
    void resetElapsedTime(bool restart) noexcept;

    void createAiObjects() noexcept;

    // Re-implement
    virtual ConnectPluginBaseSettings &getPluginSettings() const noexcept = 0;
    virtual std::optional<std::unique_ptr<OptionWidgetIntf>> createExtendedOptionWidget() const noexcept = 0;
    virtual bool onSetupFlightSimulatorShortcuts() noexcept = 0;
    virtual bool onInitialPositionSetup(const InitialPosition &initialPosition) noexcept = 0;
    virtual bool onFreezeUserAircraft(bool enable) const noexcept = 0;
    virtual bool onSimulationEvent(SimulationEvent event, float arg1) const noexcept = 0;

    /*!
     * Called when recording a Flight has started.
     *
     * \return \c true on success; \c false on error (SimConnect connection error)
     */
    virtual bool onStartFlightRecording() noexcept = 0;

    /*!
     * Called when recording an Aircraft has started, excluding Flight data.
     * This method is called when recording a formation aircraft.
     *
     * \return \c true on success; \c false on error (SimConnect connection error)
     */
    virtual bool onStartAircraftRecording() noexcept = 0;
    virtual void onRecordingPaused(Initiator initiator, bool enable) noexcept = 0;
    virtual void onStopRecording() noexcept = 0;

    virtual bool onStartReplay(std::int64_t currentTimestamp) noexcept = 0;
    virtual void onReplayPaused(Initiator initiator, bool enable) noexcept = 0;
    virtual void onStopReplay() noexcept = 0;

    virtual void onSeek(std::int64_t currentTimestamp, SeekMode seekMode) noexcept = 0;

    virtual bool sendAircraftData(std::int64_t currentTimestamp, TimeVariableData::Access access, AircraftSelection aircraftSelection) noexcept = 0;
    virtual bool isConnectedWithSim() const noexcept = 0;
    virtual bool connectWithSim() noexcept = 0;
    virtual void onDisconnectFromSim() noexcept = 0;

    virtual void onAddAiObject(const Aircraft &aircraft) noexcept = 0;
    virtual void onRemoveAiObject(std::int64_t aircraftId) noexcept = 0;
    virtual void onRemoveAllAiObjects() noexcept = 0;

    /*!
     * Requests the current location. The location is asynchronously returned via signal \p locationReceived.
     *
     * \return \c true if the request was successful; \c false else
     * \sa locationReceived
     */
    virtual bool onRequestLocation() noexcept = 0;

    /*!
     * Requests the current simulation rate in the flight simulator. The rate is asynchronously
     * returned via signal \p simulationRateReceived.
     *
     * \return \c true if the request was successful; \c false else
     * \sa simulationRateReceived
     */
    virtual bool onRequestSimulationRate() noexcept = 0;

    /*!
     * Sends the \p year, \p day, \p hour and \p minute  to be set in the flight simulator
     * as zulu date and time.
     *
     * \param year
     *        the year, e.g. 2020
     * \param day
     *        the day
     * \param hour
     *        the hour [0, 23]
     * \param minute
     *        the minute [0, 59]
     * \return \c true if the request was successful; \c false else
     */
    virtual bool onSendZuluDateTime(int year, int day, int hour, int minute) const noexcept = 0;

    /*!
     * \brief Requests information about the current simulation time zone.
     *
     * \return \c true if the request was successful; \c false else
     * \sa timeZoneInfoReceived
     */
    virtual bool onRequestTimeZoneInfo() noexcept = 0;

    void addSettings(Settings::KeyValues &keyValues) const noexcept final;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept final;
    void restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept final;

protected slots:
    std::int64_t updateCurrentTimestamp() noexcept;
    void onPluginSettingsChanged(Connect::Mode mode) noexcept;

private:
    const std::unique_ptr<AbstractSkyConnectPrivate> d;

    void frenchConnection() noexcept;
    bool hasRecordingStarted() const noexcept;
    inline std::int64_t getSkipInterval() const noexcept;

    // Resets the reconnection count and starts the reconnection attempts with 'retryConnectAndSetup'
    void tryFirstConnectAndSetup() noexcept;
    inline bool retryWithReconnect(const std::function<bool()> &func);

    bool setupInitialRecordingPosition(InitialPosition initialPosition) noexcept;
    bool setupInitialReplayPosition(InitialPosition initialPosition) noexcept;
    bool updateUserAircraftFreeze() noexcept;

    // Returns the applicable simulation rate, given the current 'replaySpeedFactor'
    // and the maximum simulation rate, as defined in the application settings
    float getApplicableSimulationRate() const noexcept;

private slots:
    void onReconnectTimer() noexcept;
    void retryConnectAndSetup(Connect::Mode mode) noexcept;

    // Updates the simulation date and time, based on the start and end flight date and time of the current flight.
    // Note: the simulation time may run "faster" than the actual real-world recorded time (given by the end timestamp),
    // so the real-world time is "stretched" to the start- and end simulation time.
    bool updateSimulationTime() noexcept;
};

#endif // ABSTRACTSKYCONNECT_H
