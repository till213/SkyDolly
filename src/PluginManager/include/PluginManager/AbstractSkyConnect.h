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
#ifndef ABSTRACTSKYCONNECTIMPL_H
#define ABSTRACTSKYCONNECTIMPL_H

#include <memory>
#include <cstdint>

#include <QObject>

#include <Kernel/SampleRate.h>
#include <Model/InitialPosition.h>
#include "SkyConnectIntf.h"
#include "Connect.h"
#include "PluginManagerLib.h"

class Flight;
class Aircraft;
class AbstractSkyConnectPrivate;

class PLUGINMANAGER_API AbstractSkyConnect : public SkyConnectIntf
{
    Q_OBJECT
public:

    enum struct AircraftSelection {
        All,
        UserAircraft
    };

    AbstractSkyConnect(QObject *parent = nullptr) noexcept;
    ~AbstractSkyConnect() noexcept override;

    bool setUserAircraftInitialPosition(const InitialPosition &initialPosition) noexcept override;
    bool freezeUserAircraft(bool enable) noexcept override;

    ReplayMode getReplayMode() const noexcept override;
    void setReplayMode(ReplayMode replayMode) noexcept override;

    void startRecording(RecordingMode recordingMode, const InitialPosition &initialPosition = InitialPosition()) noexcept override;
    void stopRecording() noexcept override;
    bool isRecording() const noexcept override;

    void startReplay(bool fromStart, const InitialPosition &flyWithFormationPosition = InitialPosition()) noexcept override;
    void stopReplay() noexcept override;
    bool isReplaying() const noexcept override;
    void stop() noexcept override;

    bool inRecordingMode() const noexcept override;
    bool inReplayMode() const noexcept override;
    bool isActive() const noexcept override;

    void setPaused(bool enabled) noexcept override;
    bool isPaused() const noexcept override;

    void skipToBegin() noexcept override;
    void skipBackward() noexcept override;
    void skipForward() noexcept override;
    void skipToEnd() noexcept override;
    void seek(std::int64_t timestamp) noexcept override;
    void handleAtEnd() noexcept override;

    std::int64_t getCurrentTimestamp() const noexcept override;
    bool isAtEnd() const noexcept override;

    double getReplaySpeedFactor() const noexcept override;
    void setReplaySpeedFactor(double timeScale) noexcept override;

    Connect::State getState() const noexcept override;
    bool isConnected() const noexcept override;
    bool isIdle() const noexcept override;

    double calculateRecordedSamplesPerSecond() const noexcept override;

public slots:
    void addAiObject(const Aircraft &aircraft) noexcept override;
    void removeAiObjects() noexcept override;
    void removeAiObject(std::int64_t removedAircraftId) noexcept override;
    void syncAiObjectsWithFlight() noexcept override;
    void updateUserAircraft(int newUserAircraftIndex, int previousUserAircraftIndex) noexcept override;

protected:
    void setState(Connect::State state) noexcept;
    Flight &getCurrentFlight() const;
    void setCurrentTimestamp(std::int64_t timestamp) noexcept;

    bool isElapsedTimerRunning() const noexcept;   
    void startElapsedTimer() const noexcept;
    void resetElapsedTime(bool restart) noexcept;
    std::int64_t updateCurrentTimestamp() noexcept;

    void createAiObjects() noexcept;

    virtual bool isTimerBasedRecording(SampleRate::SampleRate sampleRate) const noexcept = 0;

    virtual bool onInitialPositionSetup(const InitialPosition &initialPosition) noexcept = 0;
    virtual bool onFreezeUserAircraft(bool enable) noexcept = 0;

    virtual bool onStartRecording() noexcept = 0;
    virtual void onRecordingPaused(bool paused) noexcept = 0;
    virtual void onStopRecording() noexcept = 0;

    virtual bool onStartReplay(std::int64_t currentTimestamp) noexcept = 0;
    virtual void onReplayPaused(bool paused) noexcept = 0;
    virtual void onStopReplay() noexcept = 0;

    virtual void onSeek(std::int64_t currentTimestamp) noexcept = 0;
    virtual void onRecordingSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept = 0;

    virtual bool sendAircraftData(std::int64_t currentTimestamp, TimeVariableData::Access access, AircraftSelection aircraftSelection) noexcept = 0;
    virtual bool isConnectedWithSim() const noexcept = 0;
    virtual bool connectWithSim() noexcept = 0;

    virtual void onAddAiObject(const Aircraft &aircraft) noexcept = 0;
    virtual void onRemoveAiObject(std::int64_t aircraftId) noexcept = 0;

protected slots:
    virtual void recordData() noexcept = 0;

private:
    Q_DISABLE_COPY(AbstractSkyConnect)
    std::unique_ptr<AbstractSkyConnectPrivate> d;

    void frenchConnection() noexcept;
    bool hasRecordingStarted() const noexcept;
    inline std::int64_t getSkipInterval() const noexcept;

    inline bool retryWithReconnect(std::function<bool()> func);

    bool setupInitialRecordingPosition(const InitialPosition &initialPosition) noexcept;
    bool setupInitialReplayPosition(const InitialPosition &flyWithFormationPosition) noexcept;
    bool updateUserAircraftFreeze() noexcept;

private slots:
    void handleRecordingSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept;
};

#endif // ABSTRACTSKYCONNECTIMPL_H
