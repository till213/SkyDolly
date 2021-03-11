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

#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/AircraftData.h"
#include "Connect.h"
#include "SkyConnectIntf.h"
#include "AbstractSkyConnect.h"

namespace
{
    // Period [ms] over which we count the recorded samples per second
    constexpr qint64 SamplesPerSecondPeriodMilliSec = 5000;
    constexpr double SamplesPerSecondPeriodSec = SamplesPerSecondPeriodMilliSec / 1000.0;

    constexpr qint64 SkipMSec = 1000;
}

class AbstractSkyConnectPrivate
{
public:
    AbstractSkyConnectPrivate()
        : state(Connect::State::Idle),
          currentTimestamp(0),
          recordSampleRate(Settings::getInstance().getRecordSampleRateValue()),
          recordIntervalMSec(static_cast<int>(1.0 / recordSampleRate * 1000.0)),
          playbackSampleRate(Settings::getInstance().getPlaybackSampleRateValue()),
          playbackIntervalMSec(static_cast<int>(1.0 / playbackSampleRate * 1000.0)),
          timeScale(1.0),
          elapsedTime(0),
          lastSamplesPerSecondIndex(0)
    {
    }

    Connect::State state;
    Aircraft aircraft;
    AircraftData currentAircraftData;
    QTimer timer;
    qint64 currentTimestamp;
    double recordSampleRate;
    int    recordIntervalMSec;
    QElapsedTimer elapsedTimer;
    double playbackSampleRate;
    int    playbackIntervalMSec;
    double timeScale;
    qint64 elapsedTime;
    mutable int lastSamplesPerSecondIndex;
};

// PUBLIC

AbstractSkyConnect::AbstractSkyConnect(QObject *parent)
    : SkyConnectIntf(parent),
      d(new AbstractSkyConnectPrivate())
{
    frenchConnection();
}

AbstractSkyConnect::~AbstractSkyConnect()
{
    delete d;
}

Connect::State AbstractSkyConnect::getState() const
{
    return d->state;
}

void AbstractSkyConnect::setTimeScale(double timeScale)
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

double AbstractSkyConnect::getTimeScale() const
{
    return d->timeScale;
}

void AbstractSkyConnect::startDataSample()
{
    if (!isConnectedWithSim()) {
        connectWithSim();
    }

    if (isConnectedWithSim()) {
        setState(Connect::State::Recording);
        getAircraft().clear();
        d->lastSamplesPerSecondIndex = 0;
        d->timer.setInterval(d->recordIntervalMSec);
        d->elapsedTimer.invalidate();
        d->timer.start();
        onStartDataSample();
    } else {
        setState(Connect::State::NoConnection);
    }
}

void AbstractSkyConnect::stopDataSample()
{
    onStopDataSample();
    d->timer.stop();
    setState(Connect::State::Idle);
}

void AbstractSkyConnect::startReplay(bool fromStart)
{
    if (!isConnectedWithSim()) {
        connectWithSim();
    }
    if (isConnectedWithSim()) {
        setState(Connect::State::Playback);
        d->timer.setInterval(d->playbackIntervalMSec);

        if (fromStart) {
            d->elapsedTime = 0;
            setCurrentTimestamp(0);
        }

        d->elapsedTimer.invalidate();
        d->timer.start();

        onStartReplay(fromStart);

    } else {
        setState(Connect::State::NoConnection);
    }
}

void AbstractSkyConnect::stopReplay()
{
    setState(Connect::State::Idle);
    d->timer.stop();
    // Remember elapsed time since last replay start, in order to continue from
    // current timestamp
    d->elapsedTime = d->currentTimestamp;
    onStopReplay();
}

void AbstractSkyConnect::stop()
{
    stopDataSample();
    stopReplay();
}

