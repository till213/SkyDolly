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
#include <optional>

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
#include <Kernel/Enum.h>
#include <Model/TimeVariableData.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/AircraftType.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Attitude.h>
#include <Model/AttitudeData.h>
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
#include <PluginManager/Connect/AbstractSkyConnect.h>
#include <PluginManager/Connect/FlightSimulatorShortcuts.h>
#include "PathCreatorSettings.h"
#include "PathCreatorOptionWidget.h"
#include "PathCreatorPlugin.h"

namespace {
    // Hz
    constexpr int RecordingRate = 15;
    constexpr int ReplayRate = 60;
    // Implementation note: std:round will become constexpr with C++23
    const int RecordingPeriod = static_cast<int>(std::round(1000.0 / RecordingRate));
    const int ReplayPeriod = static_cast<int>(std::round(1000.0 / ReplayRate));
}

struct PathCreatorPluginPrivate
{
    PathCreatorPluginPrivate() noexcept
        : randomGenerator {QRandomGenerator::global()}
    {
        recordingTimer.setTimerType(Qt::TimerType::PreciseTimer);
        replayTimer.setTimerType(Qt::TimerType::PreciseTimer);
    }

    PathCreatorSettings pluginSettings;

    QTimer recordingTimer;
    QTimer replayTimer;
    QRandomGenerator *randomGenerator;
    bool connected {false};

    static const QStringList IcaoList;
};

const QStringList PathCreatorPluginPrivate::IcaoList {"LSZH", "LSGG", "LSME", "LSZW", "LSTZ", "LSZB", "LSMA", "LSZJ", "LSPD", "LSHG", "LSZG", "LSZN", "LSGL", "LSEY", "LSPF"};

// PUBLIC

PathCreatorPlugin::PathCreatorPlugin(QObject *parent) noexcept
    : AbstractSkyConnect {parent},
      d {std::make_unique<PathCreatorPluginPrivate>()}
{
    frenchConnection();
}

PathCreatorPlugin::~PathCreatorPlugin()
{
    closeConnection();
}

bool PathCreatorPlugin::setUserAircraftPositionAndAttitude([[maybe_unused]] const PositionData &positionData, [[maybe_unused]] const AttitudeData &attitudeData) noexcept
{
    return true;
}

// PROTECTED

ConnectPluginBaseSettings &PathCreatorPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

std::optional<std::unique_ptr<OptionWidgetIntf>> PathCreatorPlugin::createExtendedOptionWidget() const noexcept
{
    return std::make_unique<PathCreatorOptionWidget>(d->pluginSettings);
}

