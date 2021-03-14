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
#ifndef SKYCONNECTIMPL_H
#define SKYCONNECTIMPL_H
#include <memory>

#include <QObject>

#include <windows.h>
#include <SimConnect.h>

#include "../../Kernel/src/SampleRate.h"
#include "AbstractSkyConnect.h"

struct AircraftData;
class Aircraft;
class SkyConnectPrivate;

class SkyConnectImpl : public AbstractSkyConnect
{
    Q_OBJECT
public:
    SkyConnectImpl(QObject *parent = nullptr);
    virtual ~SkyConnectImpl();

protected:
    virtual void onStartDataSample() override;
    virtual void onStopDataSample() override;
    virtual void onStartReplay(bool fromStart) override;
    virtual void onStopReplay() override;
    virtual void onRecordingPaused(bool paused) override;
    virtual void onReplayPaused() override;
    virtual void onRecordSampleRateChaged(SampleRate::SampleRate sampleRate) override;
    virtual void onPlaybackSampleRateChanged(SampleRate::SampleRate sampleRate) override;

    virtual bool sendAircraftData(qint64 currentTimestamp) override;
    virtual bool connectWithSim() override;
    virtual bool isConnectedWithSim() const override;

protected slots:
    void processEvents() override;

private:
    std::unique_ptr<SkyConnectPrivate> d;

    bool reconnect();
    bool close();
    void setupRequestData();
    void setupInitialPosition();
    void setSimulationFrozen(bool enable);
    bool isSimulationFrozen() const;
    bool sendAircraftData();
    void replay();
    void updateRecordFrequency(SampleRate::SampleRate sampleRate);

    static void CALLBACK dispatch(SIMCONNECT_RECV *receivedData, DWORD cbData, void *context);
};

#endif // SKYCONNECTIMPL_H
