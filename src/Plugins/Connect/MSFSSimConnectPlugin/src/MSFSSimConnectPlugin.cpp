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
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

#include <QTimer>
#include <QApplication>
#include <QWidget>
#include <QDateTime>
#ifdef DEBUG
#include <QDebug>
#endif

#include <tsl/ordered_map.h>

#include <Kernel/Const.h>
#include <Kernel/SampleRate.h>
#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
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
#include <Model/InitialPosition.h>
#include <PluginManager/Connect.h>

#include "SimVar/SimulationVariables.h"
#include "Event/SimConnectEvent.h"
#include "Event/EventWidget.h"
#include "SimConnectType.h"
#include "SimConnectAi.h"
#include "MSFSSimConnectPlugin.h"

namespace
{
    constexpr const char *ConnectionName {"SkyConnect"};
    constexpr DWORD UserAirplaneRadiusMeters {0};
}

class SkyConnectPrivate
{
public:
    PositionData currentPositionData;
    EngineData currentEngineData;
    PrimaryFlightControlData currentPrimaryFlightControlData;
    SecondaryFlightControlData currentSecondaryFlightControlData;
    AircraftHandleData currentAircraftHandleData;
    LightData currentLightData;
    QDateTime currentLocalDateTime;
    QDateTime currentZuluDateTime;
    HANDLE simConnectHandle{nullptr};
    std::unique_ptr<SimConnectEvent> simConnectEvent {nullptr};
    std::unique_ptr<SimConnectAi> simConnectAi {nullptr};
    std::unique_ptr<EventWidget> eventWidget {std::make_unique<EventWidget>()};
    ::SIMCONNECT_PERIOD currentRequestPeriod {::SIMCONNECT_PERIOD_NEVER};
    // Insert order is order of flight plan
    tsl::ordered_map<QString, Waypoint> flightPlan;    
    bool pendingWaypointTime {false};
    bool storeDataImmediately {true};
};

// PUBLIC

MSFSSimConnectPlugin::MSFSSimConnectPlugin(QObject *parent) noexcept
    : AbstractSkyConnect(parent),
      d(std::make_unique<SkyConnectPrivate>())
{
    frenchConnection();
}

MSFSSimConnectPlugin::~MSFSSimConnectPlugin() noexcept
{
    if (d->simConnectEvent != nullptr) {
        d->simConnectEvent->freezeAircraft(::SIMCONNECT_OBJECT_ID_USER, false);
    }
    close();
}

bool MSFSSimConnectPlugin::setUserAircraftPosition(const PositionData &positionData) noexcept
{
    SimConnectPositionUser simConnnectPositionUser {positionData};
    const HRESULT result = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionUser),
                                                           ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                           sizeof(simConnnectPositionUser), &simConnnectPositionUser);
    return result == S_OK;
}

// PROTECTED

bool MSFSSimConnectPlugin::isTimerBasedRecording(SampleRate::SampleRate sampleRate) const noexcept
{
    // "Auto" and 1 Hz sample rates are processed event-based
    return sampleRate != SampleRate::SampleRate::Auto && sampleRate != SampleRate::SampleRate::Hz1;
}

bool MSFSSimConnectPlugin::onInitialPositionSetup(const InitialPosition &initialPosition) noexcept
{
    SIMCONNECT_DATA_INITPOSITION initialSimConnectPosition = SimConnectPositionCommon::toInitialPosition(initialPosition);
    HRESULT result = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::InitialPosition),
                                                     ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(::SIMCONNECT_DATA_INITPOSITION),
                                                     &initialSimConnectPosition);
    return result == S_OK;
}

bool MSFSSimConnectPlugin::onFreezeUserAircraft(bool enable) const noexcept
{
    return d->simConnectEvent->freezeAircraft(::SIMCONNECT_OBJECT_ID_USER, enable);
}

bool MSFSSimConnectPlugin::onSimulationEvent(SimulationEvent event) const noexcept
{
    HRESULT result {S_OK};

    switch (event) {
    case SimulationEvent::EngineStart:
        result = ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoStart), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        break;
    case SimulationEvent::EngineStop:
        result = ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::underly(SimConnectEvent::Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        break;
    case SimulationEvent::None:
        // Nothing to do
        break;
    }
    return result == S_OK;
}

bool MSFSSimConnectPlugin::onStartRecording() noexcept
{
    resetCurrentSampleData();
    updateRecordingFrequency(Settings::getInstance().getRecordingSampleRate());

    // Initialise flight plan
    d->flightPlan.clear();

    // Get aircraft information
    HRESULT result = ::SimConnect_RequestDataOnSimObjectType(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::AircraftInfo), Enum::underly(SimConnectType::DataDefinition::FlightInformation), ::UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
    bool ok = result == S_OK;

    // For formation flights (count > 1) send AI aircraft positions every visual frame
    if (ok && getCurrentFlight().count() > 1) {
        result = ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::underly(SimConnectEvent::Event::Frame), "Frame");
        ok = result == S_OK;
    }
    return ok;
}

