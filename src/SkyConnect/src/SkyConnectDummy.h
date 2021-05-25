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
#ifndef SKYCONNECTDUMMY_H
#define SKYCONNECTDUMMY_H

#include <memory>

#include <QObject>

#include "../../Model/src/TimeVariableData.h"
#include "AbstractSkyConnect.h"

struct PositionData;
class Aircraft;
class SkyConnectDummyPrivate;

class SkyConnectDummy : public AbstractSkyConnect
{
    Q_OBJECT
public:
    SkyConnectDummy(QObject *parent = nullptr) noexcept;
    virtual ~SkyConnectDummy() noexcept;

    virtual bool createAIObjects() noexcept override;
    virtual void destroyAIObjects() noexcept override;

protected:
    virtual bool onStartRecording() noexcept override;
    virtual void onRecordingPaused(bool paused) noexcept override;
    virtual void onStopRecording() noexcept override;

    virtual bool onStartReplay(qint64 currentTimestamp) noexcept override;
    virtual void onReplayPaused(bool paused) noexcept override;
    virtual void onStopReplay() noexcept override;

    virtual void onSeek(qint64 currentTimestamp) noexcept override;
    virtual void onRecordSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept override;

    virtual bool sendAircraftData(qint64 currentTimestamp, TimeVariableData::Access access) noexcept override;
    virtual bool isConnectedWithSim() const noexcept override;
    virtual bool connectWithSim() noexcept override;

protected slots:
    virtual void processEvents() noexcept override;

private:
    std::unique_ptr<SkyConnectDummyPrivate> d;

    void frenchConnection() noexcept;
    bool sendAircraftData(TimeVariableData::Access access) noexcept;

    void recordData() noexcept;
    void recordPositionData(qint64 timestamp) noexcept;
    void recordEngineData(qint64 timestamp) noexcept;
    void recordPrimaryControls(qint64 timestamp) noexcept;
    void recordSecondaryControls(qint64 timestamp) noexcept;
    void recordAircraftHandle(qint64 timestamp) noexcept;
    void recordLights(qint64 timestamp) noexcept;
    void recordWaypoint() noexcept;
    void recordFlightCondition() noexcept;
    void recordAircraftInfo() noexcept;

    void replay() noexcept;
};

#endif // SKYCONNECTDUMMY_H
