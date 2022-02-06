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
#include <memory>

#include <QTimer>
#include <QtGlobal>
#include <QRandomGenerator>
#include <QStringList>

#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Model/src/TimeVariableData.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/AircraftInfo.h"
#include "../../../../../Model/src/AircraftType.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/InitialPosition.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../../Model/src/AircraftHandle.h"
#include "../../../../../Model/src/AircraftHandleData.h"
#include "../../../../../Model/src/Light.h"
#include "../../../../../Model/src/LightData.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/SimType.h"
#include "../../../AbstractSkyConnect.h"
#include "PathCreatorPlugin.h"

namespace {
    // Hz
    constexpr int ReplayRate = 60;
    constexpr int ReplayPeriod = qRound(1000.0 / ReplayRate);
}

class PathCreatorPluginPrivate
{
public:
    PathCreatorPluginPrivate() noexcept
        : randomGenerator(QRandomGenerator::global())
    {}

    QTimer replayTimer;
    QRandomGenerator *randomGenerator;

    static const QStringList IcaoList;
};

const QStringList PathCreatorPluginPrivate::IcaoList {"LSZH", "LSGG", "LSME", "LSZW", "LSTZ", "LSZB", "LSMA", "LSZJ", "LSPD", "LSHG", "LSZG", "LSZN", "LSGL", "LSEY", "LSPF"};

// PUBLIC

PathCreatorPlugin::PathCreatorPlugin(QObject *parent) noexcept
    : AbstractSkyConnect(parent),
      d(std::make_unique<PathCreatorPluginPrivate>())
{
    frenchConnection();
#ifdef DEBUG
    qDebug("PathCreatorPlugin::PathCreatorPlugin: CREATED");
#endif
}

PathCreatorPlugin::~PathCreatorPlugin() noexcept
{
#ifdef DEBUG
    qDebug("PathCreatorPlugin::~PathCreatorPlugin: DELETED");
#endif
}

bool PathCreatorPlugin::setUserAircraftPosition(const PositionData &positionData) noexcept
{
    Q_UNUSED(positionData)
    return true;
}

// PROTECTED

bool PathCreatorPlugin::isTimerBasedRecording(SampleRate::SampleRate sampleRate) const noexcept
{
    Q_UNUSED(sampleRate)
    return true;
}

bool PathCreatorPlugin::onInitialPositionSetup(const InitialPosition &initialPosition) noexcept
{
    Q_UNUSED(initialPosition)
    return true;
}

bool PathCreatorPlugin::onFreezeUserAircraft(bool enable) noexcept
{
    Q_UNUSED(enable)
    return true;
}

bool PathCreatorPlugin::onStartRecording() noexcept
{
    recordFlightCondition();
    recordAircraftInfo();
    return true;
}

void PathCreatorPlugin::onRecordingPaused(bool paused) noexcept
{
    Q_UNUSED(paused)
}

void PathCreatorPlugin::onStopRecording() noexcept
{
    Flight &flight = getCurrentFlight();
    FlightCondition flightCondition = flight.getFlightConditionConst();
    flightCondition.endLocalTime = QDateTime::currentDateTime();
    flightCondition.endZuluTime = QDateTime::currentDateTimeUtc();
    flight.setFlightCondition(flightCondition);

    FlightPlan &flightPlan = flight.getUserAircraftConst().getFlightPlan();
    int waypointCount = flightPlan.count();
    if (waypointCount > 1) {
        Waypoint waypoint = flightPlan[waypointCount - 1];
        waypoint.localTime = QDateTime::currentDateTime();
        waypoint.zuluTime = QDateTime::currentDateTimeUtc();
        flightPlan.update(waypointCount - 1, waypoint);
    }
}

bool PathCreatorPlugin::onStartReplay(qint64 currentTimestamp) noexcept {
    Q_UNUSED(currentTimestamp)
    d->replayTimer.start(ReplayPeriod);
    return true;
}

void PathCreatorPlugin::onReplayPaused(bool paused) noexcept
{
    if (paused) {
         d->replayTimer.stop();
    } else {
        d->replayTimer.start(ReplayPeriod);
    }
}

void PathCreatorPlugin::onStopReplay() noexcept
{
    d->replayTimer.stop();
}

void PathCreatorPlugin::onSeek(qint64 currentTimestamp) noexcept
{
    Q_UNUSED(currentTimestamp)
};

void PathCreatorPlugin::onRecordingSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept
{
    Q_UNUSED(sampleRate)
}