bool PathCreatorPlugin::onSetupFlightSimulatorShortcuts() noexcept
{
#ifdef DEBUG
    const auto shortcuts = getPluginSettings().getFlightSimulatorShortcuts();
    qDebug() << "Recording shortcut:" << shortcuts.record.toString();
    qDebug() << "Replay shortcut:" << shortcuts.replay.toString();
    qDebug() << "Pause shortcut:" << shortcuts.pause.toString();
    qDebug() << "Stop shortcut:" << shortcuts.stop.toString();
    qDebug() << "Backward shortcut:" << shortcuts.backward.toString();
    qDebug() << "Forward shortcut:" << shortcuts.forward.toString();
    qDebug() << "Begin shortcut:" << shortcuts.begin.toString();
    qDebug() << "End shortcut:" << shortcuts.end.toString();
#endif
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

bool PathCreatorPlugin::onStartFlightRecording() noexcept
{
    d->recordingTimer.start(::RecordingPeriod);
    recordFlightInfo();
    recordFlightCondition();
    onStartAircraftRecording();
    return true;
}

bool PathCreatorPlugin::onStartAircraftRecording() noexcept
{
    if (!d->recordingTimer.isActive()) {
        d->recordingTimer.start(::RecordingPeriod);
    }
    // Get flight information in case that this is the first recorded aircraft (formation recording)
    const auto &flight = getCurrentFlight();
    const bool hasRecording = flight.hasRecording();
    if (!hasRecording) {
        recordFlightInfo();
        recordFlightCondition();
    }
    recordAircraftInfo();
    return true;
}

void PathCreatorPlugin::onRecordingPaused([[maybe_unused]] Initiator initiator, [[maybe_unused]] bool enable) noexcept
{
    if (enable) {
        d->recordingTimer.stop();
    } else {
        d->recordingTimer.start(::RecordingPeriod);
    }
#ifdef DEBUG
    qDebug() << "PathCreatorPlugin::onRecordingPaused: enable:" << enable;
#endif
}

void PathCreatorPlugin::onStopRecording() noexcept
{
    d->recordingTimer.stop();
    auto &flight = getCurrentFlight();
    FlightCondition flightCondition = flight.getFlightCondition();
    flightCondition.setEndZuluDateTime(QDateTime::currentDateTimeUtc());
    flightCondition.setEndLocalDateTime(flightCondition.getEndZuluDateTime().toLocalTime());
    flight.setFlightCondition(flightCondition);

    auto &aircraft = flight.getUserAircraft();
    auto &flightPlan = aircraft.getFlightPlan();
    auto waypointCount = static_cast<int>(flightPlan.count());
    if (waypointCount > 1) {
        Waypoint waypoint = flightPlan[waypointCount - 1];
        waypoint.zuluTime = QDateTime::currentDateTimeUtc();
        waypoint.localTime = waypoint.zuluTime.toLocalTime();

        flight.updateWaypoint(waypointCount - 1, waypoint);
    }
}

bool PathCreatorPlugin::onStartReplay([[maybe_unused]] std::int64_t currentTimestamp) noexcept {
    d->replayTimer.start(::ReplayPeriod);
    return true;
}

void PathCreatorPlugin::onReplayPaused([[maybe_unused]] Initiator initiator, bool enable) noexcept
{
    if (enable) {
         d->replayTimer.stop();
    } else {
        d->replayTimer.start(::ReplayPeriod);
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
    return d->connected;
}

bool PathCreatorPlugin::connectWithSim() noexcept
{
#ifdef DEBUG
    qDebug() << "PathCreatorPlugin::connectWithSim: CALLED";
#endif
    d->connected = true;
    return d->connected;
}

void PathCreatorPlugin::onDisconnectFromSim() noexcept
{
#ifdef DEBUG
    qDebug() << "PathCreatorPlugin::onDisconnectFromSim: CALLED";
#endif
    closeConnection();
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

bool PathCreatorPlugin::onRequestSimulationRate() noexcept
{
    const auto &settings = Settings::getInstance();
    const float simulationRate = std::min(getReplaySpeedFactor(), static_cast<float>(settings.getMaximumSimulationRate()));
    emit simulationRateReceived(simulationRate);

    return true;
}

bool PathCreatorPlugin::onSendZuluDateTime(int year, int day, int hour, int minute) const noexcept
{
#ifdef DEBUG
    qDebug() << "PathCreatorPlugin::onSendZuluDateTime: year:" << year << "day:" << day << "hour:" << hour << "minute:" << minute;
#endif
    return true;
}

// PRIVATE

void PathCreatorPlugin::frenchConnection() noexcept
{
    connect(&d->pluginSettings, &ConnectPluginBaseSettings::changed,
            this, &PathCreatorPlugin::onPluginSettingsChanged);

    connect(&d->replayTimer, &QTimer::timeout,
            this, &PathCreatorPlugin::replay);
    connect(&d->recordingTimer, &QTimer::timeout,
            this, &PathCreatorPlugin::recordData);
}

void PathCreatorPlugin::recordPositionData(std::int64_t timestamp) noexcept
{
    auto &aircraft = getCurrentFlight().getUserAircraft();

    PositionData positionData;
    positionData.latitude = -90.0 + d->randomGenerator->bounded(180);
    positionData.longitude = -180.0 + d->randomGenerator->bounded(360.0);
    positionData.altitude = d->randomGenerator->bounded(60000.0);
    positionData.indicatedAltitude = std::max(positionData.altitude - 1000.0, 0.0) + d->randomGenerator->bounded(1000.0);
    positionData.calibratedIndicatedAltitude = std::max(positionData.altitude - 1000.0, 0.0) + d->randomGenerator->bounded(1000.0);
    positionData.pressureAltitude = std::max(positionData.altitude - 1000.0, 0.0) + d->randomGenerator->bounded(1000.0);
    positionData.timestamp = timestamp;
    aircraft.getPosition().upsertLast(positionData);

    AttitudeData attitudeData;
    attitudeData.pitch = -90.0 + d->randomGenerator->bounded(180.0);
    attitudeData.bank = -180.0 + d->randomGenerator->bounded(360.0);
    attitudeData.trueHeading = -180.0 + d->randomGenerator->bounded(360.0);

    attitudeData.velocityBodyX = d->randomGenerator->bounded(1.0);
    attitudeData.velocityBodyY = d->randomGenerator->bounded(1.0);
    attitudeData.velocityBodyZ = d->randomGenerator->bounded(1.0);
    attitudeData.onGround = false;
    attitudeData.timestamp = timestamp;
    aircraft.getAttitude().upsertLast(attitudeData);
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
    auto &aircraft = getCurrentFlight().getUserAircraft();
    aircraft.getEngine().upsertLast(engineData);
}

void PathCreatorPlugin::recordPrimaryControls(std::int64_t timestamp) noexcept
{
    PrimaryFlightControlData primaryFlightControlData;
    primaryFlightControlData.rudderDeflection = static_cast<float>(Convert::degreesToRadians(-45.0 + d->randomGenerator->bounded(90.0)));
    primaryFlightControlData.elevatorDeflection = static_cast<float>(Convert::degreesToRadians(-45.0 + d->randomGenerator->bounded(90.0)));
    primaryFlightControlData.leftAileronDeflection = static_cast<float>(Convert::degreesToRadians(-45.0 + d->randomGenerator->bounded(90.0)));
    primaryFlightControlData.rightAileronDeflection = static_cast<float>(Convert::degreesToRadians(-45.0 + d->randomGenerator->bounded(90.0)));
    primaryFlightControlData.rudderPosition = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.elevatorPosition = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));
    primaryFlightControlData.aileronPosition = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));

    primaryFlightControlData.timestamp = timestamp;
    auto &aircraft = getCurrentFlight().getUserAircraft();
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
    auto &aircraft = getCurrentFlight().getUserAircraft();
    aircraft.getSecondaryFlightControl().upsertLast(secondaryFlightControlData);
}