void AbstractSkyConnect::setPaused(bool enabled)
{
    Connect::State newState;
    if (enabled) {
        switch (getState()) {
        case Connect::Recording:
            newState = Connect::RecordingPaused;
            // Store the elapsed recording time...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed();
            d->elapsedTimer.invalidate();
            onRecordingPaused(true);
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
            onRecordingPaused(false);
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

bool AbstractSkyConnect::isPaused() const {
    return getState() == Connect::RecordingPaused || getState() == Connect::PlaybackPaused;
}

void AbstractSkyConnect::skipToBegin()
{
    seek(0);
}

void AbstractSkyConnect::skipBackward()
{
    qint64 newTimeStamp = qMax(this->getCurrentTimestamp() - SkipMSec, 0ll);
    seek(newTimeStamp);
}

void AbstractSkyConnect::skipForward()
{
    qint64 endTimeStamp = d->aircraft.getLastAircraftData().timestamp;
    qint64 newTimeStamp = qMin(this->getCurrentTimestamp() + SkipMSec, endTimeStamp);
    seek(newTimeStamp);
}

void AbstractSkyConnect::skipToEnd()
{
    qint64 endTimeStamp  = d->aircraft.getLastAircraftData().timestamp;
    seek(endTimeStamp);
}

void AbstractSkyConnect::seek(qint64 timestamp)
{
    d->elapsedTime = d->currentTimestamp;
    if (getState() != Connect::State::Recording) {
        d->currentTimestamp = timestamp;
        if (sendAircraftData(timestamp)) {
            emit aircraftDataSent(d->currentTimestamp);
            if (d->elapsedTimer.isValid() && getState() == Connect::State::Playback) {
                // Restart the elapsed timer, counting onwards from the newly
                // set timestamp
                d->elapsedTimer.start();
            }
        }
    }
}

qint64 AbstractSkyConnect::getCurrentTimestamp() const
{
    return d->currentTimestamp;
}

bool AbstractSkyConnect::isAtEnd() const
{
    return d->currentTimestamp >= d->aircraft.getLastAircraftData().timestamp;
}

Aircraft &AbstractSkyConnect::getAircraft()
{
    return d->aircraft;
}

const Aircraft &AbstractSkyConnect::getAircraft() const
{
    return d->aircraft;
}

const AircraftData &AbstractSkyConnect::getCurrentAircraftData() const
{
    return d->currentAircraftData;
}

double AbstractSkyConnect::calculateRecordedSamplesPerSecond() const
{
    double samplesPerSecond;
    if (d->aircraft.getAllAircraftData().count() > 0) {
        qint64 startTimestamp = d->currentTimestamp - SamplesPerSecondPeriodMilliSec;
        int index = d->lastSamplesPerSecondIndex;
        while (d->aircraft.getAllAircraftData().at(index).timestamp < startTimestamp) {
            ++index;
        }
        d->lastSamplesPerSecondIndex = index;

        int lastIndex = d->aircraft.getAllAircraftData().count() - 1;

        int nofSamples = lastIndex - index + 1;
        samplesPerSecond = static_cast<double>(nofSamples) / SamplesPerSecondPeriodSec;
    } else {
        samplesPerSecond = 0.0;
    }
    return samplesPerSecond;
}

// PROTECTED

void AbstractSkyConnect::setState(Connect::State state)
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state);
    }
}

void AbstractSkyConnect::setCurrentTimestamp(qint64 timestamp)
{
    d->currentTimestamp = timestamp;
}

bool AbstractSkyConnect::isElapsedTimerRunning() const
{
    return d->elapsedTimer.isValid();
}
void AbstractSkyConnect::startElapsedTimer() const
{
     d->elapsedTimer.start();
}

void AbstractSkyConnect::resetElapsedTime(bool restart)
{
    d->elapsedTime = 0;
    if (restart) {
         d->elapsedTimer.start();
    }
}

void AbstractSkyConnect::updateCurrentTimestamp()
{
    if (d->elapsedTimer.isValid()) {
        if (d->state == Connect::State::Playback) {
            d->currentTimestamp = d->elapsedTime + static_cast<qint64>(d->elapsedTimer.elapsed() * d->timeScale);
        } else {
            d->currentTimestamp = d->elapsedTime + d->elapsedTimer.elapsed();
        }
    }
}

void AbstractSkyConnect::updateCurrentAircraftData()
{
    d->currentAircraftData = std::move(d->aircraft.getAircraftData(getCurrentTimestamp()));
}

// PRIVATE

void AbstractSkyConnect::frenchConnection()
{
    connect(&(d->timer), &QTimer::timeout,
            this, &AbstractSkyConnect::processEvents);
    connect(&Settings::getInstance(), &Settings::recordSampleRateChanged,
            this, &AbstractSkyConnect::handleRecordSampleRateChanged);
    connect(&Settings::getInstance(), &Settings::playbackSampleRateChanged,
            this, &AbstractSkyConnect::handlePlaybackSampleRateChanged);
}


bool AbstractSkyConnect::hasRecordingStarted() const
{
    return d->aircraft.getAllAircraftData().count();
}

// PRIVATE SLOTS

void AbstractSkyConnect::handleRecordSampleRateChanged(double sampleRateValue)
{
    d->recordSampleRate = sampleRateValue;
    d->recordIntervalMSec = static_cast<int>(1.0 / d->recordSampleRate * 1000.0);
    d->timer.setInterval(d->recordIntervalMSec);
}

void AbstractSkyConnect::handlePlaybackSampleRateChanged(double sampleRateValue)
{
    d->playbackSampleRate = sampleRateValue;
    d->playbackIntervalMSec = static_cast<int>(1.0 / d->playbackSampleRate * 1000.0);
    d->timer.setInterval(d->playbackIntervalMSec);
}

