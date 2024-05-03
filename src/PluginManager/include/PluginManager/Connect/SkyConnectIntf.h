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
#ifndef SKYCONNECT_H
#define SKYCONNECT_H

#include <cstdint>

#include <QtPlugin>

#include <Kernel/SampleRate.h>
#include <Model/TimeVariableData.h>
#include <Model/InitialPosition.h>
#include <Model/Location.h>
#include "Connect.h"
#include "FlightSimulatorShortcuts.h"
#include "../PluginIntf.h"
#include "../PluginWithOptionWidgetIntf.h"
#include "../PluginManagerLib.h"

class Flight;
class Aircraft;
struct PositionData;
class MSFSSimConnectPlugin;

class PLUGINMANAGER_API SkyConnectIntf : public QObject, public PluginWithOptionWidgetIntf, public PluginIntf
{
    Q_OBJECT
public:
    enum struct RecordingMode {
        /*! A (new) flight with a single aircrat is to be recorded. */
        SingleAircraft,
        /*!
         * The aircraft is to be added to the current flight; existing aircraft
         * are replayed during recording.
         */
        AddToFormation
    };

    /*!
     * Defines of which aircraft to take control of during formation replay.
     *
     * Implementation note: those values act as actual IDs that get persisted in the database.
     */
    enum struct ReplayMode {
        /*! All aircraft are controlled by Sky Dolly. */
        Normal = 0,
        /*! User takes control of recorded user aircraft. */
        UserAircraftManualControl = 1,
        /*! User flies along with all recorded aircraft. */
        FlyWithFormation = 2
    };

    enum struct SeekMode {

        /*! Continuation of a timeline seek operation ("drag timeline") */
        Continuous,
        /*! A single seek operation (to beginning, to end, to selected position) */
        Discrete

    };

    /*!
     * Simulation events that can explicitly be triggered (requested) by the application.
     */
    enum struct SimulationEvent {
        None,
        EngineStart,
        EngineStop,
        /*! Argument 1: simulation rate */
        SimulationRate
    };

    /*!
     * Indicates who initiated an event such as a pause event.
     */
    enum struct Initiator {
        /*! The application initiated the event */
        App,
        /*! The flight simulator initiated the event */
        FlightSimulator
    };

    SkyConnectIntf(QObject *parent = nullptr) noexcept
        : QObject(parent)
    {}

    SkyConnectIntf(const SkyConnectIntf &rhs) = delete;
    SkyConnectIntf(SkyConnectIntf &&rhs) = delete;
    SkyConnectIntf &operator=(const SkyConnectIntf &rhs) = delete;
    SkyConnectIntf &operator=(SkyConnectIntf &&rhs) = delete;
    ~SkyConnectIntf() override = default;
    
    /*!
     * Try to connect with the flight simulator. If the connection has been
     * successfully established then also setup the shortcuts in
     * the flight simulator.
     *
     * Otherwise a timer is started that tries to reconnnect in various
     * intervals with increasing length. Already running reconnect timers
     * are restarted, resetting the reconnect interval as well.
     *
     * \sa stateChanged
     */
    virtual void tryConnectAndSetup() noexcept = 0;

    /*!
     * Disconnects from the flight simulator. Also call this method when the disconnect
     * is initiated by the flight simulator itself.
     *
     * Any running reconnect timer is restarted.
     *
     * \sa tryConnectAndSetup
     * \sa stateChanged
     */
    virtual void disconnect() noexcept = 0;

    /*!
     * Returns the timer's remaining value in milliseconds left until the timeout.
     *
     * \return the remaining time until the next reconnect attempt is being made [seconds];
     *         if the timer is inactive, the returned value will be -1;
     *         if the timer is overdue, the returned value will be 0.
     */
    virtual int getRemainingReconnectTime() const noexcept = 0;

    virtual bool setUserAircraftInitialPosition(const InitialPosition &initialPosition) noexcept = 0;
    virtual bool setUserAircraftPosition(const PositionData &positionData) noexcept = 0;
    virtual bool freezeUserAircraft(bool enable) const noexcept = 0;
    virtual bool sendSimulationEvent(SimulationEvent event, float arg1) noexcept = 0;

