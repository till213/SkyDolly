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
#include "../../Model/src/Logbook.h"
#include "../../Model/src/Flight.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/Position.h"
#include "../../Model/src/PositionData.h"
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
          currentFlight(Logbook::getInstance().getCurrentFlight()),
          currentTimestamp(0),
          recordingSampleRate(Settings::getInstance().getRecordingSampleRateValue()),
          recordingIntervalMSec(SampleRate::toIntervalMSec(recordingSampleRate)),
          replaySpeedFactor(1.0),
          elapsedTime(0),
          lastSamplesPerSecondIndex(0)
    {
        recordingTimer.setTimerType(Qt::TimerType::PreciseTimer);
    }

    Connect::State state;
    Flight &currentFlight;
    QTimer recordingTimer;
    qint64 currentTimestamp;
    double recordingSampleRate;
    int    recordingIntervalMSec;
    QElapsedTimer elapsedTimer;
    double replaySpeedFactor;
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
{}

void AbstractSkyConnect::startRecording(bool addFormationAircraft) noexcept
{
    if (!isConnectedWithSim()) {
        connectWithSim();
    }

    if (isConnectedWithSim()) {
        setState(Connect::State::Recording);
        if (!addFormationAircraft) {
            // Single flight - destroy any previous AI aircrafts
            onDestroyAIObjects();
            // Start a new flight
            d->currentFlight.clear();
            // Assign user aircraft ID
            onCreateAIObjects();
        } else {
            // Check if the current user aircraft already has a recording
            if (d->currentFlight.getUserAircraft().hasRecording()) {
                // If yes, add a new aircraft to the current flight (formation)
                d->currentFlight.addUserAircraft();
            }
            // Update AI objects by simply destroying and re-creating them
            onDestroyAIObjects();
            onCreateAIObjects();
        }
        d->lastSamplesPerSecondIndex = 0;
        d->currentTimestamp = 0;
        d->elapsedTimer.invalidate();
        if (isTimerBasedRecording(Settings::getInstance().getRecordingSampleRate())) {
            d->recordingTimer.start(d->recordingIntervalMSec);
        }
        bool ok = retryWithReconnect([this]() -> bool { return onStartRecording(); });
        if (!ok) {
            setState(Connect::State::Disconnected);
        }

    } else {
        setState(Connect::State::Disconnected);
    }
}

void AbstractSkyConnect::stopRecording() noexcept
{
    onStopRecording();
    d->recordingTimer.stop();
    setState(Connect::State::Connected);
    // Update AI objects by simply destroying and re-creating them
    onDestroyAIObjects();
    onCreateAIObjects();
    emit recordingStopped();
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
            d->currentTimestamp = 0;
        }

        d->elapsedTimer.invalidate();
        bool ok = retryWithReconnect([this]() -> bool { return onStartReplay(d->currentTimestamp); });
        if (!ok) {
            setState(Connect::State::Disconnected);
        }
    } else {
        setState(Connect::State::Disconnected);
    }
}

void AbstractSkyConnect::stopReplay() noexcept
{
    setState(Connect::State::Connected);
    d->recordingTimer.stop();
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
    if (d->state == Connect::State::Recording || d->state == Connect::State::RecordingPaused) {
        stopRecording();
    } else {
        stopReplay();
    }
}

bool AbstractSkyConnect::isActive() const noexcept
{
    return d->state != Connect::State::Disconnected && d->state != Connect::State::Connected;
}