void MSFSSimConnectPlugin::onRecordingPaused(bool enable) noexcept
{
    updateRecordingFrequency(Settings::getInstance().getRecordingSampleRate());
    d->simConnectEvent->pauseSimulation(enable);
}

void MSFSSimConnectPlugin::onStopRecording() noexcept
{
    // Stop receiving "frame" events
    ::SimConnect_UnsubscribeFromSystemEvent(d->simConnectHandle, Enum::underly(SimConnectEvent::Event::Frame));

    // Stop receiving aircraft position
    updateRequestPeriod(::SIMCONNECT_PERIOD_NEVER);

    d->simConnectEvent->resumePausedSimulation();

    // Update flight plan
    Flight &flight = getCurrentFlight();
    const Aircraft &userAircraft = flight.getUserAircraft();
    FlightPlan &flightPlan = userAircraft.getFlightPlan();
    for (const auto &it : d->flightPlan) {
        flight.addWaypoint(it.second);
    }

    // Update timestamp and simulation time of last waypoint
    int waypointCount = static_cast<int>(flightPlan.count());
    if (waypointCount > 1) {
        Waypoint waypoint = flightPlan[waypointCount - 1];
        waypoint.localTime = d->currentLocalDateTime;
        waypoint.zuluTime = d->currentZuluDateTime;
        waypoint.timestamp = getCurrentTimestamp();
        flight.updateWaypoint(waypointCount - 1, waypoint);
    } else if (waypointCount == 0 && userAircraft.getPosition().count() > 0) {
        Waypoint departureWaypoint;
        const PositionData &firstPosition = userAircraft.getPosition().getFirst();
        departureWaypoint.identifier = Waypoint::CustomDepartureIdentifier;
        departureWaypoint.latitude = static_cast<float>(firstPosition.latitude);
        departureWaypoint.longitude = static_cast<float>(firstPosition.longitude);
        departureWaypoint.altitude = static_cast<float>(firstPosition.altitude);
        departureWaypoint.localTime = flight.getFlightCondition().startLocalTime;
        departureWaypoint.zuluTime = flight.getFlightCondition().startZuluTime;
        departureWaypoint.timestamp = 0;
        flight.addWaypoint(departureWaypoint);

        Waypoint arrivalWaypoint;
        const PositionData &lastPosition = userAircraft.getPosition().getLast();
        arrivalWaypoint.identifier = Waypoint::CustomArrivalIdentifier;
        arrivalWaypoint.latitude = static_cast<float>(lastPosition.latitude);
        arrivalWaypoint.longitude = static_cast<float>(lastPosition.longitude);
        arrivalWaypoint.altitude = static_cast<float>(lastPosition.altitude);
        arrivalWaypoint.localTime = d->currentLocalDateTime;
        arrivalWaypoint.zuluTime = d->currentZuluDateTime;
        arrivalWaypoint.timestamp = std::max(getCurrentTimestamp(), departureWaypoint.timestamp + 1);
        flight.addWaypoint(arrivalWaypoint);
    }

    // Update end simulation time of flight conditions
    FlightCondition condition = flight.getFlightCondition();
    condition.endLocalTime = d->currentLocalDateTime;
    condition.endZuluTime = d->currentZuluDateTime;
    flight.setFlightCondition(condition);
}

bool MSFSSimConnectPlugin::onStartReplay(std::int64_t currentTimestamp) noexcept
{
    d->simConnectEvent->reset();

    // Send aircraft position every visual frame
    HRESULT result = ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::underly(SimConnectEvent::Event::Frame), "Frame");
    return result == S_OK;
}

void MSFSSimConnectPlugin::onReplayPaused(bool enable) noexcept
{
    if (enable) {
        ::SimConnect_UnsubscribeFromSystemEvent(d->simConnectHandle, Enum::underly(SimConnectEvent::Event::Frame));
    } else {
        ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::underly(SimConnectEvent::Event::Frame), "Frame");
    }
    d->simConnectEvent->pauseSimulation(enable);
}

void MSFSSimConnectPlugin::onStopReplay() noexcept
{
    ::SimConnect_UnsubscribeFromSystemEvent(d->simConnectHandle, Enum::underly(SimConnectEvent::Event::Frame));
    d->simConnectEvent->resumePausedSimulation();
}

void MSFSSimConnectPlugin::onSeek(std::int64_t currentTimestamp) noexcept
{
    d->simConnectEvent->reset();
};

void MSFSSimConnectPlugin::onRecordingSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept
{
     updateRecordingFrequency(sampleRate);
}

