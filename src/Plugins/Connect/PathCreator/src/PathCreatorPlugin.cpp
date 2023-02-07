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
#include <memory>
#include <cstdint>
#include <cmath>

#include <QTimer>
#include <QtGlobal>
#include <QRandomGenerator>
#include <QStringList>
#ifdef DEBUG
#include <QDebug>
#include <Kernel/Enum.h>
#endif

#include <Kernel/Settings.h>
#include <Kernel/SkyMath.h>
#include <Model/TimeVariableData.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/AircraftType.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Location.h>
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
    // Implementation note: std:round will become constexpr with C++23
    const int ReplayPeriod = static_cast<int>(std::round(1000.0 / ReplayRate));
}

struct PathCreatorPluginPrivate
{
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
}

PathCreatorPlugin::~PathCreatorPlugin() = default;

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

bool PathCreatorPlugin::onFreezeUserAircraft([[maybe_unused]] bool enable) const noexcept
{
    return true;
}

bool PathCreatorPlugin::onSimulationEvent([[maybe_unused]] SimulationEvent event, [[maybe_unused]] float arg1) const noexcept
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
    FlightCondition flightCondition = flight.getFlightCondition();
    flightCondition.endLocalTime = QDateTime::currentDateTime();
    flightCondition.endZuluTime = QDateTime::currentDateTimeUtc();
    flight.setFlightCondition(flightCondition);

    Aircraft &aircraft = flight.getUserAircraft();
    FlightPlan &flightPlan = aircraft.getFlightPlan();
    int waypointCount = static_cast<int>(flightPlan.count());
    if (waypointCount > 1) {
        Waypoint waypoint = flightPlan[waypointCount - 1];
        waypoint.localTime = QDateTime::currentDateTime();
        waypoint.zuluTime = QDateTime::currentDateTimeUtc();
        flight.updateWaypoint(waypointCount - 1, waypoint);
    }
}

bool PathCreatorPlugin::onStartReplay([[maybe_unused]] std::int64_t currentTimestamp) noexcept {
    d->replayTimer.start(ReplayPeriod);
    return true;
}

void PathCreatorPlugin::onReplayPaused(bool enable) noexcept
{
    if (enable) {
         d->replayTimer.stop();
    } else {
        d->replayTimer.start(ReplayPeriod);
    }
#ifdef DEBUG
    qDebug() << "PathCreatorPlugin::onReplayPaused: enable:" << enable;
#endif
}

void PathCreatorPlugin::onStopReplay() noexcept
{
    d->replayTimer.stop();
}

void PathCreatorPlugin::onSeek([[maybe_unused]] std::int64_t currentTimestamp, [[maybe_unused]] SeekMode seekMode) noexcept
{}

void PathCreatorPlugin::onRecordingSampleRateChanged([[maybe_unused]] SampleRate::SampleRate sampleRate) noexcept
{}

