/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <array>
#include <memory>
#include <optional>

#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>
#include <QWidget>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/SkyMath.h>
#include <Kernel/Const.h>
#include <Kernel/SampleRate.h>
#include <Kernel/Settings.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/FlightData.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/InitialPosition.h>
#include <Connect/Connect.h>
#include <Connect/SkyConnectIntf.h>
#include <Connect/AbstractSkyConnect.h>
#include <Connect/ConnectPluginBaseSettings.h>
#include "BasicConnectOptionWidget.h"

namespace
{
    // Period [ms] over which we count the recorded samples per second
    constexpr std::int64_t SamplesPerSecondPeriod = 10000;

    // The frequency [Hertz] in which timestampChanged signals are emitted
    constexpr std::int64_t NotificationFrequency = 10;

    // The interval [milliseconds] in which timestampChanged signals are emitted
    constexpr std::int64_t NotificationInterval = 1000 / NotificationFrequency;

    // Periods to wait after each failed connection attempt - in fact, the first
    // NofRetryConnectPeriods Fibonacci numbers (starting with 0), corresponding
    // to about 90 seconds longest period (89, to be specific)
    constexpr int NofRetryConnectPeriods = 12;
}

struct AbstractSkyConnectPrivate
{
    AbstractSkyConnectPrivate() noexcept
    {
        recordingTimer.setTimerType(Qt::TimerType::PreciseTimer);
        reconnectTimer.setSingleShot(true);
        retryConnectPeriods = SkyMath::calculateFibonacci<::NofRetryConnectPeriods>(::NofRetryConnectPeriods);
#ifdef DEBUG
        qDebug() << "AbstractSkyConnectPrivate: AbstractSkyConnectPrivate: elapsed timer clock type:" << elapsedTimer.clockType();
#endif
    }

    SkyConnectIntf::ReplayMode replayMode {SkyConnectIntf::ReplayMode::Normal};
    Connect::State state {Connect::State::Disconnected};
    Flight &currentFlight {Logbook::getInstance().getCurrentFlight()};
    // Triggers the recording of sample data (if not event-based recording)
    QTimer recordingTimer;
    QTimer reconnectTimer;
    std::size_t reconnectAttempt {0};
    std::array<int, ::NofRetryConnectPeriods> retryConnectPeriods;
    std::int64_t currentTimestamp {0};
    std::int64_t lastNotificationTimestamp {0};
    double recordingSampleRate {Settings::getInstance().getRecordingSampleRateValue()};
    int recordingIntervalMSec {SampleRate::toIntervalMSec(recordingSampleRate)};
    QElapsedTimer elapsedTimer;
    double replaySpeedFactor {1.0};
    std::int64_t elapsedTime {0};
    mutable int lastSamplesPerSecondIndex {0};
};

// PUBLIC

AbstractSkyConnect::AbstractSkyConnect(QObject *parent) noexcept
    : SkyConnectIntf(parent),
      d(std::make_unique<AbstractSkyConnectPrivate>())
{
    frenchConnection();
}

AbstractSkyConnect::~AbstractSkyConnect() = default;

void AbstractSkyConnect::tryConnectAndSetup() noexcept
{
    tryFirstConnectAndSetup();
}

void AbstractSkyConnect::disconnect() noexcept
{
    onDisconnectFromSim();
    setState(Connect::State::Disconnected);
}

int AbstractSkyConnect::getRemainingReconnectTime() const noexcept
{
    return d->reconnectTimer.remainingTime();
}

bool AbstractSkyConnect::setUserAircraftInitialPosition(const InitialPosition &initialPosition) noexcept
{
    if (!isConnectedWithSim()) {
        tryFirstConnectAndSetup();
    }

    bool ok = isConnectedWithSim();
    if (ok) {
        ok = retryWithReconnect([this, initialPosition]() -> bool { return onInitialPositionSetup(initialPosition); });
    }
    return ok;
}

bool AbstractSkyConnect::freezeUserAircraft(bool enable) const noexcept
{
    return onFreezeUserAircraft(enable);
}