bool MSFSSimConnectPlugin::sendAircraftData(std::int64_t currentTimestamp, TimeVariableData::Access access, AircraftSelection aircraftSelection) noexcept
{
    const Flight &flight = getCurrentFlight();
    // In case of "fly with formation" always send all formation aircraft (as AI aircraft): we simply do this
    // by setting the userAircraftId to an invalid ID, so no aircraft in the Flight is considered the "user aircraft"
    // (which is really being controlled by the user as an "additional aircraft", next to the formation)
    const std::int64_t userAircraftId = getReplayMode() != ReplayMode::FlyWithFormation ?  flight.getUserAircraft().getId() : Const::InvalidId;
    bool ok {true};
    for (auto &aircraft : flight) {

        // Replay AI aircraft - if any - during recording (if all aircraft are selected for replay)
        const bool isUserAircraft = aircraft.getId() == userAircraftId;
        if (isUserAircraft && getReplayMode() == ReplayMode::UserAircraftManualControl) {
            // The user aircraft (of the formation) is manually flown
            continue;
        }

        if (!isUserAircraft && aircraftSelection == AircraftSelection::UserAircraft) {
            // Only the user aircraft is to be sent
            continue;
        }

        // When recording (a formation flight) we send the already recorded aircraft, except the
        // user aircraft (which is currently being recorded)
        if (isConnectedWithSim() &&  getState() != Connect::State::Recording || !isUserAircraft) {
            const std::int64_t objectId = isUserAircraft ? ::SIMCONNECT_OBJECT_ID_USER : d->simConnectAi->getSimulatedObjectByAircraftId(aircraft.getId());
            if (objectId != SimConnectAi::InvalidObjectId) {

                ok = true;
                const PositionData &positionData = aircraft.getPosition().interpolate(currentTimestamp, access);
                if (!positionData.isNull()) {
                    SimConnectPositionAll simConnnectPositionAll {positionData};
                    if (isUserAircraft) {
                        SimConnectPositionUser positionUser = simConnnectPositionAll.user();
                        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionUser),
                                                                            objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                            sizeof(SimConnectPositionUser), &positionUser);
                        ok = res == S_OK;
                    } else {
                        SimConnectPositionAi positionAi = simConnnectPositionAll.ai();
                        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PositionAi),
                                                                            objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                            sizeof(SimConnectPositionAi), &positionAi);
                        ok = res == S_OK;
                    }
                }

                // Engine
                if (ok) {
                    const EngineData &engineData = aircraft.getEngine().interpolate(currentTimestamp, access);
                    if (!engineData.isNull()) {
                        SimConnectEngineAll simConnectEngine {engineData};
                        if (isUserAircraft) {
                            SimConnectEngineUser engineUser = simConnectEngine.user();
                            const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineUser),
                                                                                objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                                sizeof(SimConnectEngineUser), &engineUser);
                            ok = res == S_OK;
                            if (ok) {
                                ok = d->simConnectEvent->sendEngine(simConnectEngine);
                            }
                        } else {
                            SimConnectEngineAi engineAi = simConnectEngine.ai();
                            const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::EngineAi),
                                                                                objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                                sizeof(SimConnectEngineAi), &engineAi);
                            ok = res == S_OK;
                        }
                    }
                }

                // Primary flight controls
                if (ok) {
                    const PrimaryFlightControlData &primaryFlightControlData = aircraft.getPrimaryFlightControl().interpolate(currentTimestamp, access);
                    if (!primaryFlightControlData.isNull()) {
                        if (isUserAircraft) {
                            SimConnectPrimaryFlightControl simConnectPrimaryFlightControl;
                            simConnectPrimaryFlightControl.fromPrimaryFlightControlData(primaryFlightControlData);
                            ok = d->simConnectEvent->sendPrimaryFlightControls(simConnectPrimaryFlightControl);
                        }
                        else {
                            SimConnectPrimaryFlightControlAnimation simConnectPrimaryFlightControlAnimation;
                            simConnectPrimaryFlightControlAnimation.fromPrimaryFlightControlData(primaryFlightControlData);
                            HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::PrimaryFlightControlAnimation),
                                                                          objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                          sizeof(SimConnectPrimaryFlightControlAnimation), &simConnectPrimaryFlightControlAnimation);
                            ok = res == S_OK;
                        }
                    }
                }

                // Secondary flight controls
                if (ok) {
                    const SecondaryFlightControlData &secondaryFlightControlData = aircraft.getSecondaryFlightControl().interpolate(currentTimestamp, access);
                    if (!secondaryFlightControlData.isNull()) {
                        SimConnectSecondaryFlightControl simConnectSecondaryFlightControl;
                        simConnectSecondaryFlightControl.fromSecondaryFlightControlData(secondaryFlightControlData);
                        if (isUserAircraft) {
                            ok = d->simConnectEvent->sendFlapsHandleIndex(simConnectSecondaryFlightControl.flapsHandleIndex);
                        } else {
                            const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::SecondaryFlightControl),
                                                                                objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                                sizeof(SimConnectSecondaryFlightControl), &simConnectSecondaryFlightControl);
                            ok = res == S_OK;
                        }
                    }
                }

                // Aircraft handles & brakes
                if (ok) {
                    const AircraftHandleData &aircraftHandleData = aircraft.getAircraftHandle().interpolate(currentTimestamp, access);
                    if (!aircraftHandleData.isNull()) {
                        SimConnectAircraftHandle simConnectAircraftHandle;
                        simConnectAircraftHandle.fromAircraftHandleData(aircraftHandleData);
                        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::Handle),
                                                                            objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                            sizeof(SimConnectAircraftHandle), &simConnectAircraftHandle);
                        ok = res == S_OK;
                        if (isUserAircraft) {
                            if (ok) {
                                ok = d->simConnectEvent->setGear(simConnectAircraftHandle.gearHandlePosition);
                            }
                        }
                    }
                }

                // Lights
                if (ok) {
                    const LightData &lightData = aircraft.getLight().interpolate(currentTimestamp, access);
                    if (!lightData.isNull()) {
                        SimConnectLight simConnectLight;
                        simConnectLight.fromLightData(lightData);
                        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::Light),
                                                                            objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                            sizeof(SimConnectLight), &simConnectLight);
                        ok = res == S_OK;
                    }
                }

            } // User aircraft or valid simulation object ID

        } // User aircraft not sent during recording

    } // All aircraft

    // Start the elapsed timer after sending the first sample data, but
    // only when not recording (the first received sample will start the timer then)
    if (!isElapsedTimerRunning() && access != TimeVariableData::Access::Seek && getState() != Connect::State::Recording) {
        startElapsedTimer();
    }
    return ok;
}

