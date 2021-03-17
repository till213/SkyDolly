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

#include <QObject>

#include "AbstractSkyConnect.h"

struct AircraftData;
class Aircraft;
class SkyConnectDummyPrivate;

class SkyConnectDummy : public AbstractSkyConnect
{
    Q_OBJECT
public:
    SkyConnectDummy(QObject *parent = nullptr);
    virtual ~SkyConnectDummy();

protected:
    virtual void onStartDataSample() override;
    virtual void onStopDataSample() override;
    virtual void onStartReplay(qint64 currentTimestamp) override;
    virtual void onStopReplay() override;
    virtual void onSeek(qint64 currentTimestamp) override;
    virtual void onRecordingPaused(bool paused) override;
    virtual void onReplayPaused() override;
    virtual void onRecordSampleRateChanged(SampleRate::SampleRate sampleRate) override;
    virtual void onPlaybackSampleRateChanged(SampleRate::SampleRate sampleRate) override;

    virtual bool sendAircraftData(qint64 currentTimestamp) override;
    virtual bool isConnectedWithSim() const override;
    virtual bool connectWithSim() override;

protected slots:
    virtual void processEvents() override;

private:
    SkyConnectDummyPrivate *d;

    bool sendAircraftData();
    void recordData();
    void replay();

};

#endif // SKYCONNECTDUMMY_H
