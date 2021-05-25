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
#ifndef SKYCONNECT_H
#define SKYCONNECT_H

#include "../../Kernel/src/SampleRate.h"
#include "../../Model/src/TimeVariableData.h"
#include "../../Model/src/Aircraft.h"
#include "Connect.h"
#include "SkyConnectLib.h"

class SkyConnectImpl;

class SKYCONNECT_API SkyConnectIntf : public QObject
{
    Q_OBJECT
public:

    virtual ~SkyConnectIntf() = default;

    virtual void startRecording(bool addFormationAircraft) noexcept = 0;
    virtual void stopRecording() noexcept = 0;
    virtual bool isRecording() const noexcept = 0;

    virtual void startReplay(bool fromStart) noexcept = 0;
    virtual void stopReplay() noexcept = 0;
    virtual bool isReplaying() const noexcept = 0;
    virtual void stop() noexcept = 0;

    virtual void setPaused(bool enabled) noexcept = 0;
    virtual bool isPaused() const noexcept = 0;

    virtual void skipToBegin() noexcept = 0;
    virtual void skipBackward() noexcept = 0;
    virtual void skipForward() noexcept = 0;
    virtual void skipToEnd() noexcept = 0;
    virtual void seek(qint64 timestamp) noexcept = 0;
    virtual qint64 getCurrentTimestamp() const noexcept = 0;
    virtual bool isAtEnd() const noexcept = 0;

    virtual double getReplaySpeedFactor() const noexcept = 0;
    virtual void setReplaySpeedFactor(double replaySpeed) noexcept = 0;

    virtual Connect::State getState() const noexcept = 0;
    virtual bool isConnected() const noexcept = 0;
    virtual bool isIdle() const noexcept = 0;

    virtual double calculateRecordedSamplesPerSecond() const noexcept = 0;

    virtual bool createAIObjects() noexcept = 0;
    virtual void destroyAIObjects() noexcept = 0;

protected:
    SkyConnectIntf(QObject *parent = nullptr) noexcept
        : QObject(parent)
    {}

signals:
    void timestampChanged(qint64 timestamp, TimeVariableData::Access access);
    void stateChanged(Connect::State state);
    void recordingStopped();
};

#endif // SKYCONNECT_H