bool MSFSSimConnectPlugin::isConnectedWithSim() const noexcept
{
    return d->simConnectHandle != nullptr;
}

bool MSFSSimConnectPlugin::connectWithSim() noexcept
{
    auto hWnd = reinterpret_cast<HWND>(d->eventWidget->winId());
    DWORD userEvent = EventWidget::SimConnnectUserMessage;
    HRESULT result = ::SimConnect_Open(&(d->simConnectHandle), ::ConnectionName, hWnd, userEvent, nullptr, ::SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
    if (result == S_OK) {
        d->simConnectEvent = std::make_unique<SimConnectEvent>(d->simConnectHandle);
        d->simConnectAi = std::make_unique<SimConnectAi>(d->simConnectHandle);
        setupRequestData();
    }
#ifdef DEBUG
    qDebug() << "MSFSSimConnectPlugin::connectWithSim: CONNECT with SIM, handle:" << d->simConnectHandle << "success:" << (result == S_OK);
#endif
    const bool ok = result == S_OK;
    if (ok) {
        createAiObjects();
    }
    return ok;
}

void MSFSSimConnectPlugin::onAddAiObject(const Aircraft &aircraft) noexcept
{
    // Check if initialised (only when connected with MSFS)
    if (d->simConnectAi != nullptr) {
        d->simConnectAi->addObject(aircraft, getCurrentTimestamp());
    }
}

void MSFSSimConnectPlugin::onRemoveAiObject(std::int64_t aircraftId) noexcept
{
    if (d->simConnectAi != nullptr) {
        d->simConnectAi->removeByAircraftId(aircraftId);
    }
}

void MSFSSimConnectPlugin::onRemoveAllAiObjects() noexcept
{
    if (d->simConnectAi != nullptr) {
        d->simConnectAi->removeAllObjects();
    }
}

bool MSFSSimConnectPlugin::onRequestLocation() noexcept
{
    const HRESULT result = ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::Location),
                                                               Enum::underly(SimConnectType::DataDefinition::Location),
                                                               ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_ONCE, 0);
    return result == S_OK;
}

// PROTECTED SLOTS

void MSFSSimConnectPlugin::recordData() noexcept
{
    Aircraft &userAircraft = getCurrentFlight().getUserAircraft();
    bool dataStored = false;
    if (!d->currentPositionData.isNull()) {
        userAircraft.getPosition().upsertLast(d->currentPositionData);
        // Processed
        dataStored = true;
    }
    if (!d->currentEngineData.isNull()) {
        userAircraft.getEngine().upsertLast(d->currentEngineData);
        // Processed
        dataStored = true;
    }
    if (!d->currentPrimaryFlightControlData.isNull()) {
        userAircraft.getPrimaryFlightControl().upsertLast(d->currentPrimaryFlightControlData);
        // Processed
        dataStored = true;
    }
    if (!d->currentSecondaryFlightControlData.isNull()) {
        userAircraft.getSecondaryFlightControl().upsertLast(d->currentSecondaryFlightControlData);
        // Processed
        dataStored = true;
    }
    if (!d->currentAircraftHandleData.isNull()) {
        userAircraft.getAircraftHandle().upsertLast(d->currentAircraftHandleData);
        // Processed
        dataStored = true;
    }
    if (!d->currentLightData.isNull()) {
        userAircraft.getLight().upsertLast(d->currentLightData);
        // Processed
        dataStored = true;
    }
    if (dataStored) {
        if (!isElapsedTimerRunning()) {
            // Start the elapsed timer with the storage of the first sampled data
            setCurrentTimestamp(0);
            resetElapsedTime(true);
        }
    }
}

