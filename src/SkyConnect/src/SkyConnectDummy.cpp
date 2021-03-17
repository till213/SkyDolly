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
#include <QTimer>
#include <QElapsedTimer>
#include <QtGlobal>
#include <QRandomGenerator>

#include "../../Kernel/src/AircraftData.h"
#include "../../Kernel/src/Settings.h"
#include "AbstractSkyConnect.h"
#include "SkyConnectDummy.h"


class SkyConnectDummyPrivate
{
public:
    SkyConnectDummyPrivate()
    {}
};

// PUBLIC

SkyConnectDummy::SkyConnectDummy(QObject *parent)
    : AbstractSkyConnect(parent),
      d(new SkyConnectDummyPrivate())
{
}

SkyConnectDummy::~SkyConnectDummy()
{
    delete d;
}

// PROTECTED

void SkyConnectDummy::onStartDataSample() {

}

void SkyConnectDummy::onStopDataSample() {}

void SkyConnectDummy::onStartReplay(qint64 currentTimestamp) {
    Q_UNUSED(currentTimestamp)
}

void SkyConnectDummy::onSeek(qint64 currentTimestamp)
{
    Q_UNUSED(currentTimestamp)
};

void SkyConnectDummy::onStopReplay() {}

void SkyConnectDummy::onRecordingPaused(bool paused) {
    Q_UNUSED(paused)
}

void SkyConnectDummy::onReplayPaused() {}

void SkyConnectDummy::onRecordSampleRateChanged(SampleRate::SampleRate sampleRate)
{
    Q_UNUSED(sampleRate)
}

void SkyConnectDummy::onPlaybackSampleRateChanged(SampleRate::SampleRate sampleRate)
{
    Q_UNUSED(sampleRate)
}

bool SkyConnectDummy::sendAircraftData(qint64 currentTimestamp) {
    Q_UNUSED(currentTimestamp)
    return sendAircraftData();
}

bool SkyConnectDummy::isConnectedWithSim() const {
    return true;
}

bool SkyConnectDummy::connectWithSim() {
    return true;
}

// PROTECTED SLOTS

void SkyConnectDummy::processEvents()
{
    updateCurrentTimestamp();
    switch (getState()) {
    case Connect::State::Recording:
        recordData();
        break;
    case Connect::State::Playback:
        replay();
        break;
    default:
        break;
    }
}

// PRIVATE

bool SkyConnectDummy::sendAircraftData()
{
    bool success;

    if (!updateCurrentAircraftData().isNull()) {
        // Start the elapsed timer after sending the first sample data
        if (!isElapsedTimerRunning()) {
            startElapsedTimer();
        }
        success = true;
    } else {
        success = false;
    }
    return success;
}

void SkyConnectDummy::recordData()
{
    AircraftData aircraftData;
    aircraftData.latitude = QRandomGenerator::global()->bounded(180.0);
    aircraftData.longitude = QRandomGenerator::global()->bounded(90.0);
    aircraftData.altitude = QRandomGenerator::global()->bounded(20000.0);
    aircraftData.timestamp = getCurrentTimestamp();
    getAircraft().upsertAircraftData(std::move(aircraftData));

    if (!isElapsedTimerRunning()) {
        // Start the elapsed timer with the arrival of the first sample data
        setCurrentTimestamp(0);
        resetElapsedTime(true);
    }
}

void SkyConnectDummy::replay()
{
    if (sendAircraftData()) {
        emit aircraftDataSent(getCurrentTimestamp());
    } else {
        stopReplay();
    }
}