bool PathCreatorPlugin::sendAircraftData(std::int64_t currentTimestamp, TimeVariableData::Access access, [[maybe_unused]] AircraftSelection aircraftSelection) noexcept
{
    bool dataAvailable {false};
    if (currentTimestamp <= getCurrentFlight().getTotalDurationMSec()) {
        dataAvailable = true;
        const PositionData &currentPositionData = getCurrentFlight().getUserAircraft().getPosition().interpolate(getCurrentTimestamp(), access);
        if (!currentPositionData.isNull()) {
            // Start the elapsed timer after sending the first sample data
            if (!isElapsedTimerRunning()) {
                startElapsedTimer();
            }
        }
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

void PathCreatorPlugin::onAddAiObject([[maybe_unused]] const Aircraft &aircraft) noexcept
{
#ifdef DEBUG
    qDebug() << "PathCreatorPlugin::onAddAiObject: CALLED";
#endif
}

void PathCreatorPlugin::onRemoveAiObject(std::int64_t aircraftId) noexcept
{
#ifdef DEBUG
    qDebug() << "PathCreatorPlugin::onRemoveAiObject: aircraft ID:" << aircraftId;
#endif
}

void PathCreatorPlugin::onRemoveAllAiObjects() noexcept
{
#ifdef DEBUG
    qDebug() << "PathCreatorPlugin::onRemoveAllAiObjects: CALLED.";
#endif
}

bool PathCreatorPlugin::onRequestLocation() noexcept
{
    Location location {
        -90.0 + d->randomGenerator->bounded(180),
        -180.0 + d->randomGenerator->bounded(360.0),
        d->randomGenerator->bounded(60000.0)
    };
    location.pitch = -90.0 + d->randomGenerator->bounded(180.0);
    location.bank = -180.0 + d->randomGenerator->bounded(360.0);
    location.trueHeading = -180.0 + d->randomGenerator->bounded(360.0);
    location.indicatedAirspeed = d->randomGenerator->bounded(400);
    location.onGround = false;
    emit locationReceived(location);

    return true;
}

// PROTECTED SLOTS

void PathCreatorPlugin::recordData() noexcept
{
    if (!isElapsedTimerRunning()) {
        // Start the elapsed timer with the arrival of the first sample data
        setCurrentTimestamp(0);
        resetElapsedTime(true);
    }

    const std::int64_t timestamp = updateCurrentTimestamp();
    recordPositionData(timestamp);
    recordEngineData(timestamp);
    recordPrimaryControls(timestamp);
    recordSecondaryControls(timestamp);
    recordAircraftHandle(timestamp);
    recordLights(timestamp);
    recordWaypoint(timestamp);
}

// PRIVATE

void PathCreatorPlugin::frenchConnection() noexcept
{
    connect(&d->replayTimer, &QTimer::timeout,
            this, &PathCreatorPlugin::replay);
}

void PathCreatorPlugin::recordPositionData(std::int64_t timestamp) noexcept
{
    PositionData aircraftData;
    aircraftData.latitude = -90.0 + d->randomGenerator->bounded(180);
    aircraftData.longitude = -180.0 + d->randomGenerator->bounded(360.0);
    aircraftData.altitude = d->randomGenerator->bounded(60000.0);
    aircraftData.indicatedAltitude = d->randomGenerator->bounded(20000.0);
    aircraftData.pitch = -90.0 + d->randomGenerator->bounded(180.0);
    aircraftData.bank = -180.0 + d->randomGenerator->bounded(360.0);
    aircraftData.trueHeading = -180.0 + d->randomGenerator->bounded(360.0);

    aircraftData.velocityBodyX = d->randomGenerator->bounded(1.0);
    aircraftData.velocityBodyY = d->randomGenerator->bounded(1.0);
    aircraftData.velocityBodyZ = d->randomGenerator->bounded(1.0);

    aircraftData.timestamp = timestamp;
    Aircraft &aircraft = getCurrentFlight().getUserAircraft();
    aircraft.getPosition().upsertLast(aircraftData);
}

void PathCreatorPlugin::recordEngineData(std::int64_t timestamp) noexcept
{
    EngineData engineData;
    engineData.throttleLeverPosition1 = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));
    engineData.throttleLeverPosition2 = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));
    engineData.throttleLeverPosition3 = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));
    engineData.throttleLeverPosition4 = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));
    engineData.propellerLeverPosition1 = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    engineData.propellerLeverPosition2 = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    engineData.propellerLeverPosition3 = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    engineData.propellerLeverPosition4 = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
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
    Aircraft &aircraft = getCurrentFlight().getUserAircraft();
    aircraft.getEngine().upsertLast(engineData);
}

void PathCreatorPlugin::recordPrimaryControls(std::int64_t timestamp) noexcept
{
    PrimaryFlightControlData primaryFlightControlData;
    primaryFlightControlData.rudderDeflection = Convert::degreesToRadians(-45.0 + d->randomGenerator->bounded(90.0));
    primaryFlightControlData.elevatorDeflection = Convert::degreesToRadians(-45.0 + d->randomGenerator->bounded(90.0));
    primaryFlightControlData.leftAileronDeflection = Convert::degreesToRadians(-45.0 + d->randomGenerator->bounded(90.0));
    primaryFlightControlData.rightAileronDeflection = Convert::degreesToRadians(-45.0 + d->randomGenerator->bounded(90.0));
    primaryFlightControlData.rudderPosition = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.elevatorPosition = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.aileronPosition = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));

    primaryFlightControlData.timestamp = timestamp;
    Aircraft &aircraft = getCurrentFlight().getUserAircraft();
    aircraft.getPrimaryFlightControl().upsertLast(primaryFlightControlData);
}

void PathCreatorPlugin::recordSecondaryControls(std::int64_t timestamp) noexcept
{
    SecondaryFlightControlData secondaryFlightControlData;
    secondaryFlightControlData.leftLeadingEdgeFlapsPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.rightLeadingEdgeFlapsPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.leftTrailingEdgeFlapsPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.rightTrailingEdgeFlapsPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.leftSpoilersPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.rightSpoilersPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    secondaryFlightControlData.spoilersHandlePercent = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    secondaryFlightControlData.flapsHandleIndex = static_cast<std::int8_t>(d->randomGenerator->bounded(5));

    secondaryFlightControlData.timestamp = timestamp;
    Aircraft &aircraft = getCurrentFlight().getUserAircraft();
    aircraft.getSecondaryFlightControl().upsertLast(secondaryFlightControlData);
}