bool PathCreatorPlugin::sendAircraftData(qint64 currentTimestamp, TimeVariableData::Access access, AircraftSelection aircraftSelection) noexcept
{
    Q_UNUSED(aircraftSelection)
    bool dataAvailable;
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

bool PathCreatorPlugin::isConnectedWithSim() const noexcept
{
    return true;
}

bool PathCreatorPlugin::connectWithSim() noexcept
{
    return true;
}

bool PathCreatorPlugin::onCreateAIObjects() noexcept
{
#ifdef DEBUG
    qDebug("PathCreatorPlugin::onCreateAIObjects: CALLED");
#endif
    return true;
}

void PathCreatorPlugin::onDestroyAIObjects() noexcept
{
#ifdef DEBUG
    qDebug("PathCreatorPlugin::onDestroyAIObjects: CALLED");
#endif
}

void PathCreatorPlugin::onDestroyAIObject(Aircraft &aircraft) noexcept
{
#ifdef DEBUG
    qDebug("PathCreatorPlugin::onDestroyAIObject: aircraft ID: %lld", aircraft.getId());
#endif
}

// PROTECTED SLOTS

void PathCreatorPlugin::recordData() noexcept
{
    const qint64 timestamp = updateCurrentTimestamp();

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

// PRIVATE

void PathCreatorPlugin::frenchConnection() noexcept
{
    connect(&d->replayTimer, &QTimer::timeout,
            this, &PathCreatorPlugin::replay);
}

void PathCreatorPlugin::recordPositionData(qint64 timestamp) noexcept
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
    aircraft.getPosition().upsertLast(aircraftData);
}

void PathCreatorPlugin::recordEngineData(qint64 timestamp) noexcept
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
    engineData.generalEngineCombustion1 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.generalEngineCombustion2 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.generalEngineCombustion3 = d->randomGenerator->bounded(2) < 1 ? false : true;
    engineData.generalEngineCombustion4 = d->randomGenerator->bounded(2) < 1 ? false : true;

    engineData.timestamp = timestamp;
    aircraft.getEngine().upsertLast(std::move(engineData));
}

void PathCreatorPlugin::recordPrimaryControls(qint64 timestamp) noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    PrimaryFlightControlData primaryFlightControlData;
    primaryFlightControlData.rudderPosition = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.elevatorPosition = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.aileronPosition = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));

    primaryFlightControlData.timestamp = timestamp;
    aircraft.getPrimaryFlightControl().upsertLast(std::move(primaryFlightControlData));
}

void PathCreatorPlugin::recordSecondaryControls(qint64 timestamp) noexcept
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
    aircraft.getSecondaryFlightControl().upsertLast(std::move(secondaryFlightControlData));
}

void PathCreatorPlugin::recordAircraftHandle(qint64 timestamp) noexcept
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
    aircraftHandleData.smokeEnabled = d->randomGenerator->bounded(2) < 1 ? false : true;

    aircraftHandleData.timestamp = timestamp;
    aircraft.getAircraftHandle().upsertLast(std::move(aircraftHandleData));
}

void PathCreatorPlugin::recordLights(qint64 timestamp) noexcept
{
    static int lights = 0;
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    LightData lightData;
    lightData.lightStates = static_cast<SimType::LightStates>(lights);
    lights = ++lights % 0b1111111111;

    lightData.timestamp = timestamp;
    aircraft.getLight().upsertLast(std::move(lightData));
}

void PathCreatorPlugin::recordWaypoint() noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    Waypoint waypoint;
    if (d->randomGenerator->bounded(100.0) < 0.5) {
        int i = d->randomGenerator->bounded(PathCreatorPluginPrivate::IcaoList.size());
        waypoint.identifier = PathCreatorPluginPrivate::IcaoList.at(i);
        waypoint.latitude = -180.0 + d->randomGenerator->bounded(360.0);
        waypoint.longitude = -90.0 + d->randomGenerator->bounded(180.0);
        waypoint.altitude = d->randomGenerator->bounded(3000.0);
        waypoint.localTime = QDateTime::currentDateTime();
        waypoint.zuluTime = QDateTime::currentDateTimeUtc();
        waypoint.timestamp = getCurrentTimestamp();

        aircraft.getFlightPlan().add(waypoint);
    }
}

void PathCreatorPlugin::recordFlightCondition() noexcept
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

void PathCreatorPlugin::recordAircraftInfo() noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    AircraftInfo info(aircraft.getId());

    switch (d->randomGenerator->bounded(5)) {
    case 0:
        info.aircraftType.type = "Boeing 787";
        break;
    case 1:
        info.aircraftType.type = "Cirrus SR22";
        break;
    case 2:
        info.aircraftType.type = "Douglas DC-3";
        break;
    case 3:
        info.aircraftType.type = "Cessna 172";
        break;
    case 4:
        info.aircraftType.type = "Airbus A320";
        break;
    default:
        info.aircraftType.type = "Unknown";
    }
    switch (d->randomGenerator->bounded(5)) {
    case 0:
        info.aircraftType.category = "Piston";
        break;
    case 1:
        info.aircraftType.category = "Glider";
        break;
    case 2:
        info.aircraftType.category = "Rocket";
        break;
    case 3:
        info.aircraftType.category = "Jet";
        break;
    case 4:
        info.aircraftType.category = "Turbo";
        break;
    default:
        info.aircraftType.category = "Unknown";
    }
    info.aircraftType.wingSpan = d->randomGenerator->bounded(200);
    info.aircraftType.engineType = static_cast<SimType::EngineType>(d->randomGenerator->bounded(7));
    info.aircraftType.numberOfEngines = d->randomGenerator->bounded(5);

    if (info.startDate.isNull()) {
        info.startDate = QDateTime::currentDateTime();
    }
    info.tailNumber = QString::number(d->randomGenerator->bounded(1000));
    info.airline = QString::number(d->randomGenerator->bounded(1000));
    info.flightNumber = QString::number(d->randomGenerator->bounded(100));
    info.altitudeAboveGround = d->randomGenerator->bounded(40000);
    info.startOnGround = d->randomGenerator->bounded(2) > 0 ? true : false;
    info.initialAirspeed = d->randomGenerator->bounded(600);

    aircraft.setAircraftInfo(std::move(info));
}

void PathCreatorPlugin::replay() noexcept
{
    const qint64 timestamp = updateCurrentTimestamp();
    if (!sendAircraftData(timestamp, TimeVariableData::Access::Linear, AircraftSelection::All)) {
        stopReplay();
    }
}