// PRIVATE

void MSFSSimConnectPlugin::frenchConnection() noexcept
{
    connect(d->eventWidget.get(), &EventWidget::simConnectEvent,
            this, &MSFSSimConnectPlugin::processSimConnectEvent);
}

void MSFSSimConnectPlugin::resetCurrentSampleData() noexcept
{
    d->currentPositionData.reset();
    d->currentEngineData.reset();
    d->currentPrimaryFlightControlData.reset();
    d->currentSecondaryFlightControlData.reset();
    d->currentAircraftHandleData.reset();
    d->currentLightData.reset();
}

bool MSFSSimConnectPlugin::reconnectWithSim() noexcept
{
    bool res {false};
    if (close()) {
        res = connectWithSim();
    }
    return res;
}

bool MSFSSimConnectPlugin::close() noexcept
{
    HRESULT result {S_OK};
    if (d->simConnectAi != nullptr) {
        d->simConnectAi = nullptr;
    }
    if (d->simConnectHandle != nullptr) {
        result = ::SimConnect_Close(d->simConnectHandle);
        d->simConnectHandle = nullptr;
    }
    return result == S_OK;
}

void MSFSSimConnectPlugin::setupRequestData() noexcept
{
    // Request data
    SimConnectAircraftInfo::addToDataDefinition(d->simConnectHandle);
    SimConnectPositionUser::addToDataDefinition(d->simConnectHandle);
    SimConnectPositionAi::addToDataDefinition(d->simConnectHandle);
    SimConnectPositionAll::addToDataDefinition(d->simConnectHandle);
    SimConnectEngineUser::addToDataDefinition(d->simConnectHandle);
    SimConnectEngineAi::addToDataDefinition(d->simConnectHandle);
    SimConnectEngineAll::addToDataDefinition(d->simConnectHandle);
    SimConnectPrimaryFlightControl::addToDataDefinition(d->simConnectHandle);
    SimConnectPrimaryFlightControlAnimation::addToDataDefinition(d->simConnectHandle);
    SimConnectPrimaryFlightControlReply::addToDataDefinition(d->simConnectHandle);
    SimConnectSecondaryFlightControl::addToDataDefinition(d->simConnectHandle);
    SimConnectAircraftHandle::addToDataDefinition(d->simConnectHandle);
    SimConnectLight::addToDataDefinition(d->simConnectHandle);
    SimConnectFlightPlan::addToDataDefinition(d->simConnectHandle);
    SimConnectSimulationTime::addToDataDefinition(d->simConnectHandle);
    SimConnectLocation::addToDataDefinition(d->simConnectHandle);
    // Simulation variables
    FlapsHandleIndex::addToDataDefinition(d->simConnectHandle);

    ::SimConnect_AddToDataDefinition(d->simConnectHandle, Enum::underly(SimConnectType::DataDefinition::InitialPosition), "Initial Position", nullptr, ::SIMCONNECT_DATATYPE_INITPOSITION);

    d->simConnectEvent->setupEvents();
}

void MSFSSimConnectPlugin::replay() noexcept
{
    const std::int64_t currentTimestamp = getCurrentTimestamp();
    if (currentTimestamp <= getCurrentFlight().getTotalDurationMSec()) {
        if (!sendAircraftData(currentTimestamp, TimeVariableData::Access::Linear, AircraftSelection::All)) {
            // Connection error
            stopReplay();
        }
    } else {
        // At end of replay
        handleAtEnd();
    }
}

void MSFSSimConnectPlugin::updateRecordingFrequency(SampleRate::SampleRate sampleRate) noexcept
{
    if (getState() == Connect::State::Recording) {
        switch (sampleRate) {
        case SampleRate::SampleRate::Hz1:
            // Get aircraft data @1Hz
            updateRequestPeriod(::SIMCONNECT_PERIOD_SECOND);
            d->storeDataImmediately = true;
            break;
        case SampleRate::SampleRate::Auto:
            // The received data is immediately stored in the aircraft data
            d->storeDataImmediately = true;
            // Get aircraft data every simulated frame
            updateRequestPeriod(::SIMCONNECT_PERIOD_SIM_FRAME);
            break;
        default:
            // The received data is temporarily stored until processed by the
            // timer-based processData method
            d->storeDataImmediately = false;
            // Get aircraft data every simulated frame; based on the actual
            // recording frequency samples are being overwritten, until processed by
            // the timer-based processData method
            updateRequestPeriod(::SIMCONNECT_PERIOD_SIM_FRAME);
            break;
        }
    } else {
        updateRequestPeriod(::SIMCONNECT_PERIOD_NEVER);
    }
}

