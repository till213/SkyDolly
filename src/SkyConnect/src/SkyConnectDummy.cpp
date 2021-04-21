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
#include <memory>

#include <QTimer>
#include <QtGlobal>
#include <QRandomGenerator>

#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/SkyMath.h"
#include "../../Model/src/TimeVariableData.h"
#include "../../Model/src/World.h"
#include "../../Model/src/Scenario.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/AircraftData.h"
#include "../../Model/src/Engine.h"
#include "../../Model/src/EngineData.h"
#include "../../Model/src/FlightCondition.h"
#include "../../Model/src/SimType.h"
#include "AbstractSkyConnect.h"
#include "SkyConnectDummy.h"

namespace {
    // Hz
    constexpr int ReplayRate = 60;
    constexpr int ReplayPeriod = qRound(1000.0 / ReplayRate);
}

class SkyConnectDummyPrivate
{
public:
    SkyConnectDummyPrivate() noexcept
    {
    }

    QTimer replayTimer;
};

// PUBLIC

SkyConnectDummy::SkyConnectDummy(QObject *parent) noexcept
    : AbstractSkyConnect(parent),
      d(std::make_unique<SkyConnectDummyPrivate>())
{
    frenchConnection();
}

SkyConnectDummy::~SkyConnectDummy() noexcept
{
}

// PROTECTED

void SkyConnectDummy::onStartRecording() noexcept
{
    recordFlightCondition();
    recordAircraftInfo();
}

void SkyConnectDummy::onRecordingPaused(bool paused) noexcept
{
    Q_UNUSED(paused)
}

void SkyConnectDummy::onStopRecording() noexcept
{
}

void SkyConnectDummy::onStartReplay(qint64 currentTimestamp) noexcept {
    Q_UNUSED(currentTimestamp)
    d->replayTimer.start(ReplayPeriod);
}

void SkyConnectDummy::onReplayPaused(bool paused) noexcept
{
    if (paused) {
         d->replayTimer.stop();
    } else {
        d->replayTimer.start(ReplayPeriod);
    }
}

void SkyConnectDummy::onStopReplay() noexcept
{
    d->replayTimer.stop();
}

void SkyConnectDummy::onSeek(qint64 currentTimestamp) noexcept
{
    Q_UNUSED(currentTimestamp)
};

void SkyConnectDummy::onRecordSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept
{
    Q_UNUSED(sampleRate)
}

bool SkyConnectDummy::sendAircraftData(qint64 currentTimestamp, TimeVariableData::Access access) noexcept
{
    Q_UNUSED(currentTimestamp)
    return sendAircraftData(access);
}

bool SkyConnectDummy::isConnectedWithSim() const noexcept
{
    return true;
}

bool SkyConnectDummy::connectWithSim() noexcept
{
    return true;
}

// PROTECTED SLOTS

void SkyConnectDummy::processEvents() noexcept
{
    updateCurrentTimestamp();
    switch (getState()) {
    case Connect::State::Recording:
        recordData();
        break;
    case Connect::State::Replay:
        replay();
        break;
    default:
        break;
    }
}

// PRIVATE

void SkyConnectDummy::frenchConnection() noexcept
{
    connect(&d->replayTimer, &QTimer::timeout,
            this, &SkyConnectDummy::processEvents);
}

