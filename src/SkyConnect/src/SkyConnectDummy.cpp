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

#include "../../Kernel/src/Settings.h"
#include "AbstractSkyConnectImpl.h"
#include "SkyConnectDummy.h"

namespace {
    constexpr qint64 SkipMSec = 1000;
}

class SkyConnectDummyPrivate
{
public:
    SkyConnectDummyPrivate()
        : currentTimestamp(0),
          elapsedTime(0),
          recordSampleRate(Settings::getInstance().getRecordSampleRateValue()),
          recordIntervalMSec(static_cast<int>(1.0 / recordSampleRate * 1000.0)),
          playbackSampleRate(Settings::getInstance().getPlaybackSampleRateValue()),
          playbackIntervalMSec(static_cast<int>(1.0 / playbackSampleRate * 1000.0)),
          timeScale(1.0)
    {
    }

    QTimer timer;
    qint64 currentTimestamp;
    QElapsedTimer elapsedTimer;
    qint64 elapsedTime;
    Aircraft aircraft;
    AircraftData currentAircraftData;
    double recordSampleRate;
    int    recordIntervalMSec;
    double playbackSampleRate;
    int    playbackIntervalMSec;
    double timeScale;
};

// PUBLIC

SkyConnectDummy::SkyConnectDummy(QObject *parent)
    : AbstractSkyConnectImpl(parent),
      d(new SkyConnectDummyPrivate())
{
    frenchConnection();
}


SkyConnectDummy::~SkyConnectDummy()
{
    delete d;
}

void SkyConnectDummy::startDataSample()
{
    setState(Connect::State::Recording);
    d->aircraft.clear();
    d->timer.setInterval(d->recordIntervalMSec);

    d->timer.start();
    d->elapsedTimer.start();
}

void SkyConnectDummy::stopDataSample()
{
    d->timer.stop();
    setState(Connect::State::Idle);
}

void SkyConnectDummy::startReplay(bool fromStart)
{
    setState(Connect::State::Playback);
    d->timer.setInterval(d->playbackIntervalMSec);

    if (fromStart) {
        d->elapsedTime = 0;
        d->currentTimestamp = 0;
    }

    d->timer.start();
    d->elapsedTimer.start();
}

void SkyConnectDummy::stopReplay()
{
    setState(Connect::State::Idle);
    d->timer.stop();
    // Remember elapsed time since last replay start, in order to continue from
    // current timestamp
    d->elapsedTime = d->currentTimestamp;
}

void SkyConnectDummy::stop()
{

}

void SkyConnectDummy::setPaused(bool enabled)
{
    Connect::State newState;
    if (enabled) {
        switch (getState()) {
        case Connect::Recording:
            newState = Connect::RecordingPaused;
            // Store the elapsed recording time...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed();
            d->elapsedTimer.invalidate();
            break;
        case Connect::Playback:
            newState = Connect::PlaybackPaused;
            // In case the elapsed time has started (is valid)...
            if (d->elapsedTimer.isValid()) {
                // ... store the elapsed playback time measured with the current time scale...
                d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed() * d->timeScale;
                // ... and stop the elapsed timer
                d->elapsedTimer.invalidate();
            }
            break;
         default:
            // No state change
            newState = getState();
        }
    } else {
        switch (getState()) {
        case Connect::RecordingPaused:
            newState = Connect::Recording;
            if (hasRecordingStarted()) {
                // Resume recording (but only if it has already recorded samples before)
                d->elapsedTimer.start();
            }
            break;
        case Connect::PlaybackPaused:
            newState = Connect::Playback;
            d->elapsedTimer.start();
            break;
         default:
            // No state change
            newState = getState();
        }
    }
    setState(newState);
}

bool SkyConnectDummy::isPaused() const {
    return getState() == Connect::RecordingPaused || getState() == Connect::PlaybackPaused;
}

void SkyConnectDummy::skipToBegin()
{
    setCurrentTimestamp(0);
}

void SkyConnectDummy::skipBackward()
{
    qint64 newTimeStamp = qMax(this->getCurrentTimestamp() - SkipMSec, 0ll);
    setCurrentTimestamp(newTimeStamp);
}

void SkyConnectDummy::skipForward()
{
    qint64 endTimeStamp = d->aircraft.getLastAircraftData().timestamp;
    qint64 newTimeStamp = qMin(this->getCurrentTimestamp() + SkipMSec, endTimeStamp);
    setCurrentTimestamp(newTimeStamp);
}