void MSFSSimConnectPlugin::updateRequestPeriod(::SIMCONNECT_PERIOD period) noexcept
{
    if (d->currentRequestPeriod != period) {
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::AircraftPositionReply),
                                            Enum::underly(SimConnectType::DataDefinition::PositionAll),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::EngineAll),
                                            Enum::underly(SimConnectType::DataDefinition::EngineAll),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::PrimaryFlightControlReply),
                                            Enum::underly(SimConnectType::DataDefinition::PrimaryFlightControlReply),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::SecondaryFlightControl),
                                            Enum::underly(SimConnectType::DataDefinition::SecondaryFlightControl),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::AircraftHandle),
                                            Enum::underly(SimConnectType::DataDefinition::Handle),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::Light),
                                            Enum::underly(SimConnectType::DataDefinition::Light),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        // Update the flight plan and simulation time only every second
        ::SIMCONNECT_PERIOD oneSecondPeriod = period != ::SIMCONNECT_PERIOD_NEVER ? ::SIMCONNECT_PERIOD_SECOND : ::SIMCONNECT_PERIOD_NEVER;
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::FlightPlan),
                                            Enum::underly(SimConnectType::DataDefinition::FlightPlan),
                                            ::SIMCONNECT_OBJECT_ID_USER, oneSecondPeriod, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::underly(SimConnectType::DataRequest::SimulationTime),
                                            Enum::underly(SimConnectType::DataDefinition::SimulationTime),
                                            ::SIMCONNECT_OBJECT_ID_USER, oneSecondPeriod, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        d->currentRequestPeriod = period;
    }
}