bool SkyConnectDummy::sendAircraftData(TimeVariableData::Access access) noexcept
{
    bool success;

    const AircraftData &currentAircraftData = getCurrentScenario().getUserAircraftConst().interpolate(getCurrentTimestamp(), access);
    if (!currentAircraftData.isNull()) {
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

void SkyConnectDummy::recordData() noexcept
{
    const qint64 timestamp = getCurrentTimestamp();
    Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();
    QRandomGenerator *randomGenerator = QRandomGenerator::global();

    AircraftData aircraftData;
    aircraftData.latitude = -180.0 + randomGenerator->bounded(360.0);
    aircraftData.longitude = -90.0 + randomGenerator->bounded(180.0);
    aircraftData.altitude = randomGenerator->bounded(20000.0);
    aircraftData.pitch = -90.0 + randomGenerator->bounded(180.0);
    aircraftData.bank = -180.0 + randomGenerator->bounded(360.0);
    aircraftData.heading = -180.0 + randomGenerator->bounded(360.0);

    aircraftData.rotationVelocityBodyX = randomGenerator->bounded(1.0);
    aircraftData.rotationVelocityBodyY = randomGenerator->bounded(1.0);
    aircraftData.rotationVelocityBodyZ = randomGenerator->bounded(1.0);
    aircraftData.velocityBodyX = randomGenerator->bounded(1.0);
    aircraftData.velocityBodyY = randomGenerator->bounded(1.0);
    aircraftData.velocityBodyZ = randomGenerator->bounded(1.0);

    aircraftData.timestamp = timestamp;
    aircraft.upsert(std::move(aircraftData));

    EngineData engineData;
    engineData.throttleLeverPosition1 = SkyMath::fromPosition(-1.0 + randomGenerator->bounded(2.0));
    engineData.throttleLeverPosition2 = SkyMath::fromPosition(-1.0 + randomGenerator->bounded(2.0));
    engineData.throttleLeverPosition3 = SkyMath::fromPosition(-1.0 + randomGenerator->bounded(2.0));
    engineData.throttleLeverPosition4 = SkyMath::fromPosition(-1.0 + randomGenerator->bounded(2.0));
    engineData.propellerLeverPosition1 = SkyMath::fromPosition(randomGenerator->bounded(1.0));
    engineData.propellerLeverPosition2 = SkyMath::fromPosition(randomGenerator->bounded(1.0));
    engineData.propellerLeverPosition3 = SkyMath::fromPosition(randomGenerator->bounded(1.0));
    engineData.propellerLeverPosition4 = SkyMath::fromPosition(randomGenerator->bounded(1.0));
    engineData.mixtureLeverPosition1 = SkyMath::fromPercent(randomGenerator->bounded(100.0));
    engineData.mixtureLeverPosition2 = SkyMath::fromPercent(randomGenerator->bounded(100.0));
    engineData.mixtureLeverPosition3 = SkyMath::fromPercent(randomGenerator->bounded(100.0));
    engineData.mixtureLeverPosition4 = SkyMath::fromPercent(randomGenerator->bounded(100.0));
    engineData.timestamp = timestamp;
    aircraft.getEngine().upsert(std::move(engineData));

    if (!isElapsedTimerRunning()) {
        // Start the elapsed timer with the arrival of the first sample data
        setCurrentTimestamp(0);
        resetElapsedTime(true);
    }
}

void SkyConnectDummy::recordFlightCondition() noexcept
{
    QRandomGenerator *randomGenerator = QRandomGenerator::global();
    Scenario &scenario = World::getInstance().getCurrentScenario();
    FlightCondition flightCondition;

    flightCondition.groundAltitude = randomGenerator->bounded(4000);
    flightCondition.surfaceType = static_cast<SimType::SurfaceType>(randomGenerator->bounded(26));
    flightCondition.ambientTemperature = randomGenerator->bounded(80.0) - 40.0;
    flightCondition.totalAirTemperature = randomGenerator->bounded(80.0) - 40.0;
    flightCondition.windVelocity = randomGenerator->bounded(30.0);
    flightCondition.windDirection = randomGenerator->bounded(360);
    flightCondition.precipitationState = static_cast<SimType::PrecipitationState>(randomGenerator->bounded(4)); 
    flightCondition.visibility = randomGenerator->bounded(10000.0);
    flightCondition.seaLevelPressure = 950.0 + randomGenerator->bounded(100.0);
    flightCondition.pitotIcingPercent = randomGenerator->bounded(101);
    flightCondition.structuralIcingPercent = randomGenerator->bounded(101);
    flightCondition.inClouds = randomGenerator->bounded(2) < 1 ? false : true;

    scenario.setFlightCondition(flightCondition);
}

void SkyConnectDummy::recordAircraftInfo() noexcept
{
    QRandomGenerator *randomGenerator = QRandomGenerator::global();
    Aircraft &aircraft = World::getInstance().getCurrentScenario().getUserAircraft();
    AircraftInfo info;

    switch (randomGenerator->bounded(5)) {
    case 0:
        info.name = "Boeing 787";
        break;
    case 1:
        info.name = "Cirrus SR22";
        break;
    case 2:
        info.name = "Douglas DC-3";
        break;
    case 3:
        info.name = "Cessna 172";
        break;
    case 4:
        info.name = "Airbus A320";
        break;
    default:
        info.name = "Unknown";
    }

    info.atcId = QString::number(randomGenerator->bounded(1000)).toAscii();
    info.atcAirline = QString::number(randomGenerator->bounded(1000)).toAscii();
    info.atcFlightNumber = QString::number(randomGenerator->bounded(100)).toAscii();
    switch (randomGenerator->bounded(5)) {
    case 0:
        info.category = "Piston";
        break;
    case 1:
        info.category = "Glider";
        break;
    case 2:
        info.category = "Rocket";
        break;
    case 3:
        info.category = "Jet";
        break;
    case 4:
        info.category = "Turbo";
        break;
    default:
        info.category = "Unknown";
    }
    info.startOnGround = randomGenerator->bounded(2) > 0 ? true : false;
    info.aircraftAltitudeAboveGround = randomGenerator->bounded(40000);
    info.initialAirspeed = randomGenerator->bounded(600);
    info.wingSpan = randomGenerator->bounded(200);
    info.numberOfEngines = randomGenerator->bounded(5);
    info.engineType = static_cast<SimType::EngineType>(randomGenerator->bounded(7));

    aircraft.setAircraftInfo(info);
}

void SkyConnectDummy::replay() noexcept
{
    if (!sendAircraftData(TimeVariableData::Access::Linear)) {
        stopReplay();
    }
}
