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
#ifndef SKYCONNECT_H
#define SKYCONNECT_H

#include <cstdint>

#include <QtPlugin>

#include <Kernel/SampleRate.h>
#include <Model/TimeVariableData.h>
#include <Model/InitialPosition.h>
#include "Connect.h"
#include "PluginManagerLib.h"

class Flight;
class Aircraft;
struct PositionData;
class MSFSSimConnectPlugin;

class PLUGINMANAGER_API SkyConnectIntf : public QObject
{
    Q_OBJECT
public:

    enum struct RecordingMode {
        /*! A (new) flight with a single aircrat is to be recorded. */
        SingleAircraft,
        /*! The aircraft is to be added to the current flight; existing aircraft
         *  are replayed during recording.
         */
        AddToFormation
    };

    enum struct ReplayMode {
        /*! All aircraft are controlled by Sky Dolly. */
        Normal,
        /*! User takes control of recorded user aircraft. */
        UserAircraftManualControl,
        /*! User flies along with all recorded aircraft. */
        FlyWithFormation
    };

    ~SkyConnectIntf() override = default;

    virtual bool setUserAircraftInitialPosition(const InitialPosition &initialPosition) noexcept = 0;
    virtual bool setUserAircraftPosition(const PositionData &positionData) noexcept = 0;
    virtual bool freezeUserAircraft(bool enable) noexcept = 0;

    virtual ReplayMode getReplayMode() const noexcept = 0;
    virtual void setReplayMode(ReplayMode replayMode) noexcept = 0;

    virtual void startRecording(RecordingMode recordingMode, const InitialPosition &initialPosition = InitialPosition()) noexcept = 0;
    virtual void stopRecording() noexcept = 0;
    virtual bool isRecording() const noexcept = 0;

    virtual void startReplay(bool fromStart, const InitialPosition &flyWithFormationPosition = InitialPosition()) noexcept = 0;
    virtual void stopReplay() noexcept = 0;
    virtual bool isReplaying() const noexcept = 0;
    virtual void stop() noexcept = 0;

    virtual bool inRecordingMode() const noexcept = 0;
    virtual bool inReplayMode() const noexcept = 0;

    /*!
     * Returns whether SkyConnect is active or not. \em Active means any of the following states:
     *
     * - Connect::State::Recording
     * - Connect::State::RecordingPaused
     * - Connect::State::Replay
     * - Connect::State::ReplayPaused
     *
     * Or in other words: any state except Connected and Disconnected.
     *
     * \return \c true if SkyConnect is active; \c false else
     */
    virtual bool isActive() const noexcept = 0;

    virtual void setPaused(bool enabled) noexcept = 0;
    virtual bool isPaused() const noexcept = 0;

    virtual void skipToBegin() noexcept = 0;
    virtual void skipBackward() noexcept = 0;
    virtual void skipForward() noexcept = 0;
    virtual void skipToEnd() noexcept = 0;
    virtual void seek(std::int64_t timestamp) noexcept = 0;
    virtual void handleAtEnd() noexcept = 0;

    virtual std::int64_t getCurrentTimestamp() const noexcept = 0;
    virtual bool isAtEnd() const noexcept = 0;

    virtual double getReplaySpeedFactor() const noexcept = 0;
    virtual void setReplaySpeedFactor(double replaySpeed) noexcept = 0;

    virtual Connect::State getState() const noexcept = 0;
    virtual bool isConnected() const noexcept = 0;
    virtual bool isIdle() const noexcept = 0;

    virtual double calculateRecordedSamplesPerSecond() const noexcept = 0;

public slots:
    virtual void addAiObject(const Aircraft &aircraft) noexcept = 0;
    virtual void removeAiObjects() noexcept = 0;
    virtual void removeAiObject(std::int64_t removedAircraftId) noexcept = 0;
    virtual void syncAiObjectsWithFlight() noexcept = 0;
    virtual void updateUserAircraft(int newUserAircraftIndex, int previousUserAircraftIndex) noexcept = 0;

protected:
    SkyConnectIntf(QObject *parent = nullptr) noexcept
        : QObject(parent)
    {}

    /*!
     * Sets the new connection \c state. This method will also emit the
     * signal #recordingStopped when the state changes from \em Recording
     * to any other state.
     *
     * Also refer to #stateChanged and #recordingStopped
     *
     * \param state
     *        the new state to be set
     */
    virtual void setState(Connect::State state) noexcept = 0;

signals:
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
     * Emitted whenever recording has been stopped, that is when the
     * state changes from \em Recording to any other state (\em Connected
     * or \em Disconnected).
     *
     * Note that the #stateChanged signal is emitted as well.
     */
    void recordingStopped();

private:
    Q_DISABLE_COPY(SkyConnectIntf)
};

#define SKYCONNECT_INTERFACE_IID "com.github.till213.SkyDolly.SkyConnectInterface/1.0"
Q_DECLARE_INTERFACE(SkyConnectIntf, SKYCONNECT_INTERFACE_IID)

#endif // SKYCONNECT_H