void PathCreatorPlugin::recordAircraftHandle(std::int64_t timestamp) noexcept
{
    AircraftHandleData aircraftHandleData;
    aircraftHandleData.brakeLeftPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    aircraftHandleData.brakeRightPosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    aircraftHandleData.gearSteerPosition = SkyMath::fromNormalisedPosition(-1.0 + d->randomGenerator->bounded(2.0));
    aircraftHandleData.waterRudderHandlePosition = SkyMath::fromNormalisedPosition(d->randomGenerator->bounded(1.0));
    aircraftHandleData.tailhookPosition = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.canopyOpen = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.leftWingFolding = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.rightWingFolding = SkyMath::fromPercent(d->randomGenerator->bounded(100.0));
    aircraftHandleData.gearHandlePosition = d->randomGenerator->bounded(2) < 1 ? false : true;
    aircraftHandleData.tailhookHandlePosition = d->randomGenerator->bounded(2) < 1 ? false : true;
    aircraftHandleData.foldingWingHandlePosition = d->randomGenerator->bounded(2) < 1 ? false : true;

    aircraftHandleData.timestamp = timestamp;
    auto &aircraft = getCurrentFlight().getUserAircraft();
    aircraft.getAircraftHandle().upsertLast(aircraftHandleData);
}

void PathCreatorPlugin::recordLights(std::int64_t timestamp) noexcept
{
    static int lights = 0;
    LightData lightData;
    lightData.lightStates = static_cast<SimType::LightStates>(lights);
    lights = ++lights % 0b1111111111;

    lightData.timestamp = timestamp;
    auto &aircraft = getCurrentFlight().getUserAircraft();
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
        waypoint.zuluTime = QDateTime::currentDateTimeUtc();
        waypoint.localTime = waypoint.zuluTime.toLocalTime();
        waypoint.timestamp = timestamp;

        auto &flight = getCurrentFlight();
        flight.addWaypoint(waypoint);
    }
}