void PathCreatorPlugin::recordAircraftHandle(std::int64_t timestamp) noexcept
{
    AircraftHandleData aircraftHandleData;
    aircraftHandleData.brakeLeftPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    aircraftHandleData.brakeRightPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    aircraftHandleData.waterRudderHandlePosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    aircraftHandleData.tailhookPosition = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.canopyOpen = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.leftWingFolding = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.rightWingFolding = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.gearHandlePosition = d->randomGenerator->bounded(2) < 1 ? false : true;
    aircraftHandleData.tailhookHandlePosition = d->randomGenerator->bounded(2) < 1 ? false : true;
    aircraftHandleData.foldingWingHandlePosition = d->randomGenerator->bounded(2) < 1 ? false : true;
    aircraftHandleData.smokeEnabled = d->randomGenerator->bounded(2) < 1 ? false : true;

    aircraftHandleData.timestamp = timestamp;
    Aircraft &aircraft = getCurrentFlight().getUserAircraft();
    aircraft.getAircraftHandle().upsertLast(aircraftHandleData);
}

void PathCreatorPlugin::recordLights(std::int64_t timestamp) noexcept
{
    static int lights = 0;
    LightData lightData;
    lightData.lightStates = static_cast<SimType::LightStates>(lights);
    lights = ++lights % 0b1111111111;

    lightData.timestamp = timestamp;
    Aircraft &aircraft = getCurrentFlight().getUserAircraft();
    aircraft.getLight().upsertLast(lightData);
}

void PathCreatorPlugin::recordWaypoint(std::int64_t timestamp) noexcept
{
    Waypoint waypoint;
    if (d->randomGenerator->bounded(100.0) < 0.5) {
        auto i = d->randomGenerator->bounded(PathCreatorPluginPrivate::IcaoList.size());
        waypoint.identifier = PathCreatorPluginPrivate::IcaoList.at(i);
        waypoint.latitude = -90.0f + static_cast<float>(d->randomGenerator->bounded(180.0));
        waypoint.longitude = -180.0f + static_cast<float>(d->randomGenerator->bounded(90.0));
        waypoint.altitude = static_cast<float>(d->randomGenerator->bounded(3000.0));
        waypoint.localTime = QDateTime::currentDateTime();
        waypoint.zuluTime = QDateTime::currentDateTimeUtc();
        waypoint.timestamp = timestamp;

        Flight &flight = getCurrentFlight();
        flight.addWaypoint(waypoint);
    }
}

void PathCreatorPlugin::recordFlightCondition() noexcept
{
    FlightCondition flightCondition;

    flightCondition.groundAltitude = static_cast<float>(d->randomGenerator->bounded(4000.0));
    flightCondition.surfaceType = static_cast<SimType::SurfaceType>(d->randomGenerator->bounded(26));
    flightCondition.surfaceCondition = static_cast<SimType::SurfaceCondition>(d->randomGenerator->bounded(5));
    flightCondition.ambientTemperature = static_cast<float>(d->randomGenerator->bounded(80.0f)) - 40.0f;
    flightCondition.totalAirTemperature = static_cast<float>(d->randomGenerator->bounded(80.0f)) - 40.0f;
    flightCondition.windSpeed = static_cast<float>(d->randomGenerator->bounded(30.0));
    flightCondition.windDirection = static_cast<float>(d->randomGenerator->bounded(360.0));
    flightCondition.precipitationState = static_cast<SimType::PrecipitationState>(d->randomGenerator->bounded(4));
    flightCondition.visibility = static_cast<float>(d->randomGenerator->bounded(10000.0));
    flightCondition.seaLevelPressure = 950.0f + static_cast<float>(d->randomGenerator->bounded(100.0));
    flightCondition.pitotIcingPercent = d->randomGenerator->bounded(101);
    flightCondition.structuralIcingPercent = d->randomGenerator->bounded(101);
    flightCondition.inClouds = d->randomGenerator->bounded(2) < 1 ? false : true;
    flightCondition.onAnyRunway = d->randomGenerator->bounded(2) < 1 ? false : true;
    flightCondition.onParkingSpot = d->randomGenerator->bounded(2) < 1 ? false : true;
    flightCondition.startLocalTime = QDateTime::currentDateTime();
    flightCondition.startZuluTime = QDateTime::currentDateTimeUtc();

    getCurrentFlight().setFlightCondition(flightCondition);
}

void PathCreatorPlugin::recordAircraftInfo() noexcept
{
    Flight &flight = getCurrentFlight();
    Aircraft &aircraft = flight.getUserAircraft();
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
    info.altitudeAboveGround = static_cast<float>(d->randomGenerator->bounded(40000.0));
    info.startOnGround = d->randomGenerator->bounded(2) > 0 ? true : false;
    info.initialAirspeed = d->randomGenerator->bounded(600);

    aircraft.setAircraftInfo(info);
    emit flight.aircraftInfoChanged(aircraft);
}

void PathCreatorPlugin::replay() noexcept
{
    const std::int64_t timestamp = updateCurrentTimestamp();
    if (!sendAircraftData(timestamp, TimeVariableData::Access::Linear, AircraftSelection::All)) {
        handleAtEnd();
    }
}