void SkyConnectDummy::skipToEnd()
{
    qint64 endTimeStamp  = d->aircraft.getLastAircraftData().timestamp;
    setCurrentTimestamp(endTimeStamp);
}

Aircraft &SkyConnectDummy::getAircraft()
{
    return d->aircraft;
}

const Aircraft &SkyConnectDummy::getAircraft() const
{
    return d->aircraft;
}

void SkyConnectDummy::setTimeScale(double timeScale)
{
    if (!qFuzzyCompare(d->timeScale, timeScale)) {
        // If the elapsed timer is running...
        if (d->elapsedTimer.isValid()) {
            // ... then store the elapsed time measured with the previous scale...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed() * d->timeScale;
            // ... and restart timer
            d->elapsedTimer.start();
        }
        d->timeScale = timeScale;
    }
}

double SkyConnectDummy::getTimeScale() const
{
    return d->timeScale;
}

void SkyConnectDummy::setCurrentTimestamp(qint64 timestamp)
{
    if (getState() != Connect::State::Recording) {
        d->currentTimestamp = timestamp;
        d->elapsedTime = d->currentTimestamp;
        if (sendAircraftPosition()) {
            emit aircraftDataSent(d->currentTimestamp);
            if (d->elapsedTimer.isValid() && getState() == Connect::State::Playback) {
                // Restart the elapsed timer, counting onwards from the newly
                // set timestamp
                d->elapsedTimer.start();
            }
        }
    }
}

qint64 SkyConnectDummy::getCurrentTimestamp() const
{
    return d->currentTimestamp;
}

bool SkyConnectDummy::isAtEnd() const
{
    return d->currentTimestamp >= d->aircraft.getLastAircraftData().timestamp;
}

const AircraftData &SkyConnectDummy::getCurrentAircraftData() const
{
    return d->currentAircraftData;
}

// PRIVATE

void SkyConnectDummy::frenchConnection()
{
    connect(&(d->timer), &QTimer::timeout,
            this, &SkyConnectDummy::processEvents);
    connect(&Settings::getInstance(), &Settings::recordSampleRateChanged,
            this, &SkyConnectDummy::handleRecordSampleRateChanged);
    connect(&Settings::getInstance(), &Settings::playbackSampleRateChanged,
            this, &SkyConnectDummy::handlePlaybackSampleRateChanged);
}

bool SkyConnectDummy::hasRecordingStarted() const
{
    return d->aircraft.getAllAircraftData().count();
}

bool SkyConnectDummy::sendAircraftPosition() const
{
    d->currentAircraftData = std::move(d->aircraft.getAircraftData(d->currentTimestamp));
    return true;
}

void SkyConnectDummy::recordData()
{
    if (d->elapsedTimer.isValid()) {
        d->currentTimestamp = d->elapsedTime + static_cast<qint64>(d->elapsedTimer.elapsed());
    }

    AircraftData aircraftData;
    aircraftData.latitude = QRandomGenerator::global()->bounded(180.0);
    aircraftData.longitude = QRandomGenerator::global()->bounded(90.0);
    aircraftData.altitude = QRandomGenerator::global()->bounded(20000.0);
    aircraftData.timestamp = d->currentTimestamp;
    d->aircraft.upsertAircraftData(std::move(aircraftData));
}

void SkyConnectDummy::replay()
{
    if (d->elapsedTimer.isValid()) {
        d->currentTimestamp = d->elapsedTime + static_cast<qint64>(d->elapsedTimer.elapsed() * d->timeScale);
    }

    if (sendAircraftPosition()) {
        emit aircraftDataSent(d->currentTimestamp);
    } else {
        stopReplay();
    }
}

// PRIVATE SLOTS

void SkyConnectDummy::processEvents()
{
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

void SkyConnectDummy::handleRecordSampleRateChanged(double sampleRateValue)
{
    d->recordSampleRate = sampleRateValue;
    d->recordIntervalMSec = static_cast<int>(1.0 / d->recordSampleRate * 1000.0);
    d->timer.setInterval(d->recordIntervalMSec);
}

void SkyConnectDummy::handlePlaybackSampleRateChanged(double sampleRateValue)
{
    d->playbackSampleRate = sampleRateValue;
    d->playbackIntervalMSec = static_cast<int>(1.0 / d->playbackSampleRate * 1000.0);
    d->timer.setInterval(d->playbackIntervalMSec);
}
