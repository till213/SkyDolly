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

#include <QObject>

#include <windows.h>
#include <SimConnect.h>

#include "../../Kernel/src/SampleRate.h"
#include "AbstractSkyConnectImpl.h"

struct AircraftData;
class Aircraft;
class SkyConnectPrivate;

class SkyConnectImpl : public AbstractSkyConnectImpl
{
    Q_OBJECT
public:
    SkyConnectImpl(QObject *parent = nullptr);
    virtual ~SkyConnectImpl();

    virtual void startDataSample() override;
    virtual void stopDataSample() override;

    virtual void startReplay(bool fromStart) override;
    virtual void stopReplay() override;
    virtual void stop() override;

    virtual void setPaused(bool enabled) override;
    virtual bool isPaused() const override;

    virtual void skipToBegin() override;
    virtual void skipBackward() override;
    virtual void skipForward() override;
    virtual void skipToEnd() override;

    virtual Aircraft &getAircraft()override;
    virtual const Aircraft &getAircraft() const override;

    virtual void setTimeScale(double timeScale) override;
    virtual double getTimeScale() const override;

    virtual void setCurrentTimestamp(qint64 timestamp) override;
    virtual qint64 getCurrentTimestamp() const override;
    virtual bool isAtEnd() const override;

    virtual const AircraftData &getCurrentAircraftData() const override;

private:
    SkyConnectPrivate *d;

    bool open();
    bool close();
    bool isConnected() const;

    void frenchConnection();
    void setupRequestData();
    void setupInitialPosition();
    void setSimulationFrozen(bool enable);
    bool isSimulationFrozen() const;
    bool sendAircraftPosition() const;
    void replay();
    void stopAll();
    void updateCurrentTimestamp();
    void stopRecording();
    void stopPlayback();
    bool hasRecordingStarted() const;

    static void CALLBACK dispatch(SIMCONNECT_RECV *receivedData, DWORD cbData, void *context);

private slots:
    void processEvents();
    void handleRecordSampleRateChanged(double sampleRateValue);
    void handlePlaybackSampleRateChanged(double sampleRateValue);
};

#endif // SKYCONNECTIMPL_H