    /*!
     * Returns the replay mode.
     *
     * \return the ReplayMode
     */
    virtual ReplayMode getReplayMode() const noexcept = 0;

    /*!
     * Sets the replay mode.
     *
     * \param replayMode
     *         the replay mode to set
     * \sa replayModeChanged
     */
    virtual void setReplayMode(ReplayMode replayMode) noexcept = 0;

    /*!
     * Starts recording the flight. Depending on the \c recordingMode already recorded formation aircraft
     * are replayed during recording. If the \c initialPosition is given (\c isNull() returns false) then
     * the user aircraft is placed at the given \c initialPosition before recording. This position is
     * typically calculated to be relative of the previous user aircraft in the formation.
     *
     * \param recordingMode
     *        the RecordingMode which controls whether existing formation aircraft are to be replayed
     * \param initialPosition
     *        the optional initial position where the current user aircraft is placed before recording;
     *        set to a \e null position if the user aircraft should keep its current initial position
     */
    virtual void startRecording(RecordingMode recordingMode, const InitialPosition &initialPosition = InitialPosition()) noexcept = 0;
    virtual void stopRecording() noexcept = 0;

    /*!
     * Returns whether SkyConnect is in \e Recording state, specifically:
     *
     * - Connect::State::Recording
     *
     * \return \c true if SkyConnect is in \e Recording state; \c false else
     * \sa isInRecordingState
     */
    virtual bool isRecording() const noexcept = 0;

    /*!
     * Returns whether SkyConnect is in a recording state or not. A recording state means any of the following states:
     *
     * - Connect::State::Recording
     * - Connect::State::RecordingPaused
     *
     * \return \c true if SkyConnect is in a recording state; \c false else
     * \sa isRecording
     * \sa isInReplayState
     * \sa isActive
     */
    virtual bool isInRecordingState() const noexcept = 0;

    virtual void startReplay(bool fromStart, const InitialPosition &flyWithFormationPosition = InitialPosition()) noexcept = 0;
    virtual void stopReplay() noexcept = 0;

    /*!
     * Returns whether SkyConnect is in \e Replay state, specifically:
     *
     * - Connect::State::Replay
     *
     * \return \c true if SkyConnect is in \e Replay state; \c false else
     * \sa isInReplayState
     */
    virtual bool isReplaying() const noexcept = 0;

    /*!
     * Returns whether SkyConnect is in a replay state or not. A replay state means any of the following states:
     *
     * - Connect::State::Replay
     * - Connect::State::ReplayPaused
     *
     * \return \c true if SkyConnect is in replay state; \c false else
     * \sa isReplaying
     * \sa isInRecordingState
     * \sa isActive
     */
    virtual bool isInReplayState() const noexcept = 0;

    virtual void stop() noexcept = 0;

    /*!
     * Returns whether SkyConnect is active or not. Active means any of the following states:
     *
     * - Connect::State::Recording
     * - Connect::State::RecordingPaused
     * - Connect::State::Replay
     * - Connect::State::ReplayPaused
     *
     * Or in other words: any state except \e Connected and \e Disconnected.
     *
     * \return \c true if SkyConnect is active; \c false else
     * \sa isInRecordingState
     * \sa isInReplayState
     */
    virtual bool isActive() const noexcept = 0;

    /*!
     * Pauses the current activity - either recording or replay.
     *
     * \param initiator
     *        the Initiator of the action (the flight simulator or the application)
     * \param enable
     *        set to \c true to pause the current activity; \c false to resume
     *        the paused activity
     */
    virtual void setPaused(Initiator initiator, bool enable) noexcept = 0;

    /*!
     * Returns whether either recording or replay has been paused.
     *
     * \return \c true if recording or replay has been paused; \c false else
     * \sa Connect::State::RecordingPaused
     * \sa Connect::State::ReplayPaused
     */
    virtual bool isPaused() const noexcept = 0;

    /*!
     * Returns whether recording has been paused.
     *
     * \return \c true if recording has been paused; \c false else
     * \sa Connect::State::RecordingPaused
     */
    virtual bool isRecordingPaused() const noexcept = 0;

