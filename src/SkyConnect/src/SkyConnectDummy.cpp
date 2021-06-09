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
#include <QStringList>

#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/SkyMath.h"
#include "../../Model/src/TimeVariableData.h"
#include "../../Model/src/Logbook.h"
#include "../../Model/src/Flight.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/Position.h"
#include "../../Model/src/PositionData.h"
#include "../../Model/src/Engine.h"
#include "../../Model/src/EngineData.h"
#include "../../Model/src/PrimaryFlightControl.h"
#include "../../Model/src/PrimaryFlightControlData.h"
#include "../../Model/src/SecondaryFlightControl.h"
#include "../../Model/src/SecondaryFlightControlData.h"
#include "../../Model/src/AircraftHandle.h"
#include "../../Model/src/AircraftHandleData.h"
#include "../../Model/src/Light.h"
#include "../../Model/src/LightData.h"
#include "../../Model/src/FlightPlan.h"
#include "../../Model/src/Waypoint.h"
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
        : randomGenerator(QRandomGenerator::global())
    {}

    QTimer replayTimer;
    QRandomGenerator *randomGenerator;

    static const QStringList IcaoList;
};

const QStringList SkyConnectDummyPrivate::IcaoList {"LSZH", "LSGG", "LSME", "LSZW", "LSTZ", "LSZB", "LSMA", "LSZJ", "LSPD", "LSHG", "LSZG", "LSZN", "LSGL", "LSEY", "LSPF"};

// PUBLIC

SkyConnectDummy::SkyConnectDummy(QObject *parent) noexcept
    : AbstractSkyConnect(parent),
      d(std::make_unique<SkyConnectDummyPrivate>())
{
    frenchConnection();
}

SkyConnectDummy::~SkyConnectDummy() noexcept
{
#ifdef DEBUG
    qDebug("SkyConnectDummy::~SkyConnectDummy: DELETED");
#endif
}

// PROTECTED

bool SkyConnectDummy::onStartRecording() noexcept
{
    recordFlightCondition();
    recordAircraftInfo();
    return true;
}

void SkyConnectDummy::onRecordingPaused(bool paused) noexcept
{
    Q_UNUSED(paused)
}

void SkyConnectDummy::onStopRecording() noexcept
{
    Flight &flight = getCurrentFlight();
    FlightCondition flightCondition = flight.getFlightConditionConst();
    flightCondition.endLocalTime = QDateTime::currentDateTime();
    flightCondition.endZuluTime = QDateTime::currentDateTimeUtc();
    flight.setFlightCondition(flightCondition);

    FlightPlan &flightPlan = flight.getUserAircraftConst().getFlightPlan();
    int waypointCount = flightPlan.getAllConst().count();
    if (waypointCount > 1) {
        Waypoint waypoint = flightPlan.getAllConst().at(waypointCount - 1);
        waypoint.localTime = QDateTime::currentDateTime();
        waypoint.zuluTime = QDateTime::currentDateTimeUtc();
        flightPlan.update(waypointCount - 1, waypoint);
    }
}

