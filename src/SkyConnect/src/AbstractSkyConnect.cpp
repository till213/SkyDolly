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
#include <QDateTime>

#include "../../Kernel/src/SampleRate.h"
#include "../../Kernel/src/Settings.h"
#include "../../Model/src/World.h"
#include "../../Model/src/Scenario.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/AircraftData.h"
#include "../../Persistence/src/Service/ScenarioService.h"
#include "Connect.h"
#include "SkyConnectIntf.h"
#include "AbstractSkyConnect.h"

namespace
{
    // Period [ms] over which we count the recorded samples per second
    constexpr qint64 SamplesPerSecondPeriodMSec = 10000;
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
          lastSamplesPerSecondIndex(0),
          scenarioService(std::make_unique<ScenarioService>())
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
    std::unique_ptr<ScenarioService> scenarioService;
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

    // Update end time
    Aircraft &userAircraft = d->currentScenario.getUserAircraft();
    AircraftInfo aircraftInfo = userAircraft.getAircraftInfoConst();
    const qint64 elapsed = d->elapsedTimer.elapsed();
    aircraftInfo.endDate = aircraftInfo.startDate.addMSecs(elapsed);
    userAircraft.setAircraftInfo(aircraftInfo);

    d->elapsedTimer.invalidate();
    setState(Connect::State::Connected);
    d->scenarioService->store(d->currentScenario);
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
    d->elapsedTimer.invalidate();
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
    if (enabled) {
        switch (getState()) {
        case Connect::State::Recording:
            // Store the elapsed recording time...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed();
            d->elapsedTimer.invalidate();
            setState(Connect::State::RecordingPaused);
            onRecordingPaused(true);
            break;
        case Connect::State::Replay:
            // In case the elapsed time has started (is valid)...
            if (d->elapsedTimer.isValid()) {
                // ... store the elapsed replay time measured with the current time scale...
                d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed() * d->timeScale;
                // ... and stop the elapsed timer
                d->elapsedTimer.invalidate();
            }
            setState(Connect::State::ReplayPaused);
            onReplayPaused(true);
            break;
         default:
            // No state change
            break;
        }
    } else {
        switch (getState()) {
        case Connect::State::RecordingPaused:
            if (hasRecordingStarted()) {
                // Resume recording (but only if it has already recorded samples before)
                startElapsedTimer();
            }
            setState(Connect::State::Recording);
            onRecordingPaused(false);
            break;
        case Connect::State::ReplayPaused:
            startElapsedTimer();
            setState(Connect::State::Replay);
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
    qint64 skipMSec = getSkipInterval();
    const qint64 newTimeStamp = qMax(getCurrentTimestamp() - skipMSec, 0ll);
    seek(newTimeStamp);
}

void AbstractSkyConnect::skipForward() noexcept
{
    qint64 skipMSec = getSkipInterval();
    const qint64 totalDuration = d->currentScenario.getTotalDurationMSec();
    const qint64 newTimeStamp = qMin(getCurrentTimestamp() + skipMSec, totalDuration);
    seek(newTimeStamp);
}

void AbstractSkyConnect::skipToEnd() noexcept
{
    const qint64 totalDuration  = d->currentScenario.getTotalDurationMSec();
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
            if (d->elapsedTimer.isValid()) {
                // Restart the elapsed timer, counting onwards from the newly
                // set timestamp
                startElapsedTimer();
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
    return d->currentTimestamp >= d->currentScenario.getTotalDurationMSec();
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
            startElapsedTimer();
        }
        d->timeScale = timeScale;
    }
}

double AbstractSkyConnect::calculateRecordedSamplesPerSecond() const noexcept
{
    double samplesPerSecond;
    const QVector<AircraftData> &aircraftData = d->currentScenario.getUserAircraftConst().getAllConst();
    if (aircraftData.count() > 0) {
        const qint64 startTimestamp = qMin(qMax(d->currentTimestamp - SamplesPerSecondPeriodMSec, 0ll), aircraftData.last().timestamp);
        int index = d->lastSamplesPerSecondIndex;
        while (d->currentScenario.getUserAircraftConst().getAllConst().at(index).timestamp < startTimestamp) {
            ++index;
        }
        d->lastSamplesPerSecondIndex = index;

        const int lastIndex = d->currentScenario.getUserAircraftConst().getAllConst().count() - 1;
        const int nofSamples = lastIndex - index + 1;
        const qint64 period = d->currentScenario.getUserAircraftConst().getAllConst().at(lastIndex).timestamp - d->currentScenario.getUserAircraftConst().getAllConst().at(index).timestamp;
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
    if (d->state == Connect::State::Replay || d->state == Connect::State::Recording) {
        d->elapsedTimer.start();
    }
}

void AbstractSkyConnect::resetElapsedTime(bool restart) noexcept
{
    d->elapsedTime = 0;
    if (restart) {
         startElapsedTimer();
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
    return d->currentScenario.getUserAircraftConst().getAllConst().count();
}

qint64 AbstractSkyConnect::getSkipInterval() const noexcept
{
    Settings &settings = Settings::getInstance();
    return static_cast<qint64>(qRound(settings.isAbsoluteSeekEnabled() ?
                                          settings.getSeekIntervalSeconds() * 1000.0 :
                                          settings.getSeekIntervalPercent() * d->currentScenario.getTotalDurationMSec() / 100.0));
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

