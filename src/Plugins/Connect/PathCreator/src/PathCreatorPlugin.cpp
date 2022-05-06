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
#include <cstdint>
#include <cinttypes>

#include <QTimer>
#include <QtGlobal>
#include <QRandomGenerator>
#include <QStringList>

#include <Kernel/Settings.h>
#include <Kernel/SkyMath.h>
#include <Model/TimeVariableData.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/AircraftType.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/InitialPosition.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/PrimaryFlightControl.h>
#include <Model/PrimaryFlightControlData.h>
#include <Model/SecondaryFlightControl.h>
#include <Model/SecondaryFlightControlData.h>
#include <Model/AircraftHandle.h>
#include <Model/AircraftHandleData.h>
#include <Model/Light.h>
#include <Model/LightData.h>
#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include <Model/FlightCondition.h>
#include <Model/SimType.h>
#include <PluginManager/AbstractSkyConnect.h>
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
    {
        replayTimer.setTimerType(Qt::TimerType::PreciseTimer);
    }

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

bool PathCreatorPlugin::setUserAircraftPosition([[maybe_unused]] const PositionData &positionData) noexcept
{
    return true;
}

// PROTECTED

bool PathCreatorPlugin::isTimerBasedRecording([[maybe_unused]] SampleRate::SampleRate sampleRate) const noexcept
{
    return true;
}

bool PathCreatorPlugin::onInitialPositionSetup([[maybe_unused]] const InitialPosition &initialPosition) noexcept
{
    return true;
}

bool PathCreatorPlugin::onFreezeUserAircraft([[maybe_unused]] bool enable) noexcept
{
    return true;
}

bool PathCreatorPlugin::onStartRecording() noexcept
{
    recordFlightCondition();
    recordAircraftInfo();
    return true;
}

void PathCreatorPlugin::onRecordingPaused([[maybe_unused]] bool paused) noexcept
{}

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

bool PathCreatorPlugin::onStartReplay([[maybe_unused]] std::int64_t currentTimestamp) noexcept {
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

void PathCreatorPlugin::onSeek([[maybe_unused]] std::int64_t currentTimestamp) noexcept
{}

void PathCreatorPlugin::onRecordingSampleRateChanged([[maybe_unused]] SampleRate::SampleRate sampleRate) noexcept
{}

bool PathCreatorPlugin::sendAircraftData(std::int64_t currentTimestamp, TimeVariableData::Access access, [[maybe_unused]] AircraftSelection aircraftSelection) noexcept
{
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

void PathCreatorPlugin::onCreateAiObjects(const Flight &flight) noexcept
{
#ifdef DEBUG
    qDebug("PathCreatorPlugin::onCreateAiObjects: CALLED");
#endif
}

void PathCreatorPlugin::onDestroyAiObjects() noexcept
{
#ifdef DEBUG
    qDebug("PathCreatorPlugin::onDestroyAiObjects: CALLED");
#endif
}

void PathCreatorPlugin::onAddAiObject(const Aircraft &aircraft) noexcept
{
#ifdef DEBUG
    qDebug("PathCreatorPlugin::onAddAiObject: CALLED");
#endif
}

void PathCreatorPlugin::onDestroyAiObject(std::int64_t aircraftId) noexcept
{
#ifdef DEBUG
    qDebug("PathCreatorPlugin::onDestroyAiObject: simulated object ID: %" PRId64, aircraftId);
#endif
}

// PROTECTED SLOTS

void PathCreatorPlugin::recordData() noexcept
{
    const std::int64_t timestamp = updateCurrentTimestamp();

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

void PathCreatorPlugin::recordPositionData(std::int64_t timestamp) noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    PositionData aircraftData;
    aircraftData.latitude = -90.0 + d->randomGenerator->bounded(180);
    aircraftData.longitude = -180.0 + d->randomGenerator->bounded(360.0);
    aircraftData.altitude = d->randomGenerator->bounded(20000.0);
    aircraftData.indicatedAltitude = d->randomGenerator->bounded(20000.0);
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

void PathCreatorPlugin::recordEngineData(std::int64_t timestamp) noexcept
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

void PathCreatorPlugin::recordPrimaryControls(std::int64_t timestamp) noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    PrimaryFlightControlData primaryFlightControlData;
    primaryFlightControlData.rudderPosition = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.elevatorPosition = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.aileronPosition = SkyMath::fromPosition(-1.0 + d->randomGenerator->bounded(2.0));

    primaryFlightControlData.timestamp = timestamp;
    aircraft.getPrimaryFlightControl().upsertLast(std::move(primaryFlightControlData));
}

void PathCreatorPlugin::recordSecondaryControls(std::int64_t timestamp) noexcept
{
    Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();

    SecondaryFlightControlData secondaryFlightControlData;
    secondaryFlightControlData.leadingEdgeFlapsLeftPosition = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.leadingEdgeFlapsRightPosition = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.trailingEdgeFlapsLeftPosition = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.trailingEdgeFlapsRightPosition = SkyMath::fromPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.spoilersHandlePosition = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    secondaryFlightControlData.flapsHandleIndex = d->randomGenerator->bounded(5);

    secondaryFlightControlData.timestamp = timestamp;
    aircraft.getSecondaryFlightControl().upsertLast(std::move(secondaryFlightControlData));
}

void PathCreatorPlugin::recordAircraftHandle(std::int64_t timestamp) noexcept
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

void PathCreatorPlugin::recordLights(std::int64_t timestamp) noexcept
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
        waypoint.latitude = -90.0f + static_cast<float>(d->randomGenerator->bounded(180.0));
        waypoint.longitude = -180.0f + static_cast<float>(d->randomGenerator->bounded(90.0));
        waypoint.altitude = static_cast<float>(d->randomGenerator->bounded(3000.0));
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
    const std::int64_t timestamp = updateCurrentTimestamp();
    if (!sendAircraftData(timestamp, TimeVariableData::Access::Linear, AircraftSelection::All)) {
        handleAtEnd();
    }
}