bool SkyConnectDummy::onStartReplay(qint64 currentTimestamp) noexcept {
    Q_UNUSED(currentTimestamp)
    d->replayTimer.start(ReplayPeriod);
    return true;
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

    const Flight &flight = getCurrentFlight();
    for (const auto &it : flight) {
        // Send data
    }
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

bool SkyConnectDummy::onCreateAIObjects() noexcept
{
#ifdef DEBUG
    qDebug("SkyConnectDummy::onCreateAIObjects: CALLED");
#endif
    return true;
}

void SkyConnectDummy::onDestroyAIObjects() noexcept
{
#ifdef DEBUG
    qDebug("SkyConnectDummy::onDestroyAIObjects: CALLED");
#endif
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
    bool dataAvailable;
    const qint64 currentTimestamp = getCurrentTimestamp();
    if (currentTimestamp <= getCurrentFlight().getTotalDurationMSec()) {
        dataAvailable = true;
        const PositionData &currentPositionData = getCurrentFlight().getUserAircraftConst().getPosition().interpolate(getCurrentTimestamp(), access);
        if (!currentPositionData.isNull()) {
            // Start the elapsed timer after sending the first sample data
            if (!isElapsedTimerRunning()) {
                startElapsedTimer();
            }
        }
    } else {
        // At end of recording
        dataAvailable = false;
    }
    return dataAvailable;
}

void SkyConnectDummy::recordData() noexcept
{
    const qint64 timestamp = getCurrentTimestamp();

    recordPositionData(timestamp);
    recordEngineData(timestamp);
    recordPrimaryControls(timestamp);
    recordSecondaryControls(timestamp);
    recordAircraftHandle(timestamp);
    recordLights(timestamp);
    recordWaypoint();

    if (!isElapsedTimerRunning()) {
        // Start the elapsed timer with the arrival of the first sample data
        setCurrentTimestamp(0);
        resetElapsedTime(true);
    }
}

void SkyConnectDummy::recordPositionData(qint64 timestamp) noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    PositionData aircraftData;
    aircraftData.latitude = -180.0 + d->randomGenerator->bounded(360.0);
    aircraftData.longitude = -90.0 + d->randomGenerator->bounded(180.0);
    aircraftData.altitude = d->randomGenerator->bounded(20000.0);
    aircraftData.pitch = -90.0 + d->randomGenerator->bounded(180.0);
    aircraftData.bank = -180.0 + d->randomGenerator->bounded(360.0);
    aircraftData.heading = -180.0 + d->randomGenerator->bounded(360.0);

    aircraftData.rotationVelocityBodyX = d->randomGenerator->bounded(1.0);
    aircraftData.rotationVelocityBodyY = d->randomGenerator->bounded(1.0);
    aircraftData.rotationVelocityBodyZ = d->randomGenerator->bounded(1.0);
    aircraftData.velocityBodyX = d->randomGenerator->bounded(1.0);
    aircraftData.velocityBodyY = d->randomGenerator->bounded(1.0);
    aircraftData.velocityBodyZ = d->randomGenerator->bounded(1.0);

    aircraftData.timestamp = timestamp;
    aircraft.getPosition().upsert(aircraftData);
}

void SkyConnectDummy::recordEngineData(qint64 timestamp) noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    EngineData engineData;
    engineData.throttleLeverPosition1 = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    engineData.throttleLeverPosition2 = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    engineData.throttleLeverPosition3 = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    engineData.throttleLeverPosition4 = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    engineData.propellerLeverPosition1 = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    engineData.propellerLeverPosition2 = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    engineData.propellerLeverPosition3 = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    engineData.propellerLeverPosition4 = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    engineData.mixtureLeverPosition1 = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    engineData.mixtureLeverPosition2 = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    engineData.mixtureLeverPosition3 = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    engineData.mixtureLeverPosition4 = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    engineData.cowlFlapPosition1 = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    engineData.cowlFlapPosition2 = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    engineData.cowlFlapPosition3 = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    engineData.cowlFlapPosition4 = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    engineData.electricalMasterBattery1 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.electricalMasterBattery2 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.electricalMasterBattery3 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.electricalMasterBattery4 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.generalEngineStarter1 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.generalEngineStarter2 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.generalEngineStarter3 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.generalEngineStarter4 = d->randomGenerator->bounded(2) < 1 ? false : true;

    engineData.timestamp = timestamp;
    aircraft.getEngine().upsert(std::move(engineData));
}

void SkyConnectDummy::recordPrimaryControls(qint64 timestamp) noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    PrimaryFlightControlData primaryFlightControlData;
    primaryFlightControlData.rudderPosition = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.elevatorPosition = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.aileronPosition = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));

    primaryFlightControlData.timestamp = timestamp;
    aircraft.getPrimaryFlightControl().upsert(std::move(primaryFlightControlData));
}

void SkyConnectDummy::recordSecondaryControls(qint64 timestamp) noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    SecondaryFlightControlData secondaryFlightControlData;
    secondaryFlightControlData.leadingEdgeFlapsLeftPercent = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    secondaryFlightControlData.leadingEdgeFlapsRightPercent = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    secondaryFlightControlData.trailingEdgeFlapsLeftPercent = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    secondaryFlightControlData.trailingEdgeFlapsRightPercent = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    secondaryFlightControlData.flapsHandleIndex = d->randomGenerator->bounded(5);

    secondaryFlightControlData.timestamp = timestamp;
    aircraft.getSecondaryFlightControl().upsert(std::move(secondaryFlightControlData));
}

void SkyConnectDummy::recordAircraftHandle(qint64 timestamp) noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    AircraftHandleData aircraftHandleData;
    aircraftHandleData.brakeLeftPosition = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    aircraftHandleData.brakeRightPosition = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    aircraftHandleData.waterRudderHandlePosition = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    aircraftHandleData.tailhookPosition = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.canopyOpen = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.leftWingFolding = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.rightWingFolding = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.gearHandlePosition = d->randomGenerator->bounded(2) < 1 ? false : true;

    aircraftHandleData.timestamp = timestamp;
    aircraft.getAircraftHandle().upsert(std::move(aircraftHandleData));
}