void PathCreatorPlugin::recordFlightInfo() noexcept
{
    getCurrentFlight().setFlightNumber(QString::number(d->randomGenerator->bounded(100)));
}

void PathCreatorPlugin::recordFlightCondition() noexcept
{
    FlightCondition flightCondition;

    flightCondition.groundAltitude = static_cast<float>(d->randomGenerator->bounded(4000.0));
    flightCondition.surfaceType = static_cast<SimType::SurfaceType>(d->randomGenerator->bounded(Enum::underly(SimType::SurfaceType::Last) + 1));
    flightCondition.surfaceCondition = static_cast<SimType::SurfaceCondition>(d->randomGenerator->bounded(Enum::underly(SimType::SurfaceCondition::Last) + 1));
    flightCondition.ambientTemperature = static_cast<float>(d->randomGenerator->bounded(80.0f)) - 40.0f;
    flightCondition.totalAirTemperature = static_cast<float>(d->randomGenerator->bounded(80.0f)) - 40.0f;
    flightCondition.windSpeed = static_cast<float>(d->randomGenerator->bounded(30.0));
    flightCondition.windDirection = static_cast<float>(d->randomGenerator->bounded(360.0));
    flightCondition.precipitationState = static_cast<SimType::PrecipitationState>(d->randomGenerator->bounded(Enum::underly(SimType::PrecipitationState::Last) + 1));
    flightCondition.visibility = static_cast<float>(d->randomGenerator->bounded(10000.0));
    flightCondition.seaLevelPressure = 950.0f + static_cast<float>(d->randomGenerator->bounded(100.0));
    flightCondition.pitotIcingPercent = d->randomGenerator->bounded(101);
    flightCondition.structuralIcingPercent = d->randomGenerator->bounded(101);
    flightCondition.inClouds = d->randomGenerator->bounded(2) < 1 ? false : true;
    flightCondition.onAnyRunway = d->randomGenerator->bounded(2) < 1 ? false : true;
    flightCondition.onParkingSpot = d->randomGenerator->bounded(2) < 1 ? false : true;
    flightCondition.setStartZuluDateTime(QDateTime::currentDateTimeUtc());
    flightCondition.setStartLocalDateTime(flightCondition.getStartZuluDateTime().toLocalTime());

    getCurrentFlight().setFlightCondition(flightCondition);
}

void PathCreatorPlugin::recordAircraftInfo() noexcept
{
    auto &flight = getCurrentFlight();
    auto &aircraft = flight.getUserAircraft();
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
    info.altitudeAboveGround = static_cast<float>(d->randomGenerator->bounded(40000.0));
    info.startOnGround = d->randomGenerator->bounded(2) > 0 ? true : false;
    info.initialAirspeed = d->randomGenerator->bounded(600);

    aircraft.setAircraftInfo(info);
    emit flight.aircraftInfoChanged(aircraft);
}

void PathCreatorPlugin::closeConnection() noexcept
{
    d->connected = false;
}

// PRIVATE SLOTS

void PathCreatorPlugin::replay() noexcept
{
    const auto timestamp = updateCurrentTimestamp();
    if (!sendAircraftData(timestamp, TimeVariableData::Access::Linear, AircraftSelection::All)) {
        onEndReached();
    }
}

void PathCreatorPlugin::recordData() noexcept
{
    if (!isElapsedTimerRunning()) {
        // Start the elapsed timer with the arrival of the first sample data
        setCurrentTimestamp(0);
        resetElapsedTime(true);
    }

    const auto timestamp = updateCurrentTimestamp();
    recordPositionData(timestamp);
    recordEngineData(timestamp);
    recordPrimaryControls(timestamp);
    recordSecondaryControls(timestamp);
    recordAircraftHandle(timestamp);
    recordLights(timestamp);
    recordWaypoint(timestamp);
}
