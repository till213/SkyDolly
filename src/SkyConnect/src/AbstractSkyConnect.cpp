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
#include <QtGlobal>
#include <QTimer>
#include <QElapsedTimer>

#include "../../Kernel/src/Settings.h"
#include "../../Model/src/World.h"
#include "../../Model/src/Scenario.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/AircraftData.h"
#include "../../Kernel/src/SampleRate.h"
#include "Connect.h"
#include "SkyConnectIntf.h"
#include "AbstractSkyConnect.h"

namespace
{
    // Period [ms] over which we count the recorded samples per second
    constexpr qint64 SamplesPerSecondPeriodMilliSec = 10000;

    // The amount of milliseconds to skip either forward or backward
    constexpr qint64 SkipMSec = 1000;
}

class AbstractSkyConnectPrivate
{
public:
    AbstractSkyConnectPrivate() noexcept
        : state(Connect::State::Disconnected),
          currentScenario(World::getInstance().getCurrentScenario()),
          currentTimestamp(0),
          recordSampleRate(Settings::getInstance().getRecordSampleRateValue()),
          recordIntervalMSec(SampleRate::toInterval(recordSampleRate)),
          timeScale(1.0),
          elapsedTime(0),
          lastSamplesPerSecondIndex(0)
    {
        timer.setTimerType(Qt::TimerType::PreciseTimer);
    }

    Connect::State state;
    Scenario &currentScenario;
    QTimer timer;
    qint64 currentTimestamp;
    double recordSampleRate;
    int    recordIntervalMSec;
    QElapsedTimer elapsedTimer;
    double timeScale;
    qint64 elapsedTime;
    mutable int lastSamplesPerSecondIndex;
};

// PUBLIC

AbstractSkyConnect::AbstractSkyConnect(QObject *parent) noexcept
    : SkyConnectIntf(parent),
      d(std::make_unique<AbstractSkyConnectPrivate>())
{
    frenchConnection();
}

AbstractSkyConnect::~AbstractSkyConnect() noexcept
{
}

void AbstractSkyConnect::startRecording() noexcept
{
    if (!isConnectedWithSim()) {
        connectWithSim();
    }

    if (isConnectedWithSim()) {
        setState(Connect::State::Recording);
        d->currentScenario.getUserAircraft().clear();
        d->lastSamplesPerSecondIndex = 0;
        d->currentTimestamp = 0;
        d->elapsedTimer.invalidate();
        if (!isAutoRecordSampleRate()) {
            d->timer.start(d->recordIntervalMSec);
        }
        onStartRecording();
    } else {
        setState(Connect::State::Disconnected);
    }
}

void AbstractSkyConnect::stopRecording() noexcept
{
    onStopRecording();
    d->timer.stop();
    setState(Connect::State::Connected);
}

bool AbstractSkyConnect::isRecording() const noexcept
{
    return d->state == Connect::State::Recording;
}

void AbstractSkyConnect::startReplay(bool fromStart) noexcept
{
    if (!isConnectedWithSim()) {
        connectWithSim();
    }
    if (isConnectedWithSim()) {
        setState(Connect::State::Replay);
        if (fromStart) {
            d->elapsedTime = 0;
            setCurrentTimestamp(0);
        }

        d->elapsedTimer.invalidate();
        onStartReplay(d->currentTimestamp);

    } else {
        setState(Connect::State::Disconnected);
    }
}

void AbstractSkyConnect::stopReplay() noexcept
{
    setState(Connect::State::Connected);
    d->timer.stop();
    // Remember elapsed time since last replay start, in order to continue from
    // current timestamp
    d->elapsedTime = d->currentTimestamp;
    onStopReplay();
}

bool AbstractSkyConnect::isReplaying() const noexcept
{
    return d->state == Connect::State::Replay;
}

void AbstractSkyConnect::stop() noexcept
{
    stopRecording();
    stopReplay();
}