    virtual void skipToBegin() noexcept = 0;
    virtual void skipBackward() noexcept = 0;
    virtual void skipForward() noexcept = 0;
    virtual void skipToEnd() noexcept = 0;
    virtual void seek(std::int64_t timestamp, SeekMode seekMode) noexcept = 0;
    virtual void handleAtEnd() noexcept = 0;

    virtual Connect::State getState() const noexcept = 0;
    virtual bool isConnected() const noexcept = 0;
    virtual bool isIdle() const noexcept = 0;

    virtual std::int64_t getCurrentTimestamp() const noexcept = 0;
    virtual bool isAtEnd() const noexcept = 0;

    virtual double getReplaySpeedFactor() const noexcept = 0;
    virtual void setReplaySpeedFactor(double factor) noexcept = 0;

    virtual double calculateRecordedSamplesPerSecond() const noexcept = 0;

    /*!
     * Requests the current position of the user aircraft whcich is asynchronously
     * returned as Location.
     *
     * \return \c true if the request was sent successfully; \c false else (e.g. no connection)
     * \sa locationReceived
     */
    virtual bool requestLocation() noexcept = 0;

public slots:
    virtual void addAiObject(const Aircraft &aircraft) noexcept = 0;
    virtual void removeAiObjects() noexcept = 0;
    virtual void removeAiObject(std::int64_t removedAircraftId) noexcept = 0;
    virtual void syncAiObjectsWithFlight() noexcept = 0;
    virtual void updateUserAircraft(int newUserAircraftIndex, int previousUserAircraftIndex) noexcept = 0;
    virtual void onTimeOffsetChanged() noexcept = 0;
    virtual void onTailNumberChanged(const Aircraft &aircraft) noexcept = 0;

signals:
    /*!
     * Emitted whenever the timestamp has changed.
     *
     * \param timestamp
     *        the current timestamp [millisceonds]
     * \param access
     *        the way the current position was accessed
     */
    void timestampChanged(std::int64_t timestamp, TimeVariableData::Access access);

    /*!
     * Emitted whenver the connection state has changed.
     *
     * Also refer to #recordingStopped.
     *
     * \param state
     *        the current connection state
     */
    void stateChanged(Connect::State state);

    /*!
     * Emitted whenever the replay mode has changed.
     *
     * \param replayMode
     *        the current replay mode
     */
    void replayModeChanged(SkyConnectIntf::ReplayMode replayMode);

    /*!
     * Emitted whenever recording has been started, that is when the
     * state changes from \e Connected or \e Disconnected to a recording
     * state (typically \e Recording, possibly also \e RecordingPaused).
     *
     * Note that the #stateChanged signal is emitted as well.
     *
     * \sa isInRecordingState
     * \sa stateChanged
     */
    void recordingStarted();

    /*!
     * Emitted whenever recording has been stopped, that is when the
     * state changes from \e Recording or \e RecordingPaused to any other
     * state (\e Connected or \e Disconnected).
     *
     * Note that the #stateChanged signal is emitted as well.
     *
     * \sa isInRecordingState
     * \sa stateChanged
     */
    void recordingStopped();

    /*!
     * Emitted whenever the response to the Location request has been received.
     *
     * \param location
     *        the received Location
     */
    void locationReceived(Location location);

    /*!
     * Emitted whenever a keyboard shortcut was triggered for the given \p action.
     *
     * \param action
     *        the action that was triggered in the flight simulator
     */
    void shortCutActivated(FlightSimulatorShortcuts::Action action);

protected:
    /*!
     * Sets the new connection \c state. This method will also emit the
     * signal #recordingStarted and #recordingStopped when the state changes
     * to/from \e Recording.
     *
     * \param state
     *        the new state to be set
     * \sa stateChanged
     * \sa recordingStarted
     * \sa recordingStopped
     */
    virtual void setState(Connect::State state) noexcept = 0;
};

#define SKYCONNECT_INTERFACE_IID "com.github.till213.SkyDolly.SkyConnectInterface/1.0"
Q_DECLARE_INTERFACE(SkyConnectIntf, SKYCONNECT_INTERFACE_IID)

#endif // SKYCONNECT_H