void CALLBACK MSFSSimConnectPlugin::dispatch(::SIMCONNECT_RECV *receivedData, [[maybe_unused]] DWORD cbData, void *context) noexcept
{
    auto skyConnect = static_cast<MSFSSimConnectPlugin *>(context);
    Flight &flight = skyConnect->getCurrentFlight();
    Aircraft &userAircraft = flight.getUserAircraft();
    ::SIMCONNECT_RECV_SIMOBJECT_DATA *objectData {nullptr};

    bool dataStored = false;
    switch (receivedData->dwID) {
    case ::SIMCONNECT_RECV_ID_EVENT:
    {
        const ::SIMCONNECT_RECV_EVENT *evt = static_cast<::SIMCONNECT_RECV_EVENT *>(receivedData);
        switch (static_cast<SimConnectEvent::Event>(evt->uEventID)) {
        case SimConnectEvent::Event::SimStart:
#ifdef DEBUG
            qDebug() << "MSFSSimConnectPlugin::dispatch: SIMCONNECT_RECV_ID_EVENT: SIMSTART event";
#endif
            break;

        case SimConnectEvent::Event::Pause:
#ifdef DEBUG
            qDebug() << "MSFSSimConnectPlugin::dispatch: SIMCONNECT_RECV_ID_EVENT: PAUSE event:" << evt->dwData;
#endif
            // It seems that the pause event is currently only triggered by selecting "Pause Simulation"
            // in the developer mode (MSFS), but neither when "active pause" is selected nor when ESC
            // (in-game meu") is entered; also, we ignore the first "unpause" event (which is always
            // sent by MSFS after the initial connect), as we explicitly pause the replay after having
            // loaded a flight: we simply do this by assuming that no "unpause" would normally be sent
            // at the very beginning (timestamp 0) of the replay
            if (evt->dwData > 0 || skyConnect->getCurrentTimestamp() > 0) {
                skyConnect->setPaused(evt->dwData == 1);
            }
            break;

        case SimConnectEvent::Event::Crashed:
#ifdef DEBUG
            qDebug() << "MSFSSimConnectPlugin::dispatch: SIMCONNECT_RECV_ID_EVENT: CRASHED event";
#endif
            switch (skyConnect->getState()) {
            case Connect::State::Recording:
                skyConnect->stopRecording();
                break;
            case Connect::State::Replay:
                skyConnect->stopReplay();
                break;
            default:
                break;
            }
            break;

        default:
            break;
        }
        break;
    }

    case ::SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
    {
        objectData = static_cast<::SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *>(receivedData);
        switch (static_cast<SimConnectType::DataRequest>(objectData->dwRequestID)) {
        case SimConnectType::DataRequest::AircraftInfo:
        {
            auto simConnectAircraftInfo = reinterpret_cast<const SimConnectAircraftInfo *>(&objectData->dwData);
            AircraftInfo aircraftInfo = simConnectAircraftInfo->toAircraftInfo();
            userAircraft.setAircraftInfo(aircraftInfo);
            emit flight.aircraftInfoChanged(userAircraft);
            FlightCondition flightCondition = simConnectAircraftInfo->toFlightCondition();
            flight.setFlightCondition(flightCondition);
            break;
        }
        default:
            break;
        }
        break;
    }

    case ::SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
    {
        objectData = static_cast<::SIMCONNECT_RECV_SIMOBJECT_DATA *>(receivedData);

        const bool storeDataImmediately = skyConnect->d->storeDataImmediately;
        switch (static_cast<SimConnectType::DataRequest>(objectData->dwRequestID)) {
        case SimConnectType::DataRequest::AircraftPositionReply:
        {
            if (skyConnect->getState() == Connect::State::Recording) {
                auto simConnectPositionAll = reinterpret_cast<const SimConnectPositionAll *>(&objectData->dwData);
                PositionData positionData = simConnectPositionAll->toPositionData();
                positionData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getPosition().upsertLast(positionData);
                    dataStored = true;
                } else {
                    skyConnect->d->currentPositionData = std::move(positionData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::EngineAll:
        {
            if (skyConnect->getState() == Connect::State::Recording) {
                auto simConnectEngineAll = reinterpret_cast<const SimConnectEngineAll *>(&objectData->dwData);
                EngineData engineData = simConnectEngineAll->toEngineData();
                engineData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getEngine().upsertLast(engineData);
                    dataStored = true;
                } else {
                    skyConnect->d->currentEngineData = std::move(engineData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::PrimaryFlightControlReply:
        {
            if (skyConnect->getState() == Connect::State::Recording) {
                auto simConnectPrimaryFlightControlReply = reinterpret_cast<const SimConnectPrimaryFlightControlReply *>(&objectData->dwData);
                PrimaryFlightControlData primaryFlightControlData = simConnectPrimaryFlightControlReply->toPrimaryFlightControlData();
                primaryFlightControlData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getPrimaryFlightControl().upsertLast(primaryFlightControlData);
                    dataStored = true;
                } else {
                    skyConnect->d->currentPrimaryFlightControlData = std::move(primaryFlightControlData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::SecondaryFlightControl:
        {
            if (skyConnect->getState() == Connect::State::Recording) {
                auto simConnectSecondaryFlightControl = reinterpret_cast<const SimConnectSecondaryFlightControl *>(&objectData->dwData);
                SecondaryFlightControlData secondaryFlightControlData = simConnectSecondaryFlightControl->toSecondaryFlightControlData();
                secondaryFlightControlData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getSecondaryFlightControl().upsertLast(secondaryFlightControlData);
                    dataStored = true;
                } else {
                    skyConnect->d->currentSecondaryFlightControlData = std::move(secondaryFlightControlData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::AircraftHandle:
        {
            if (skyConnect->getState() == Connect::State::Recording) {
                auto simConnectAircraftHandle = reinterpret_cast<const SimConnectAircraftHandle *>(&objectData->dwData);
                AircraftHandleData aircraftHandleData = simConnectAircraftHandle->toAircraftHandleData();
                aircraftHandleData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getAircraftHandle().upsertLast(aircraftHandleData);
                    dataStored = true;
                } else {
                    skyConnect->d->currentAircraftHandleData = std::move(aircraftHandleData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::Light:
        {
            if (skyConnect->getState() == Connect::State::Recording) {
                auto simConnectLight = reinterpret_cast<const SimConnectLight *>(&objectData->dwData);
                LightData lightData = simConnectLight->toLightData();
                lightData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getLight().upsertLast(lightData);
                    dataStored = true;
                } else {
                    skyConnect->d->currentLightData = std::move(lightData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::FlightPlan:
        {
            if (skyConnect->getState() == Connect::State::Recording) {
                auto simConnectFlightPlan = reinterpret_cast<const SimConnectFlightPlan *>(&objectData->dwData);
                Waypoint waypoint = simConnectFlightPlan->toPreviousWaypoint();
                if (waypoint.isValid()) {
                    if (skyConnect->d->currentLocalDateTime.isValid()) {
                        waypoint.localTime = skyConnect->d->currentLocalDateTime;
                        waypoint.zuluTime = skyConnect->d->currentZuluDateTime;
                    } else {
                        // No simulation time received yet: set flag for pending update
                        skyConnect->d->pendingWaypointTime = true;
                    }
                    const std::int64_t currentTimeStamp = skyConnect->getCurrentTimestamp();
                    waypoint.timestamp = currentTimeStamp;
                    skyConnect->d->flightPlan[waypoint.identifier] = waypoint;
                    waypoint = simConnectFlightPlan->toNextWaypoint();
                    if (waypoint.isValid()) {
                        waypoint.timestamp = currentTimeStamp + 1;
                        skyConnect->d->flightPlan[waypoint.identifier] = waypoint;
                    }
                }
            }
            break;
        }
        case SimConnectType::DataRequest::SimulationTime:
        {
            if (skyConnect->getState() == Connect::State::Recording) {
                auto simConnectSimulationTime = reinterpret_cast<const SimConnectSimulationTime *>(&objectData->dwData);
                skyConnect->d->currentLocalDateTime = simConnectSimulationTime->toLocalDateTime();
                skyConnect->d->currentZuluDateTime = simConnectSimulationTime->toZuluDateTime();
                if (skyConnect->d->pendingWaypointTime) {
                    for (auto it = skyConnect->d->flightPlan.begin(); it != skyConnect->d->flightPlan.end(); ++it)
                    {
                        it.value().localTime = skyConnect->d->currentLocalDateTime;
                        it.value().zuluTime = skyConnect->d->currentLocalDateTime;
                        skyConnect->d->pendingWaypointTime = false;
                    }
                }
            }
            break;
        }
        case SimConnectType::DataRequest::Location:
        {
            auto simConnectLocation = reinterpret_cast<const SimConnectLocation *>(&objectData->dwData);
            emit skyConnect->locationReceived(simConnectLocation->toLocation());
            break;

        }
        case SimConnectType::DataRequest::FlapsHandleIndex:
        {
            if (skyConnect->getState() == Connect::State::Replay || skyConnect->getState() == Connect::State::ReplayPaused) {
                auto flapsHandleIndex = reinterpret_cast<const FlapsHandleIndex *>(&objectData->dwData);
                skyConnect->d->simConnectEvent->setCurrentFlapsHandleIndex(flapsHandleIndex->flapsHandleIndex);
            }
            break;
        }
        default:
            break;
        }
        break;
    }

    case ::SIMCONNECT_RECV_ID_EVENT_FRAME:
    {
        const Connect::State state = skyConnect->getState();
        if (state == Connect::State::Replay) {
            skyConnect->replay();
        } else if (state == Connect::State::Recording) {
            const std::int64_t currentTimeStamp = skyConnect->getCurrentTimestamp();
            if (!skyConnect->sendAircraftData(currentTimeStamp, TimeVariableData::Access::Linear, AircraftSelection::All)) {
                // Connection error
                skyConnect->stopRecording();
            }
        }
        break;
    }

    case ::SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID:
    {
        auto objectData = static_cast<SIMCONNECT_RECV_ASSIGNED_OBJECT_ID *>(receivedData);
        std::int64_t simulationObjectId = objectData->dwObjectID;
        if (skyConnect->d->simConnectAi->registerObjectId(objectData->dwRequestID, simulationObjectId)) {
            ::SimConnect_AIReleaseControl(skyConnect->d->simConnectHandle, simulationObjectId, Enum::underly(SimConnectType::DataRequest::AiReleaseControl));
            skyConnect->d->simConnectEvent->freezeAircraft(objectData->dwObjectID, true);
        } else {
            // No pending request (request has already been removed), so destroy the
            // just generated AI object again
            skyConnect->d->simConnectAi->removeByObjectId(objectData->dwObjectID);
#ifdef DEBUG
            qDebug() << "MSFSSimConnectPlugin::dispatch: SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID: orphaned AI object response for original request:" <<  objectData->dwRequestID
                     << "DESTROYING AI Object again:" << objectData->dwObjectID;
#endif
        }
        break;
    }

    case ::SIMCONNECT_RECV_ID_QUIT:
#ifdef DEBUG
        qDebug() << "MSFSSimConnectPlugin::dispatch: SIMCONNECT_RECV_ID_QUIT";
#endif
        skyConnect->close();
        break;

    case ::SIMCONNECT_RECV_ID_OPEN:
#ifdef DEBUG
        qDebug() << "MSFSSimConnectPlugin::dispatch: SIMCONNECT_RECV_ID_OPEN";
#endif
        break;

    case ::SIMCONNECT_RECV_ID_EXCEPTION:
#ifdef DEBUG
    {
        auto exception = static_cast<SIMCONNECT_RECV_EXCEPTION *>(receivedData);
        qDebug() << "MSFSSimConnectPlugin::dispatch: SIMCONNECT_RECV_ID_EXCEPTION: A server exception" << exception->dwException
                 << "happened: sender ID:" << exception->dwSendID
                 << "index:" << exception->dwIndex
                 << "data:" << cbData;
    }
#endif
        break;
    case ::SIMCONNECT_RECV_ID_NULL:
#ifdef DEBUG
        qDebug() << "MSFSSimConnectPlugin::dispatch: IMCONNECT_RECV_ID_NULL";
#endif
        break;

    default:
        break;
    }

    if (dataStored) {
        if (!skyConnect->isElapsedTimerRunning()) {
            // Start the elapsed timer with the arrival of the first sample data
            skyConnect->setCurrentTimestamp(0);
            skyConnect->resetElapsedTime(true);
        }
    }
}

// PRIVATE SLOTS

void MSFSSimConnectPlugin::processSimConnectEvent() noexcept
{
    updateCurrentTimestamp();
    // Process system events
    ::SimConnect_CallDispatch(d->simConnectHandle, MSFSSimConnectPlugin::dispatch, this);
}