void AbstractSkyConnect::setPaused(bool enabled) noexcept
{
    Connect::State newState;
    if (enabled) {
        switch (getState()) {
        case Connect::State::Recording:
            newState = Connect::State::RecordingPaused;
            // Store the elapsed recording time...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed();
            d->elapsedTimer.invalidate();
            setState(newState);
            onRecordingPaused(true);
            break;
        case Connect::State::Replay:
            newState = Connect::State::ReplayPaused;
            // In case the elapsed time has started (is valid)...
            if (d->elapsedTimer.isValid()) {
                // ... store the elapsed playback time measured with the current time scale...
                d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed() * d->timeScale;
                // ... and stop the elapsed timer
                d->elapsedTimer.invalidate();
            }
            setState(newState);
            onReplayPaused(true);
            break;
         default:
            // No state change
            break;
        }
    } else {
        switch (getState()) {
        case Connect::State::RecordingPaused:
            newState = Connect::State::Recording;
            if (hasRecordingStarted()) {
                // Resume recording (but only if it has already recorded samples before)
                d->elapsedTimer.start();
            }
            setState(newState);
            onRecordingPaused(false);
            break;
        case Connect::State::ReplayPaused:
            newState = Connect::State::Replay;
            d->elapsedTimer.start();
            setState(newState);
            onReplayPaused(false);
            break;
         default:
            // No state change
            break;
        }
    }    
}

bool AbstractSkyConnect::isPaused() const noexcept {
    return getState() == Connect::State::RecordingPaused || getState() == Connect::State::ReplayPaused;
}

void AbstractSkyConnect::skipToBegin() noexcept
{
    seek(0);
}

void AbstractSkyConnect::skipBackward() noexcept
{
    qint64 newTimeStamp = qMax(getCurrentTimestamp() - SkipMSec, 0ll);
    seek(newTimeStamp);
}

void AbstractSkyConnect::skipForward() noexcept
{
    qint64 totalDuration = d->currentScenario.getTotalDuration();
    qint64 newTimeStamp = qMin(getCurrentTimestamp() + SkipMSec, totalDuration);
    seek(newTimeStamp);
}

void AbstractSkyConnect::skipToEnd() noexcept
{
    qint64 totalDuration  = d->currentScenario.getTotalDuration();
    seek(totalDuration);
}

void AbstractSkyConnect::seek(qint64 timestamp) noexcept
{
    d->elapsedTime = d->currentTimestamp;
    if (getState() != Connect::State::Recording) {
        d->currentTimestamp = timestamp;
        d->elapsedTime = timestamp;
        emit timestampChanged(d->currentTimestamp, TimeVariableData::Access::Seek);
        if (sendAircraftData(timestamp, TimeVariableData::Access::Seek)) {
            if (d->elapsedTimer.isValid() && getState() == Connect::State::Replay) {
                // Restart the elapsed timer, counting onwards from the newly
                // set timestamp
                d->elapsedTimer.start();
            }
            onSeek(d->currentTimestamp);
        }
    }
}

Scenario &AbstractSkyConnect::getCurrentScenario() const
{
    return d->currentScenario;
}

qint64 AbstractSkyConnect::getCurrentTimestamp() const noexcept
{
    return d->currentTimestamp;
}

bool AbstractSkyConnect::isAtEnd() const noexcept
{
    return d->currentTimestamp >= d->currentScenario.getTotalDuration();
}

double AbstractSkyConnect::getTimeScale() const noexcept
{
    return d->timeScale;
}

Connect::State AbstractSkyConnect::getState() const noexcept
{
    return d->state;
}

bool AbstractSkyConnect::isConnected() const noexcept
{
    return d->state != Connect::State::Disconnected;
}

void AbstractSkyConnect::setTimeScale(double timeScale) noexcept
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