void AbstractSkyConnect::setPaused(bool enabled) noexcept
{
    if (enabled) {
        switch (getState()) {
        case Connect::State::Recording:
            setState(Connect::State::RecordingPaused);
            // Store the elapsed recording time...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed();
            d->elapsedTimer.invalidate();
            onRecordingPaused(true);
            break;
        case Connect::State::Replay:
            setState(Connect::State::ReplayPaused);
            // In case the elapsed time has started (is valid)...
            if (d->elapsedTimer.isValid()) {
                // ... store the elapsed replay time measured with the current time scale...
                d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed() * d->replaySpeedFactor;
                // ... and stop the elapsed timer
                d->elapsedTimer.invalidate();
            }
            onReplayPaused(true);
            break;
         default:
            // No state change
            break;
        }
    } else {
        switch (getState()) {
        case Connect::State::RecordingPaused:
            setState(Connect::State::Recording);
            if (hasRecordingStarted()) {
                // Resume recording (but only if it has already recorded samples before)
                startElapsedTimer();
            }
            onRecordingPaused(false);
            break;
        case Connect::State::ReplayPaused:
            setState(Connect::State::Replay);
            startElapsedTimer();
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
    const qint64 totalDuration = d->currentFlight.getTotalDurationMSec();
    const qint64 newTimeStamp = qMin(getCurrentTimestamp() + skipMSec, totalDuration);
    seek(newTimeStamp);
}

void AbstractSkyConnect::skipToEnd() noexcept
{
    const qint64 totalDuration = d->currentFlight.getTotalDurationMSec();
    seek(totalDuration);
}

void AbstractSkyConnect::seek(qint64 timestamp) noexcept
{
    if (!isConnectedWithSim()) {
        if (connectWithSim()) {
            setState(Connect::State::Connected);
        }
    }
    if (isConnectedWithSim()) {
        d->elapsedTime = d->currentTimestamp;
        if (getState() != Connect::State::Recording) {
            d->currentTimestamp = timestamp;
            d->elapsedTime = timestamp;
            emit timestampChanged(d->currentTimestamp, TimeVariableData::Access::Seek);
            bool ok = retryWithReconnect([this, timestamp]() -> bool { return sendAircraftData(timestamp, TimeVariableData::Access::Seek); });
            if (ok) {
                if (d->elapsedTimer.isValid()) {
                    // Restart the elapsed timer, counting onwards from the newly
                    // set timestamp
                    startElapsedTimer();
                }
                onSeek(d->currentTimestamp);
            } else {
                 setState(Connect::State::Disconnected);
            }
        }
    } else {
        setState(Connect::State::Disconnected);
    }
}

Flight &AbstractSkyConnect::getCurrentFlight() const
{
    return d->currentFlight;
}

qint64 AbstractSkyConnect::getCurrentTimestamp() const noexcept
{
    return d->currentTimestamp;
}

bool AbstractSkyConnect::isAtEnd() const noexcept
{
    return d->currentTimestamp >= d->currentFlight.getTotalDurationMSec();
}

double AbstractSkyConnect::getReplaySpeedFactor() const noexcept
{
    return d->replaySpeedFactor;
}

void AbstractSkyConnect::setReplaySpeedFactor(double replaySpeedFactor) noexcept
{
    if (!qFuzzyCompare(d->replaySpeedFactor, replaySpeedFactor)) {
        // If the elapsed timer is running...
        if (d->elapsedTimer.isValid()) {
            // ... then store the elapsed time measured with the previous scale...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed() * d->replaySpeedFactor;
            // ... and restart timer
            startElapsedTimer();
        }
        d->replaySpeedFactor = replaySpeedFactor;
    }
}

Connect::State AbstractSkyConnect::getState() const noexcept
{
    return d->state;
}

bool AbstractSkyConnect::isConnected() const noexcept
{
    return d->state != Connect::State::Disconnected;
}

bool AbstractSkyConnect::isIdle() const noexcept
{
    return d->state == Connect::State::Connected || d->state == Connect::State::Disconnected;
}

double AbstractSkyConnect::calculateRecordedSamplesPerSecond() const noexcept
{
    double samplesPerSecond;
    const QVector<PositionData> &positionData = d->currentFlight.getUserAircraftConst().getPosition().getAllConst();
    if (positionData.count() > 0) {
        const qint64 startTimestamp = qMin(qMax(d->currentTimestamp - SamplesPerSecondPeriodMSec, 0ll), positionData.last().timestamp);
        int index = d->lastSamplesPerSecondIndex;

        while (positionData.at(index).timestamp < startTimestamp) {
            ++index;
        }
        d->lastSamplesPerSecondIndex = index;

        const int lastIndex = positionData.count() - 1;
        const int nofSamples = lastIndex - index + 1;
        const qint64 period = positionData.at(lastIndex).timestamp - positionData.at(index).timestamp;
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

bool AbstractSkyConnect::createAIObjects() noexcept
{
    bool ok;
    if (isConnectedWithSim()) {
        ok = onCreateAIObjects();
    } else {
        ok = true;
    }
    return ok;
}

void AbstractSkyConnect::destroyAIObjects() noexcept
{
    if (isConnected()) {
        onDestroyAIObjects();
    }
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

qint64 AbstractSkyConnect::updateCurrentTimestamp() noexcept
{
    if (d->elapsedTimer.isValid()) {
        // Ignore spontaneous SimConnect events: do not update
        // the current timestamp unless we are replaying or recording
        if (d->state == Connect::State::Replay) {
            d->currentTimestamp = d->elapsedTime + static_cast<qint64>(d->elapsedTimer.elapsed() * d->replaySpeedFactor);
            emit timestampChanged(d->currentTimestamp, TimeVariableData::Access::Linear);
        } else if (d->state == Connect::State::Recording) {
            d->currentTimestamp = d->elapsedTime + d->elapsedTimer.elapsed();
            // The signal is delayed until after the latest data has been recorded,
            // by using a singleshot timer with 0 ms delay (but which is only
            // executed once execution returns to the Qt event queue)
            QTimer::singleShot(0, this, [this]() {emit timestampChanged(d->currentTimestamp, TimeVariableData::Access::Linear);});
        }        
    }
    return d->currentTimestamp;
}

// PRIVATE

void AbstractSkyConnect::frenchConnection() noexcept
{
    connect(&(d->recordingTimer), &QTimer::timeout,
            this, &AbstractSkyConnect::recordData);
    connect(&Settings::getInstance(), &Settings::recordingSampleRateChanged,
            this, &AbstractSkyConnect::handleRecordingSampleRateChanged);
}

bool AbstractSkyConnect::hasRecordingStarted() const noexcept
{
    return d->currentFlight.getUserAircraftConst().getPosition().getAllConst().count() > 0;
}

qint64 AbstractSkyConnect::getSkipInterval() const noexcept
{
    Settings &settings = Settings::getInstance();
    return static_cast<qint64>(qRound(settings.isAbsoluteSeekEnabled() ?
                                          settings.getSeekIntervalSeconds() * 1000.0 :
                                          settings.getSeekIntervalPercent() * d->currentFlight.getTotalDurationMSec() / 100.0));
}

bool AbstractSkyConnect::retryWithReconnect(std::function<bool()> func)
{
    int nofAttempts = 2;
    bool ok = true;
    while (nofAttempts > 0) {
        ok = func();
        --nofAttempts;
        if (!ok && nofAttempts > 0) {
#ifdef DEBUG
            qDebug("AbstractSkyConnect::retryWithReconnect: previous connection is stale, RETRY with reconnect %d more time(s)...", nofAttempts);
#endif
            // Automatically reconnect in case the server crashed
            // previously (without sending a "quit" message)
            if (!connectWithSim()) {
                nofAttempts = 0;
            }
        } else {
            nofAttempts = 0;
        }
    }
    return ok;
}

// PRIVATE SLOTS

void AbstractSkyConnect::handleRecordingSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept
{
    d->recordingSampleRate = SampleRate::toValue(sampleRate);
    d->recordingIntervalMSec = SampleRate::toIntervalMSec(d->recordingSampleRate);
    if (d->state == Connect::State::Recording || d->state == Connect::State::RecordingPaused) {
        if (isTimerBasedRecording(sampleRate)) {
            d->recordingTimer.setInterval(d->recordingIntervalMSec);
            if (!d->recordingTimer.isActive()) {
                d->recordingTimer.start();
            }
        } else {
            d->recordingTimer.stop();
        }
        onRecordingSampleRateChanged(sampleRate);
    }
}
