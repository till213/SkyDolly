/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <cstdint>

#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>

#include <Kernel/SampleRate.h>
#include <Kernel/Settings.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/InitialPosition.h>
#include "Connect.h"
#include "SkyConnectIntf.h"
#include "AbstractSkyConnect.h"

namespace
{
    // Period [ms] over which we count the recorded samples per second
    constexpr std::int64_t SamplesPerSecondPeriodMSec = 10000;
}

class AbstractSkyConnectPrivate
{
public:
    AbstractSkyConnectPrivate() noexcept
        : replayMode(SkyConnectIntf::ReplayMode::Normal),
          state(Connect::State::Disconnected),
          currentFlight(Logbook::getInstance().getCurrentFlight()),
          currentTimestamp(0),
          recordingSampleRate(Settings::getInstance().getRecordingSampleRateValue()),
          recordingIntervalMSec(SampleRate::toIntervalMSec(recordingSampleRate)),
          replaySpeedFactor(1.0),
          elapsedTime(0),
          lastSamplesPerSecondIndex(0)
    {
        recordingTimer.setTimerType(Qt::TimerType::PreciseTimer);
#ifdef DEBUG
        qDebug("AbstractSkyConnectPrivate: AbstractSkyConnectPrivate: elapsed timer clock type: %d", elapsedTimer.clockType());
#endif
    }