bool AbstractSkyConnect::sendSimulationEvent(SimulationEvent event, float arg1) noexcept
{
    if (!isConnectedWithSim()) {
        tryFirstConnectAndSetup();
    }

    bool ok = isConnectedWithSim();
    if (ok) {
        ok = retryWithReconnect([this, event, arg1]() -> bool { return onSimulationEvent(event, arg1); });
    }
    return ok;
}

SkyConnectIntf::ReplayMode AbstractSkyConnect::getReplayMode() const noexcept
{
    return d->replayMode;
}

void AbstractSkyConnect::setReplayMode(ReplayMode replayMode) noexcept
{
    if (!isInRecordingState()) {
        if (d->replayMode != replayMode) {
            switch (d->replayMode) {
            case ReplayMode::Normal:
                break;
            case ReplayMode::UserAircraftManualControl:
                break;
            case ReplayMode::FlyWithFormation:
                onRemoveAiObject(d->currentFlight.getUserAircraft().getId());
                break;
            }
            d->replayMode = replayMode;
            switch (d->replayMode) {
            case ReplayMode::Normal:
                break;
            case ReplayMode::UserAircraftManualControl:
                break;
            case ReplayMode::FlyWithFormation:
                onAddAiObject(d->currentFlight.getUserAircraft());
                break;
            }
            updateUserAircraftFreeze();
            emit replayModeChanged(d->replayMode);
        }
        sendAircraftData(d->currentTimestamp, TimeVariableData::Access::DiscreteSeek, AircraftSelection::All);
    }
}

void AbstractSkyConnect::startRecording(RecordingMode recordingMode, const InitialPosition &initialPosition) noexcept
{
    if (!isConnectedWithSim()) {
        tryFirstConnectAndSetup();
    }

    bool ok = isConnectedWithSim();
    if (ok) {
        switch (recordingMode) {
        case RecordingMode::SingleAircraft:
            // Start a new flight
            d->currentFlight.clear(true, FlightData::CreationTimeMode::Update);
            d->currentFlight.setId(Const::RecordingId);
            // Set the recording state only *after* the flight has been cleared
            setState(Connect::State::Recording);
            break;
        case RecordingMode::AddToFormation:
            // Set the recording state *before* the new user aircraft has been added, otherwise
            // it gets re-positioned relative to itself, having an invalid position
            setState(Connect::State::Recording);
            // Check if the current user aircraft already has a recording
            if (d->currentFlight.getUserAircraft().hasRecording()) {
                // If yes, add a new aircraft to the current flight (formation)
                d->currentFlight.addUserAircraft(Const::RecordingId);
            } else {
                // Start a new flight
                d->currentFlight.clear(true, FlightData::CreationTimeMode::Update);
                d->currentFlight.setId(Const::RecordingId);
            }
            break;
        }

        d->lastSamplesPerSecondIndex = 0;
        d->currentTimestamp = 0;
        d->lastNotificationTimestamp = d->currentTimestamp;
        d->elapsedTimer.invalidate();
        if (isTimerBasedRecording(Settings::getInstance().getRecordingSampleRate())) {
            d->recordingTimer.start(d->recordingIntervalMSec);
        }
        ok = retryWithReconnect([this, initialPosition]() -> bool { return setupInitialRecordingPosition(initialPosition); });
        if (ok) {
            switch (recordingMode) {
            case RecordingMode::SingleAircraft:
                ok = retryWithReconnect([this]() -> bool { return onStartFlightRecording(); });
                break;
            case RecordingMode::AddToFormation:
                ok = retryWithReconnect([this]() -> bool { return onStartAircraftRecording(); });
                break;
            }
        }
    }

    if (!ok) {
        setState(Connect::State::Disconnected);
    }
}

