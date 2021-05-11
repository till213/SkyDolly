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
#ifndef ABSTRACTSKYCONNECTIMPL_H
#define ABSTRACTSKYCONNECTIMPL_H

#include <memory>

#include <QObject>

#include "../../Kernel/src/SampleRate.h"
#include "SkyConnectIntf.h"
#include "Connect.h"

class Flight;
class AbstractSkyConnectPrivate;

class  AbstractSkyConnect : public SkyConnectIntf
{
    Q_OBJECT

public:
    AbstractSkyConnect(QObject *parent = nullptr) noexcept;
    virtual ~AbstractSkyConnect() noexcept;

    virtual void startRecording() noexcept override;
    virtual void stopRecording() noexcept override;
    virtual bool isRecording() const noexcept override;

    virtual void startReplay(bool fromStart) noexcept override;
    virtual void stopReplay() noexcept override;
    virtual bool isReplaying() const noexcept override;
    virtual void stop() noexcept override;

    virtual void setPaused(bool enabled) noexcept override;
    virtual bool isPaused() const noexcept override;

    virtual void skipToBegin() noexcept override;
    virtual void skipBackward() noexcept override;
    virtual void skipForward() noexcept override;
    virtual void skipToEnd() noexcept override;
    virtual void seek(qint64 timestamp) noexcept override;

    virtual qint64 getCurrentTimestamp() const noexcept override;
    virtual bool isAtEnd() const noexcept override;

    virtual double getReplaySpeedFactor() const noexcept override;
    virtual void setReplaySpeedFactor(double timeScale) noexcept override;

    virtual Connect::State getState() const noexcept override;
    virtual bool isConnected() const noexcept override;

    virtual double calculateRecordedSamplesPerSecond() const noexcept override;

protected:
    void setState(Connect::State state) noexcept;
    Flight &getCurrentFlight() const;
    void setCurrentTimestamp(qint64 timestamp) noexcept;

    bool isElapsedTimerRunning() const noexcept;
    bool isAutoRecordSampleRate() const noexcept;
    void startElapsedTimer() const noexcept;
    void resetElapsedTime(bool restart) noexcept;
    void updateCurrentTimestamp() noexcept;

    virtual void onStartRecording() = 0;
    virtual void onRecordingPaused(bool paused) = 0;
    virtual void onStopRecording() = 0;

    virtual void onStartReplay(qint64 currentTimestamp) = 0;
    virtual void onReplayPaused(bool paused) = 0;
    virtual void onStopReplay() = 0;

    virtual void onSeek(qint64 currentTimestamp) = 0;
    virtual void onRecordSampleRateChanged(SampleRate::SampleRate sampleRate) = 0;

    virtual bool sendAircraftData(qint64 currentTimestamp, TimeVariableData::Access access) = 0;
    virtual bool isConnectedWithSim() const = 0;
    virtual bool connectWithSim() = 0;

protected slots:
    virtual void processEvents() noexcept = 0;

private:
    Q_DISABLE_COPY(AbstractSkyConnect)
    std::unique_ptr<AbstractSkyConnectPrivate> d;

    void frenchConnection() noexcept;
    bool hasRecordingStarted() const noexcept;
    inline qint64 getSkipInterval() const noexcept;

private slots:
    void handleRecordSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept;
};

#endif // ABSTRACTSKYCONNECTIMPL_H