void SkyConnectDummy::recordLights(qint64 timestamp) noexcept
{
    static int lights = 0;
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    LightData lightData;
    lightData.lightStates = static_cast<SimType::LightStates>(lights);
    lights = ++lights % 0b1111111111;

    lightData.timestamp = timestamp;
    aircraft.getLight().upsert(std::move(lightData));
}

void SkyConnectDummy::recordWaypoint() noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    Waypoint waypoint;
    if (d->randomGenerator->bounded(100.0) < 0.5) {
        int i = d->randomGenerator->bounded(SkyConnectDummyPrivate::IcaoList.size());
        waypoint.identifier = SkyConnectDummyPrivate::IcaoList.at(i);
        waypoint.latitude = -180.0 + d->randomGenerator->bounded(360.0);
        waypoint.longitude = -90.0 + d->randomGenerator->bounded(180.0);
        waypoint.altitude = d->randomGenerator->bounded(3000.0);
        waypoint.localTime = QDateTime::currentDateTime();
        waypoint.zuluTime = QDateTime::currentDateTimeUtc();
        waypoint.timestamp = getCurrentTimestamp();

        aircraft.getFlightPlan().add(waypoint);
    }
}

void SkyConnectDummy::recordFlightCondition() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    FlightCondition flightCondition;

    flightCondition.groundAltitude = d->randomGenerator->bounded(4000);
    flightCondition.surfaceType = static_cast<SimType::SurfaceType>(d->randomGenerator->bounded(26));
    flightCondition.ambientTemperature = d->randomGenerator->bounded(80.0) - 40.0;
    flightCondition.totalAirTemperature = d->randomGenerator->bounded(80.0) - 40.0;
    flightCondition.windVelocity = d->randomGenerator->bounded(30.0);
    flightCondition.windDirection = d->randomGenerator->bounded(360);
    flightCondition.precipitationState = static_cast<SimType::PrecipitationState>(d->randomGenerator->bounded(4));
    flightCondition.visibility = d->randomGenerator->bounded(10000.0);
    flightCondition.seaLevelPressure = 950.0 + d->randomGenerator->bounded(100.0);
    flightCondition.pitotIcingPercent = d->randomGenerator->bounded(101);
    flightCondition.structuralIcingPercent = d->randomGenerator->bounded(101);
    flightCondition.inClouds = d->randomGenerator->bounded(2) < 1 ? false : true;
    flightCondition.startLocalTime = QDateTime::currentDateTime();
    flightCondition.startZuluTime = QDateTime::currentDateTimeUtc();

    flight.setFlightCondition(flightCondition);
}

void SkyConnectDummy::recordAircraftInfo() noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    AircraftInfo info(aircraft.getId());

    switch (d->randomGenerator->bounded(5)) {
    case 0:
        info.type = "Boeing 787";
        break;
    case 1:
        info.type = "Cirrus SR22";
        break;
    case 2:
        info.type = "Douglas DC-3";
        break;
    case 3:
        info.type = "Cessna 172";
        break;
    case 4:
        info.type = "Airbus A320";
        break;
    default:
        info.type = "Unknown";
    }

    if (info.startDate.isNull()) {
        info.startDate = QDateTime::currentDateTime();
    }
    info.tailNumber = QString::number(d->randomGenerator->bounded(1000));
    info.airline = QString::number(d->randomGenerator->bounded(1000));
    info.flightNumber = QString::number(d->randomGenerator->bounded(100));
    switch (d->randomGenerator->bounded(5)) {
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
    info.altitudeAboveGround = d->randomGenerator->bounded(40000);
    info.startOnGround = d->randomGenerator->bounded(2) > 0 ? true : false;
    info.initialAirspeed = d->randomGenerator->bounded(600);
    info.wingSpan = d->randomGenerator->bounded(200);
    info.engineType = static_cast<SimType::EngineType>(d->randomGenerator->bounded(7));
    info.numberOfEngines = d->randomGenerator->bounded(5);

    aircraft.setAircraftInfo(info);
}

void SkyConnectDummy::replay() noexcept
{
    if (!sendAircraftData(TimeVariableData::Access::Linear)) {
        stopReplay();
    }
}