double AbstractSkyConnect::calculateRecordedSamplesPerSecond() const noexcept
{
    double samplesPerSecond;
    const QVector<AircraftData> aircraftData = d->currentScenario.getUserAircraftConst().getAll();
    if (aircraftData.count() > 0) {
        qint64 startTimestamp = qMin(qMax(d->currentTimestamp - SamplesPerSecondPeriodMilliSec, 0ll), aircraftData.last().timestamp);
        int index = d->lastSamplesPerSecondIndex;
        while (d->currentScenario.getUserAircraftConst().getAll().at(index).timestamp < startTimestamp) {
            ++index;
        }
        d->lastSamplesPerSecondIndex = index;

        int lastIndex = d->currentScenario.getUserAircraftConst().getAll().count() - 1;

        int nofSamples = lastIndex - index + 1;
        qint64 period = d->currentScenario.getUserAircraftConst().getAll().at(lastIndex).timestamp - d->currentScenario.getUserAircraftConst().getAll().at(index).timestamp;
        if (period > 0) {
            samplesPerSecond = static_cast<double>(nofSamples) * 1000.0 / (static_cast<double>(period));
        } else {
            samplesPerSecond = 0.0;
        }
    } else {
        samplesPerSecond = 0.0;
    }
    return samplesPerSecond;
}

// PROTECTED

void AbstractSkyConnect::setState(Connect::State state) noexcept
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state);
    }
}

void AbstractSkyConnect::setCurrentTimestamp(qint64 timestamp) noexcept
{
    d->currentTimestamp = timestamp;
}

bool AbstractSkyConnect::isElapsedTimerRunning() const noexcept
{
    return d->elapsedTimer.isValid();
}

bool AbstractSkyConnect::isAutoRecordSampleRate() const noexcept
{
    return d->recordSampleRate == SampleRate::AutoValue;
}

void AbstractSkyConnect::startElapsedTimer() const noexcept
{
     d->elapsedTimer.start();
}

void AbstractSkyConnect::resetElapsedTime(bool restart) noexcept
{
    d->elapsedTime = 0;
    if (restart) {
         d->elapsedTimer.start();
    }
}

void AbstractSkyConnect::updateCurrentTimestamp() noexcept
{
    if (d->elapsedTimer.isValid()) {
        // Ignore spontaneous SimConnect events: do not update
        // the current timestamp unless we are replaying or recording
        if (d->state == Connect::State::Replay) {
            d->currentTimestamp = d->elapsedTime + static_cast<qint64>(d->elapsedTimer.elapsed() * d->timeScale);
            emit timestampChanged(d->currentTimestamp, TimeVariableData::Access::Linear);
        } else if (d->state == Connect::State::Recording) {
            d->currentTimestamp = d->elapsedTime + d->elapsedTimer.elapsed();
            // The signal is delayed until after the latest data has been recorded,
            // by using a singleshot timer with 0 ms delay (but which is only
            // executed once execution returns to the Qt event queue)
            QTimer::singleShot(0, this, [this]() {emit timestampChanged(d->currentTimestamp, TimeVariableData::Access::Linear);});
        }        
    }
}

// PRIVATE

void AbstractSkyConnect::frenchConnection() noexcept
{
    connect(&(d->timer), &QTimer::timeout,
            this, &AbstractSkyConnect::processEvents);
    connect(&Settings::getInstance(), &Settings::recordSampleRateChanged,
            this, &AbstractSkyConnect::handleRecordSampleRateChanged);
}

bool AbstractSkyConnect::hasRecordingStarted() const noexcept
{
    return d->currentScenario.getUserAircraftConst().getAll().count();
}

// PRIVATE SLOTS

void AbstractSkyConnect::handleRecordSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept
{
    d->recordSampleRate = SampleRate::toValue(sampleRate);
    d->recordIntervalMSec = SampleRate::toInterval(d->recordSampleRate);
    if (d->state == Connect::State::Recording || d->state == Connect::State::RecordingPaused) {
        if (!isAutoRecordSampleRate()) {
            d->timer.setInterval(d->recordIntervalMSec);
            if (!d->timer.isActive()) {
                d->timer.start();
            }
        } else {
            d->timer.stop();
        }
        onRecordSampleRateChanged(sampleRate);
    }
}