void AbstractSkyConnect::stopRecording() noexcept
{
    onStopRecording();
    Aircraft &aircraft = d->currentFlight.getUserAircraft();
    aircraft.invalidateDuration();
    d->recordingTimer.stop();
    // Only go into "recording stopped" state once the aircraft duration has been invalidated, in
    // order to properly update the total flight duration
    Connect::State state = isConnected() ? Connect::State::Connected : Connect::State::Disconnected;
    setState(state);

    // Create a new AI object for the newly recorded aircraft in case "fly with formation" is enabled
    if (state == Connect::State::Connected && d->replayMode == ReplayMode::FlyWithFormation) {
        onAddAiObject(d->currentFlight.getUserAircraft());
    }
}

bool AbstractSkyConnect::isRecording() const noexcept
{
    return d->state == Connect::State::Recording;
}

bool AbstractSkyConnect::isInRecordingState() const noexcept
{
    return isRecording() || d->state == Connect::State::RecordingPaused;
}

void AbstractSkyConnect::startReplay(bool fromStart, const InitialPosition &flyWithFormationPosition) noexcept
{
    if (!isConnectedWithSim()) {
        tryFirstConnectAndSetup();
    }
    if (isConnectedWithSim()) {
        setState(Connect::State::Replay);
        if (fromStart) {
            d->elapsedTime = 0;
            d->currentTimestamp = 0;   
        }
        d->lastNotificationTimestamp = d->currentTimestamp;

        d->elapsedTimer.invalidate();
        bool ok = retryWithReconnect([this]() -> bool { return onStartReplay(d->currentTimestamp); });
        if (ok) {
            ok = setupInitialReplayPosition(flyWithFormationPosition);
            if (ok) {
                ok = updateUserAircraftFreeze();
            }
            if (ok) {
                sendSimulationEvent(SimulationEvent::SimulationRate, d->replaySpeedFactor);
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
    // Reset simulation rate
    sendSimulationEvent(SimulationEvent::SimulationRate, 1.0f);
}

bool AbstractSkyConnect::isReplaying() const noexcept
{
    return d->state == Connect::State::Replay;
}

bool AbstractSkyConnect::isInReplayState() const noexcept
{
    return isReplaying() || d->state == Connect::State::ReplayPaused;
}

void AbstractSkyConnect::stop() noexcept
{
    if (isInRecordingState()) {
        stopRecording();
    } else {
        stopReplay();
    }
}

void AbstractSkyConnect::setPaused(Initiator initiator, bool enable) noexcept
{
    if (enable) {
        switch (d->state) {
        case Connect::State::Recording:
            setState(Connect::State::RecordingPaused);
            // Store the elapsed recording time...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed();
            d->elapsedTimer.invalidate();
            d->recordingTimer.stop();
            onRecordingPaused(initiator, true);
            break;
        case Connect::State::Replay:
            setState(Connect::State::ReplayPaused);
            // In case the elapsed time has started (is valid)...
            if (d->elapsedTimer.isValid()) {
                // ... store the elapsed replay time measured with the current time scale...
                d->elapsedTime = d->elapsedTime + static_cast<std::int64_t>(std::round(static_cast<double>(d->elapsedTimer.elapsed()) * d->replaySpeedFactor));
                // ... and stop the elapsed timer
                d->elapsedTimer.invalidate();
            }
            updateUserAircraftFreeze();
            onReplayPaused(initiator, true);
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
            if (isTimerBasedRecording(Settings::getInstance().getRecordingSampleRate())) {
                d->recordingTimer.start(d->recordingIntervalMSec);
            }
            onRecordingPaused(initiator, false);
            break;
        case Connect::State::ReplayPaused:
            setState(Connect::State::Replay);
            startElapsedTimer();
            updateUserAircraftFreeze();
            onReplayPaused(initiator, false);
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

bool AbstractSkyConnect::isRecordingPaused() const noexcept {
    return d->state == Connect::State::RecordingPaused;
}

void AbstractSkyConnect::skipToBegin() noexcept
{
    seek(0, SeekMode::Discrete);
}

void AbstractSkyConnect::skipBackward() noexcept
{
    std::int64_t skipMSec = getSkipInterval();
    const std::int64_t newTimeStamp = std::max(d->currentTimestamp - skipMSec, std::int64_t(0));
    seek(newTimeStamp, SeekMode::Discrete);
}

void AbstractSkyConnect::skipForward() noexcept
{
    std::int64_t skipMSec = getSkipInterval();
    const std::int64_t totalDuration = d->currentFlight.getTotalDurationMSec();
    const std::int64_t newTimeStamp = std::min(d->currentTimestamp + skipMSec, totalDuration);
    seek(newTimeStamp, SeekMode::Discrete);
}

void AbstractSkyConnect::skipToEnd() noexcept
{
    const std::int64_t totalDuration = d->currentFlight.getTotalDurationMSec();
    seek(totalDuration, SeekMode::Discrete);
}

void AbstractSkyConnect::seek(std::int64_t timestamp, SeekMode seekMode) noexcept
{
    if (!isConnectedWithSim()) {
        tryFirstConnectAndSetup();
    }
    if (isConnectedWithSim()) {
        if (d->state != Connect::State::Recording) {
            d->currentTimestamp = timestamp;
            d->lastNotificationTimestamp = d->currentTimestamp;
            d->elapsedTime = timestamp;
            const TimeVariableData::Access access = seekMode == SeekMode::Continuous ? TimeVariableData::Access::ContinuousSeek : TimeVariableData::Access::DiscreteSeek;
            emit timestampChanged(d->currentTimestamp, access);
            onSeek(d->currentTimestamp, seekMode);
            bool ok = retryWithReconnect([this, access]() -> bool { return sendAircraftData(d->currentTimestamp, access, AircraftSelection::All); });
            if (ok) {
                if (d->elapsedTimer.isValid()) {
                    // Restart the elapsed timer, counting onwards from the newly
                    // set timestamp
                    startElapsedTimer();
                }                
            } else {
                setState(Connect::State::Disconnected);
            }
        }
    } else {
        setState(Connect::State::Disconnected);
    }
}

void AbstractSkyConnect::onEndReached() noexcept
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

bool AbstractSkyConnect::isActive() const noexcept
{
    return !isIdle();
}

std::int64_t AbstractSkyConnect::getCurrentTimestamp() const noexcept
{
    return d->currentTimestamp;
}

bool AbstractSkyConnect::isEndReached() const noexcept
{
    return d->currentTimestamp >= d->currentFlight.getTotalDurationMSec();
}

double AbstractSkyConnect::getReplaySpeedFactor() const noexcept
{
    return d->replaySpeedFactor;
}

void AbstractSkyConnect::setReplaySpeedFactor(double factor) noexcept
{
    if (!qFuzzyCompare(d->replaySpeedFactor, factor)) {
        // If the elapsed timer is running...
        if (d->elapsedTimer.isValid()) {
            // ... then store the elapsed time measured with the previous scale...
            d->elapsedTime = d->elapsedTime + static_cast<std::int64_t>(std::round(static_cast<double>(d->elapsedTimer.elapsed()) * d->replaySpeedFactor));
            // ... and restart timer
            startElapsedTimer();
        }
        d->replaySpeedFactor = factor;
        if (isInReplayState()) {
            sendSimulationEvent(SimulationEvent::SimulationRate, d->replaySpeedFactor);
        }
    }
}

double AbstractSkyConnect::calculateRecordedSamplesPerSecond() const noexcept
{
    double samplesPerSecond {0.0};
    const Position &position = d->currentFlight.getUserAircraft().getPosition();
    if (position.count() > 0) {
        const std::int64_t startTimestamp = std::min(std::max(d->currentTimestamp - SamplesPerSecondPeriod, std::int64_t(0)), position.getLast().timestamp);
        int index = d->lastSamplesPerSecondIndex;

        while (position[index].timestamp < startTimestamp) {
            ++index;
        }
        d->lastSamplesPerSecondIndex = index;

        const std::size_t lastIndex = position.count() - 1;
        const std::size_t nofSamples = lastIndex - index + 1;
        const std::int64_t period = position[lastIndex].timestamp - position[index].timestamp;
        if (period > 0) {
            samplesPerSecond = static_cast<double>(nofSamples) * 1000.0 / (static_cast<double>(period));
        }
    }
    return samplesPerSecond;
}

bool AbstractSkyConnect::requestLocation() noexcept
{
    if (!isConnectedWithSim()) {
        tryFirstConnectAndSetup();
    }

    bool ok = isConnectedWithSim();
    if (ok) {
        ok = retryWithReconnect([this]() -> bool { return onRequestLocation(); });
    }
    return ok;
}

std::optional<std::unique_ptr<OptionWidgetIntf>> AbstractSkyConnect::createOptionWidget() const noexcept
{
    auto optionWidget = std::make_unique<BasicConnectOptionWidget>(getPluginSettings());
    auto extendedOptionWidget = createExtendedOptionWidget();
    if (extendedOptionWidget) {
        // Transfer ownership to optionWidget
        optionWidget->setExtendedOptionWidget(extendedOptionWidget->release());
    }
    return optionWidget;
}

// PUBLIC SLOTS

void AbstractSkyConnect::addAiObject(const Aircraft &aircraft) noexcept
{
    if (isConnected()) {
        onAddAiObject(aircraft);
    }
}

void AbstractSkyConnect::removeAiObjects() noexcept
{
    if (isConnected()) {
        onRemoveAllAiObjects();
    }
}

void AbstractSkyConnect::removeAiObject(std::int64_t removedAircraftId) noexcept
{
    if (isConnected()) {
        onRemoveAiObject(removedAircraftId);
    }
}

void AbstractSkyConnect::syncAiObjectsWithFlight() noexcept
{
    removeAiObjects();
    createAiObjects();
}

void AbstractSkyConnect::updateUserAircraft(int newUserAircraftIndex, int previousUserAircraftIndex) noexcept
{
    // Update the user aircraft when recording or replaying, except when "fly with formation" is selected
    // and the user controls the user aircraft (hence no AI object is to be generated)
    if (d->replayMode != ReplayMode::FlyWithFormation) {
        // Check if the new user aircraft has just been newly added, which is indicated
        // by an invalid ID (= it has never been persisted). In such a case it does not
        // have an associated AI object either
        const Aircraft &userAircraft = d->currentFlight[newUserAircraftIndex];
        if (userAircraft.getId() != Const::InvalidId) {
            removeAiObject(userAircraft.getId());
        }
        if (previousUserAircraftIndex != Const::InvalidIndex) {
            const Aircraft &aircraft = d->currentFlight[previousUserAircraftIndex];
            addAiObject(aircraft);
        }
        sendAircraftData(d->currentTimestamp, TimeVariableData::Access::DiscreteSeek, AircraftSelection::UserAircraft);
    }
}

void AbstractSkyConnect::onTimeOffsetChanged() noexcept
{
    // Only send the updated positions (due to a time offset change) when replay is paused
    if (getState() == Connect::State::ReplayPaused) {
        seek(getCurrentTimestamp(), SeekMode::Discrete);
    }
}

void AbstractSkyConnect::onTailNumberChanged(const Aircraft &aircraft) noexcept
{
    onRemoveAiObject(aircraft.getId());
    onAddAiObject(aircraft);
    // Only send the updated positions (due to a time offset change) when replay is paused
    if (getState() == Connect::State::ReplayPaused) {
        seek(getCurrentTimestamp(), SeekMode::Discrete);
    }
}

// PROTECTED

void AbstractSkyConnect::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    getPluginSettings().addSettings(keyValues);
}

void AbstractSkyConnect::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    getPluginSettings().addKeysWithDefaults(keysWithDefaults);
}

void AbstractSkyConnect::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    getPluginSettings().restoreSettings(valuesByKey);
}

void AbstractSkyConnect::setState(Connect::State state) noexcept
{
    if (d->state != state) {
        const bool previousRecordingState = isInRecordingState();
        d->state = state;
        emit stateChanged(state);
        // Recording started or stopped?
        if (!previousRecordingState && isInRecordingState()) {
            emit recordingStarted();
        } else if (previousRecordingState && !isInRecordingState()) {
            emit recordingStopped();
        }
    }
}

void AbstractSkyConnect::setCurrentTimestamp(std::int64_t timestamp) noexcept
{
    d->currentTimestamp = timestamp;
    d->lastNotificationTimestamp = d->currentTimestamp;
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

void AbstractSkyConnect::createAiObjects() noexcept
{
    if (isConnectedWithSim()) {
        // When "fly with formation" is enabled we also create an AI aircraft for the user aircraft
        // (the user aircraft of the recorded aircraft in the formation, that is)
        const bool includingUserAircraft = getReplayMode() == ReplayMode::FlyWithFormation;
        const std::int64_t userAircraftId = d->currentFlight.getUserAircraft().getId();
        for (const auto &aircraft : d->currentFlight) {
            if (aircraft.getId() != userAircraftId || includingUserAircraft) {
                onAddAiObject(aircraft);
            }
        }
    }
}

// PROTECTED SLOTS

std::int64_t AbstractSkyConnect::updateCurrentTimestamp() noexcept
{
    if (d->elapsedTimer.isValid()) {
        // Ignore spontaneous SimConnect events: do not update
        // the current timestamp unless we are replaying or recording
        if (d->state == Connect::State::Replay) {
            d->currentTimestamp = d->elapsedTime + static_cast<std::int64_t>(std::round(static_cast<double>(d->elapsedTimer.elapsed()) * d->replaySpeedFactor));
            if (d->currentTimestamp == 0 || d->currentTimestamp - d->lastNotificationTimestamp > ::NotificationInterval) {
                d->lastNotificationTimestamp = d->currentTimestamp;
                emit timestampChanged(d->currentTimestamp, TimeVariableData::Access::Linear);
            }
        } else if (d->state == Connect::State::Recording) {
            d->currentTimestamp = d->elapsedTime + d->elapsedTimer.elapsed();
            if (d->currentTimestamp == 0 || d->currentTimestamp - d->lastNotificationTimestamp > ::NotificationInterval) {
                d->lastNotificationTimestamp = d->currentTimestamp;
                // The signal is delayed until after the latest data has been recorded,
                // by using a singleshot timer with 0 ms delay (but which is only
                // executed once execution returns to the Qt event queue)
                QTimer::singleShot(0, this, [this]() {emit timestampChanged(d->currentTimestamp, TimeVariableData::Access::Linear);});
            }
        }
    }
    return d->currentTimestamp;
}

void AbstractSkyConnect::onPluginSettingsChanged(Connect::Mode mode) noexcept
{
    switch (mode)
    {
    case Connect::Mode::Reconnect:
        d->reconnectAttempt = 0;
        retryConnectAndSetup(mode);
        break;
    case Connect::Mode::SetupOnly:
        d->reconnectAttempt = 0;
        retryConnectAndSetup(mode);
        break;
    default:
        break;
    }
}

// PRIVATE

void AbstractSkyConnect::frenchConnection() noexcept
{
    connect(&(d->recordingTimer), &QTimer::timeout,
            this, &AbstractSkyConnect::recordData);
    connect(&(d->reconnectTimer), &QTimer::timeout,
            this, &AbstractSkyConnect::onReconnectTimer);
    Settings &settings = Settings::getInstance();
    connect(&settings, &Settings::recordingSampleRateChanged,
            this, &AbstractSkyConnect::onRecordingSampleRateSettingsChanged);
}

bool AbstractSkyConnect::hasRecordingStarted() const noexcept
{
    return d->currentFlight.getUserAircraft().getPosition().count() > 0;
}

std::int64_t AbstractSkyConnect::getSkipInterval() const noexcept
{
    Settings &settings = Settings::getInstance();
    return static_cast<std::int64_t>(std::round(settings.isAbsoluteSeekEnabled() ?
                                     settings.getSeekIntervalSeconds() * 1000.0 :
                                     settings.getSeekIntervalPercent() * d->currentFlight.getTotalDurationMSec() / 100.0));
}

void AbstractSkyConnect::tryFirstConnectAndSetup() noexcept
{
    d->reconnectAttempt = 0;
    retryConnectAndSetup(Connect::Mode::SetupOnly);
}

bool AbstractSkyConnect::retryWithReconnect(const std::function<bool()> &func)
{
    int nofAttempts {2};
    bool ok {false};
    while (!ok && nofAttempts > 0) {
        ok = func();
        if (!ok && nofAttempts > 0) {
#ifdef DEBUG
            qDebug() << "AbstractSkyConnect::retryWithReconnect: previous connection is stale, RETRY with reconnect" << nofAttempts << "more time(s)...";
#endif
            // Automatically reconnect in case the server crashed
            // previously (without sending a "quit" message)
            connectWithSim();
            --nofAttempts;
        }
    }
    return ok;
}

bool AbstractSkyConnect::setupInitialRecordingPosition(InitialPosition initialPosition) noexcept
{
    bool ok {true};
    if (!initialPosition.isNull()) {
        // Set initial recording position
        ok = onInitialPositionSetup(initialPosition);
    }
    return ok;
}

bool AbstractSkyConnect::setupInitialReplayPosition(InitialPosition flyWithFormationPosition) noexcept
{
    bool ok {true};
    switch (d->replayMode) {
    case ReplayMode::FlyWithFormation:
        if (!flyWithFormationPosition.isNull()) {
            ok = onInitialPositionSetup(flyWithFormationPosition);
        }
        break;
    case ReplayMode::UserAircraftManualControl:
        [[fallthrough]];
    case ReplayMode::Normal:
        if (d->currentTimestamp == 0) {
            const Aircraft &userAircraft = getCurrentFlight().getUserAircraft();
            // Make sure recorded position data exists
            ok = userAircraft.getPosition().count() > 0;
            if (ok) {
                const PositionData &positionData = userAircraft.getPosition().getFirst();
                const AircraftInfo aircraftInfo = userAircraft.getAircraftInfo();
                const InitialPosition initialPosition = InitialPosition(positionData, aircraftInfo);
                ok = onInitialPositionSetup(initialPosition);
            }
        }
        break;
    }
    return ok;
}

bool AbstractSkyConnect::updateUserAircraftFreeze() noexcept
{
    bool freeze {false};
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
    }
    return onFreezeUserAircraft(freeze);
}

// PRIVATE SLOTS

void AbstractSkyConnect::onRecordingSampleRateSettingsChanged(SampleRate::SampleRate sampleRate) noexcept
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

void AbstractSkyConnect::onReconnectTimer() noexcept
{
    retryConnectAndSetup(Connect::Mode::SetupOnly);
}

void AbstractSkyConnect::retryConnectAndSetup(Connect::Mode mode) noexcept
{
    d->reconnectTimer.stop();
    if (mode == Connect::Mode::Reconnect) {
        disconnect();
    }
    if (!isConnectedWithSim()) {
        connectWithSim();
    }

    bool ok = isConnectedWithSim();
    if (ok && getPluginSettings().getFlightSimulatorShortcuts().hasAny()) {
        ok = retryWithReconnect([this]() -> bool { return onSetupFlightSimulatorShortcuts(); });   
    }
    if (ok) {
        setState(Connect::State::Connected);
    } else {
        // Try later, with progressively increasing retry periods
        setState(Connect::State::Disconnected);
        const auto attempt = std::min(d->retryConnectPeriods.size() - 1, d->reconnectAttempt);
        const auto period = d->retryConnectPeriods[attempt];
#ifdef DEBUG
        qDebug() << "AbstractSkyConnectPrivate: retryConnectAndSetup: attempt:" << (d->reconnectAttempt + 1)
                 << "failed, trying again in" << period << "seconds";
#endif
        d->reconnectAttempt++;
        d->reconnectTimer.start(period * 1000);
    }
}