    SkyConnectIntf::ReplayMode replayMode;
    Connect::State state;
    Flight &currentFlight;
    QTimer recordingTimer;
    std::int64_t currentTimestamp;
    double recordingSampleRate;
    int    recordingIntervalMSec;
    QElapsedTimer elapsedTimer;
    double replaySpeedFactor;
    std::int64_t elapsedTime;
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

bool AbstractSkyConnect::setUserAircraftInitialPosition(const InitialPosition &initialPosition) noexcept
{
    return onInitialPositionSetup(initialPosition);
}

bool AbstractSkyConnect::freezeUserAircraft(bool enable) noexcept
{
    return onFreezeUserAircraft(enable);
}

SkyConnectIntf::ReplayMode AbstractSkyConnect::getReplayMode() const noexcept
{
    return d->replayMode;
}

void AbstractSkyConnect::setReplayMode(ReplayMode replayMode) noexcept
{
    if (d->replayMode != replayMode) {
        d->replayMode = replayMode;
        updateAIObjects();
        updateUserAircraftFreeze();
    }
}

void AbstractSkyConnect::startRecording(RecordingMode recordingMode, const InitialPosition &initialPosition) noexcept
{
    if (!isConnectedWithSim()) {
        connectWithSim();
    }

    if (isConnectedWithSim()) {
        setState(Connect::State::Recording);
        switch (recordingMode) {
        case RecordingMode::SingleAircraft:
            // Single flight - destroy any previous AI aircraft
            onDestroyAIObjects();
            // Start a new flight
            d->currentFlight.clear(true);
            // Assign user aircraft ID
            onCreateAIObjects();
            break;
        case RecordingMode::AddToFormation:
            // Check if the current user aircraft already has a recording
            if (d->currentFlight.getUserAircraft().hasRecording()) {
                // If yes, add a new aircraft to the current flight (formation)
                d->currentFlight.addUserAircraft();
            }
            // Update AI objects
            updateAIObjects();
            break;
        }
        d->lastSamplesPerSecondIndex = 0;
        d->currentTimestamp = 0;
        d->elapsedTimer.invalidate();
        if (isTimerBasedRecording(Settings::getInstance().getRecordingSampleRate())) {
            d->recordingTimer.start(d->recordingIntervalMSec);
        }
        const bool ok = retryWithReconnect([this, initialPosition]() -> bool { return setupInitialRecordingPosition(initialPosition); });
        if (ok) {
            onStartRecording();
        }
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
    emit recordingStopped();
}

bool AbstractSkyConnect::isRecording() const noexcept
{
    return d->state == Connect::State::Recording;
}

void AbstractSkyConnect::startReplay(bool fromStart, const InitialPosition &flyWithFormationPosition) noexcept
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
        if (ok) {
            ok = setupInitialReplayPosition(flyWithFormationPosition);
            if (ok) {
                ok = updateUserAircraftFreeze();
            }
            if (!ok) {
                stopReplay();
            }
        } else {
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
    updateUserAircraftFreeze();
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

bool AbstractSkyConnect::inRecordingMode() const noexcept
{
    return isRecording() || d->state == Connect::State::RecordingPaused;
}

bool AbstractSkyConnect::inReplayMode() const noexcept
{
    return isReplaying() || d->state == Connect::State::ReplayPaused;
}

bool AbstractSkyConnect::isActive() const noexcept
{
    return d->state != Connect::State::Disconnected && d->state != Connect::State::Connected;
}

void AbstractSkyConnect::setPaused(bool enabled) noexcept
{
    if (enabled) {
        switch (d->state) {
        case Connect::State::Recording:
            setState(Connect::State::RecordingPaused);
            // Store the elapsed recording time...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed();
            d->elapsedTimer.invalidate();
            d->recordingTimer.stop();
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
            updateUserAircraftFreeze();
            onReplayPaused(true);
            break;
         default:
            // No state change
            break;
        }
    } else {
        switch (d->state) {
        case Connect::State::RecordingPaused:
            setState(Connect::State::Recording);
            if (hasRecordingStarted()) {
                // Resume recording (but only if it has already recorded samples before)
                startElapsedTimer();
            }
            d->recordingTimer.start();
            onRecordingPaused(false);
            break;
        case Connect::State::ReplayPaused:
            setState(Connect::State::Replay);
            startElapsedTimer();
            updateUserAircraftFreeze();
            onReplayPaused(false);
            break;
         default:
            // No state change
            break;
        }
    }    
}

bool AbstractSkyConnect::isPaused() const noexcept {
    return d->state == Connect::State::RecordingPaused || d->state == Connect::State::ReplayPaused;
}

void AbstractSkyConnect::skipToBegin() noexcept
{
    seek(0);
}

void AbstractSkyConnect::skipBackward() noexcept
{
    std::int64_t skipMSec = getSkipInterval();
    const std::int64_t newTimeStamp = qMax(getCurrentTimestamp() - skipMSec, std::int64_t(0));
    seek(newTimeStamp);
}

void AbstractSkyConnect::skipForward() noexcept
{
    std::int64_t skipMSec = getSkipInterval();
    const std::int64_t totalDuration = d->currentFlight.getTotalDurationMSec();
    const std::int64_t newTimeStamp = qMin(getCurrentTimestamp() + skipMSec, totalDuration);
    seek(newTimeStamp);
}

void AbstractSkyConnect::skipToEnd() noexcept
{
    const std::int64_t totalDuration = d->currentFlight.getTotalDurationMSec();
    seek(totalDuration);
}

void AbstractSkyConnect::seek(std::int64_t timestamp) noexcept
{
    if (!isConnectedWithSim()) {
        if (connectWithSim()) {
            setState(Connect::State::Connected);
        }
    }
    if (isConnectedWithSim()) {
        if (d->state != Connect::State::Recording) {
            d->currentTimestamp = timestamp;
            d->elapsedTime = timestamp;
            emit timestampChanged(d->currentTimestamp, TimeVariableData::Access::Seek);
            bool ok = retryWithReconnect([this, timestamp]() -> bool { return sendAircraftData(timestamp, TimeVariableData::Access::Seek, AircraftSelection::All); });
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

void AbstractSkyConnect::handleAtEnd() noexcept
{
    if (Settings::getInstance().isReplayLoopEnabled()) {
        skipToBegin();
    } else {
        stopReplay();
    }
}

Flight &AbstractSkyConnect::getCurrentFlight() const
{
    return d->currentFlight;
}

std::int64_t AbstractSkyConnect::getCurrentTimestamp() const noexcept
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
    const Position &position = d->currentFlight.getUserAircraftConst().getPosition();
    if (position.count() > 0) {
        const std::int64_t startTimestamp = qMin(qMax(d->currentTimestamp - SamplesPerSecondPeriodMSec, std::int64_t(0)), position.getLast().timestamp);
        int index = d->lastSamplesPerSecondIndex;

        while (position[index].timestamp < startTimestamp) {
            ++index;
        }
        d->lastSamplesPerSecondIndex = index;

        const int lastIndex = position.count() - 1;
        const int nofSamples = lastIndex - index + 1;
        const std::int64_t period = position[lastIndex].timestamp - position[index].timestamp;
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

void AbstractSkyConnect::destroyAIObject(Aircraft &aircraft) noexcept
{
    if (isConnected()) {
        onDestroyAIObject(aircraft);
    }
}

bool AbstractSkyConnect::updateAIObjects() noexcept
{
    destroyAIObjects();
    bool ok = createAIObjects();
    return ok;
}

bool AbstractSkyConnect::updateUserAircraft() noexcept
{
    bool ok = updateAIObjects();
    if (ok) {
        ok = sendAircraftData(d->currentTimestamp, TimeVariableData::Access::Seek, AircraftSelection::UserAircraft);
    }
    return ok;
}

// PROTECTED

void AbstractSkyConnect::setState(Connect::State state) noexcept
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state);
    }
}

void AbstractSkyConnect::setCurrentTimestamp(std::int64_t timestamp) noexcept
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

std::int64_t AbstractSkyConnect::updateCurrentTimestamp() noexcept
{
    if (d->elapsedTimer.isValid()) {
        // Ignore spontaneous SimConnect events: do not update
        // the current timestamp unless we are replaying or recording
        if (d->state == Connect::State::Replay) {
            d->currentTimestamp = d->elapsedTime + static_cast<std::int64_t>(d->elapsedTimer.elapsed() * d->replaySpeedFactor);
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
    return d->currentFlight.getUserAircraftConst().getPosition().count() > 0;
}

std::int64_t AbstractSkyConnect::getSkipInterval() const noexcept
{
    Settings &settings = Settings::getInstance();
    return static_cast<std::int64_t>(qRound(settings.isAbsoluteSeekEnabled() ?
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

bool AbstractSkyConnect::setupInitialRecordingPosition(const InitialPosition &initialPosition) noexcept
{
    bool ok;
    if (!initialPosition.isNull()) {
        // Set initial recording position
        ok = onInitialPositionSetup(initialPosition);
    } else {
        ok = true;
    }
    return ok;
}

bool AbstractSkyConnect::setupInitialReplayPosition(const InitialPosition &flyWithFormationPosition) noexcept
{
    bool ok;
    switch (d->replayMode) {
    case ReplayMode::FlyWithFormation:
        if (!flyWithFormationPosition.isNull()) {
            ok = onInitialPositionSetup(flyWithFormationPosition);
        } else {
            // No initial position given
            ok = true;
        }
        break;
    default:
        if (d->currentTimestamp == 0) {
            const Aircraft &userAircraft = getCurrentFlight().getUserAircraftConst();
            const PositionData &positionData = userAircraft.getPositionConst().getFirst();
            // Make sure recorded position data exists
            ok = !positionData.isNull();
            if (ok) {
                const AircraftInfo aircraftInfo = userAircraft.getAircraftInfoConst();
                const InitialPosition initialPosition = InitialPosition(positionData, aircraftInfo);
                ok = onInitialPositionSetup(initialPosition);
            }
        } else {
            // Not at beginning of replay
            ok = true;
        }
        break;
    }
    return ok;
}

bool AbstractSkyConnect::updateUserAircraftFreeze() noexcept
{
    bool freeze;
    switch (d->replayMode) {
    case ReplayMode::Normal:
        freeze = d->state == Connect::State::Replay || d->state == Connect::State::ReplayPaused;
        break;
    case ReplayMode::UserAircraftManualControl:
        freeze = d->state == Connect::State::ReplayPaused;
        break;
    case ReplayMode::FlyWithFormation:
        freeze = d->state == Connect::State::ReplayPaused;
        break;
    default:
        freeze = false;
        break;
    }
    return onFreezeUserAircraft(freeze);
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
