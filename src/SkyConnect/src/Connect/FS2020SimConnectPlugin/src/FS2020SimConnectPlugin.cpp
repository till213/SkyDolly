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
#include <unordered_map>

#include <windows.h>
#include <SimConnect.h>

#include <QTimer>
#include <QtGlobal>
#include <QApplication>
#include <QWidget>
#include <QDateTime>

#include <tsl/ordered_map.h>

#include "../../../../../Kernel/src/SampleRate.h"
#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/AircraftInfo.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
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
#include "../../../Connect.h"
#include "SimConnectType.h"
#include "SimConnectAircraftInfo.h"
#include "SimConnectPosition.h"
#include "SimConnectEngineReply.h"
#include "SimConnectEngineRequest.h"
#include "SimConnectPrimaryFlightControl.h"
#include "SimConnectSecondaryFlightControl.h"
#include "SimConnectAircraftHandle.h"
#include "SimConnectLight.h"
#include "SimConnectFlightPlan.h"
#include "SimConnectSimulationTime.h"
#include "SimConnectAI.h"
#include "EventWidget.h"
#include "FS2020SimConnectPlugin.h"

namespace
{
    const char *ConnectionName = "SkyConnect";
    constexpr DWORD UserAirplaneRadiusMeters = 0;

    enum class Event: ::SIMCONNECT_CLIENT_EVENT_ID {
        SimStart,
        Pause,
        Crashed,
        Frame,
        FreezeLatituteLongitude,
        FreezeAltitude,
        FreezeAttitude,
        EngineAutoStart,
        EngineAutoShutdown
    };

    enum class EngineState: int {
        Unknown,
        Starting,
        Started,
        Stopped
    };
}

class SkyConnectPrivate
{
public:
    SkyConnectPrivate() noexcept
        : storeDataImmediately(true),
          engineState(EngineState::Unknown),
          pendingWaypointTime(false),
          simConnectHandle(nullptr),
          eventWidget(std::make_unique<EventWidget>()),
          currentRequestPeriod(::SIMCONNECT_PERIOD_NEVER),
          simConnectAI(nullptr)
    {}

    bool storeDataImmediately;
    PositionData currentPositionData;
    EngineData currentEngineData;
    EngineState engineState;
    PrimaryFlightControlData currentPrimaryFlightControlData;
    SecondaryFlightControlData currentSecondaryFlightControlData;
    AircraftHandleData currentAircraftHandleData;
    LightData currentLightData;
    QDateTime currentLocalDateTime;
    QDateTime currentZuluDateTime;
    bool pendingWaypointTime;
    HANDLE simConnectHandle;
    std::unique_ptr<EventWidget> eventWidget;
    ::SIMCONNECT_PERIOD currentRequestPeriod;
    // Insert order is order of flight plan
    tsl::ordered_map<QString, Waypoint> flightPlan;
    std::unique_ptr<SimConnectAI> simConnectAI;
    // Ordered key: request ID - value: AI object ID
    std::unordered_map<::SIMCONNECT_DATA_REQUEST_ID, Aircraft *> pendingAIAircraftCreationRequests;
};

// PUBLIC

FS2020SimConnectPlugin::FS2020SimConnectPlugin(QObject *parent) noexcept
    : AbstractSkyConnect(parent),
      d(std::make_unique<SkyConnectPrivate>())
{
    frenchConnection();
#ifdef DEBUG
    qDebug("FS2020SimConnectPlugin::FS2020SimConnectPlugin: CREATED");
#endif
}

FS2020SimConnectPlugin::~FS2020SimConnectPlugin() noexcept
{
    setAircraftFrozen(::SIMCONNECT_OBJECT_ID_USER, false);
    close();
#ifdef DEBUG
    qDebug("FS2020SimConnectPlugin::~FS2020SimConnectPlugin: DELETED");
#endif
}

bool FS2020SimConnectPlugin::setUserAircraftPosition(const PositionData &positionData) noexcept
{
    SimConnectPosition simConnnectPosition;
    simConnnectPosition.fromPositionData(positionData);
    const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftPositionDefinition),
                                                        ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                        sizeof(SimConnectPosition), &simConnnectPosition);
    return res == S_OK;
}

// PROTECTED

bool FS2020SimConnectPlugin::isTimerBasedRecording(SampleRate::SampleRate sampleRate) const noexcept
{
    // "Auto" and 1 Hz sample rates are processed event-based
    return sampleRate != SampleRate::SampleRate::Auto && sampleRate != SampleRate::SampleRate::Hz1;
}

bool FS2020SimConnectPlugin::onInitialPositionSetup(const InitialPosition &initialPosition) noexcept
{
    HRESULT result;
    SIMCONNECT_DATA_INITPOSITION initialSimConnectPosition = SimConnectPosition::toInitialPosition(initialPosition);
    result = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInitialPosition),
                                             ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(::SIMCONNECT_DATA_INITPOSITION), &initialSimConnectPosition);
    return result == S_OK;
}

bool FS2020SimConnectPlugin::onFreezeUserAircraft(bool enable) noexcept
{
    return setAircraftFrozen(::SIMCONNECT_OBJECT_ID_USER, enable);
}

bool FS2020SimConnectPlugin::onStartRecording() noexcept
{
    resetCurrentSampleData();
    updateRecordingFrequency(Settings::getInstance().getRecordingSampleRate());

    // Initialise flight plan
    d->flightPlan.clear();

    // Get aircraft information
    HRESULT result = ::SimConnect_RequestDataOnSimObjectType(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataRequest::AircraftInfo), Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), ::UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
    bool ok = result == S_OK;

    // Send AI aircraft positions every visual frame
    if (ok) {
        result = ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame), "Frame");
        ok = result == S_OK;
    }
    return ok;
}

void FS2020SimConnectPlugin::onRecordingPaused(bool paused) noexcept
{
    Q_UNUSED(paused)
    updateRecordingFrequency(Settings::getInstance().getRecordingSampleRate());
}

void FS2020SimConnectPlugin::onStopRecording() noexcept
{
    // Stop receiving "frame" events
    ::SimConnect_UnsubscribeFromSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame));

    // Stop receiving aircraft position
    updateRequestPeriod(::SIMCONNECT_PERIOD_NEVER);

    // Update flight plan
    Flight &flight = getCurrentFlight();
    const Aircraft &userAircraft = flight.getUserAircraftConst();
    FlightPlan &flightPlan = userAircraft.getFlightPlan();
    for (const auto &it : d->flightPlan) {
        flightPlan.add(it.second);
    }

    // Update timestamp and simulation time of last waypoint
    int waypointCount = flightPlan.count();
    if (waypointCount > 1) {
        Waypoint waypoint = flightPlan[waypointCount - 1];
        waypoint.localTime = d->currentLocalDateTime;
        waypoint.zuluTime = d->currentZuluDateTime;
        waypoint.timestamp = getCurrentTimestamp();
        flightPlan.update(waypointCount - 1, waypoint);
    } else if (waypointCount == 0 && userAircraft.getPositionConst().count() > 0) {
        Waypoint departureWaypoint;
        PositionData position = userAircraft.getPositionConst().getFirst();
        departureWaypoint.identifier = Waypoint::CustomDepartureIdentifier;
        departureWaypoint.latitude = position.latitude;
        departureWaypoint.longitude = position.longitude;
        departureWaypoint.altitude = position.altitude;
        departureWaypoint.localTime = flight.getFlightConditionConst().startLocalTime;
        departureWaypoint.zuluTime = flight.getFlightConditionConst().startZuluTime;
        departureWaypoint.timestamp = 0;
        flightPlan.add(departureWaypoint);

        Waypoint arrivalWaypoint;
        position = userAircraft.getPositionConst().getLast();
        arrivalWaypoint.identifier = Waypoint::CustomArrivalIdentifier;
        arrivalWaypoint.latitude = position.latitude;
        arrivalWaypoint.longitude = position.longitude;
        arrivalWaypoint.altitude = position.altitude;
        arrivalWaypoint.localTime = d->currentLocalDateTime;
        arrivalWaypoint.zuluTime = d->currentZuluDateTime;
        arrivalWaypoint.timestamp = qMax(getCurrentTimestamp(), departureWaypoint.timestamp + 1);
        flightPlan.add(arrivalWaypoint);
    }

    // Update end simulation time of flight conditions
    FlightCondition condition = flight.getFlightConditionConst();
    condition.endLocalTime = d->currentLocalDateTime;
    condition.endZuluTime = d->currentZuluDateTime;
    flight.setFlightCondition(condition);
}

bool FS2020SimConnectPlugin::onStartReplay(qint64 currentTimestamp) noexcept
{
    d->engineState = EngineState::Unknown;

    // Send aircraft position every visual frame
    HRESULT result = ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame), "Frame");
    return result == S_OK;
}

void FS2020SimConnectPlugin::onReplayPaused(bool paused) noexcept
{
    if (paused) {
        ::SimConnect_UnsubscribeFromSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame));
    } else {
        ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame), "Frame");
    }
}

void FS2020SimConnectPlugin::onStopReplay() noexcept
{
    ::SimConnect_UnsubscribeFromSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame));
}

void FS2020SimConnectPlugin::onSeek(qint64 currentTimestamp) noexcept
{
    d->engineState = EngineState::Unknown;
};

void FS2020SimConnectPlugin::onRecordingSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept
{
     updateRecordingFrequency(sampleRate);
}

bool FS2020SimConnectPlugin::sendAircraftData(qint64 currentTimestamp, TimeVariableData::Access access, AircraftSelection aircraftSelection) noexcept
{
    const Flight &flight = getCurrentFlight();
    const Aircraft &userAircraft = flight.getUserAircraftConst();
    bool ok = true;
    for (auto &aircraft : flight) {

        // Replay AI aircrafts - if any - during recording (if all aircrafts are selected for replay)
        const bool isUserAircraft = *aircraft == userAircraft;
        if (isUserAircraft && getReplayMode() == ReplayMode::UserAircraftManualControl) {
            // The user aircraft is manually flown
            continue;
        }

        if (!isUserAircraft && aircraftSelection == AircraftSelection::UserAircraft) {
            // Only the user aircraft is to be sent
            continue;
        }

        if (getState() != Connect::State::Recording || !isUserAircraft) {

            const qint64 objectId = aircraft->getSimulationObjectId();
            if (objectId != Aircraft::InvalidSimulationId && objectId != Aircraft::PendingSimulationId) {

                ok = true;
                const PositionData &positionData = aircraft->getPositionConst().interpolate(currentTimestamp, access);
                if (!positionData.isNull()) {
                    SimConnectPosition simConnnectPosition;
                    simConnnectPosition.fromPositionData(positionData);
                    const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftPositionDefinition),
                                                                        objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                        sizeof(SimConnectPosition), &simConnnectPosition);
                    ok = res == S_OK;
                }

                // Engine
                if (ok) {
                    const EngineData &engineData = aircraft->getEngineConst().interpolate(currentTimestamp, access);
                    if (!engineData.isNull()) {
                        SimConnectEngineRequest simConnectEngineRequest;
                        simConnectEngineRequest.fromEngineData(engineData);
                        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftEngineRequestDefinition),
                                                                            objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                            sizeof(SimConnectEngineRequest), &simConnectEngineRequest);
                        ok = res == S_OK;
                        if (ok) {
                            ok = updateAndSendEngineStartEvent(objectId, engineData, access);
                        }
                    }
                }

                // Primary flight controls
                if (ok) {
                    const PrimaryFlightControlData &primaryFlightControlData = aircraft->getPrimaryFlightControlConst().interpolate(currentTimestamp, access);
                    if (!primaryFlightControlData.isNull()) {
                        SimConnectPrimaryFlightControl simConnectPrimaryFlightControl;
                        simConnectPrimaryFlightControl.fromPrimaryFlightControlData(primaryFlightControlData);
                        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftPrimaryFlightControlDefinition),
                                                                            objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                            sizeof(SimConnectPrimaryFlightControl), &simConnectPrimaryFlightControl);
                        ok = res == S_OK;
                    }
                }

                // Secondary flight controls
                if (ok) {
                    const SecondaryFlightControlData &secondaryFlightControlData = aircraft->getSecondaryFlightControlConst().interpolate(currentTimestamp, access);
                    if (!secondaryFlightControlData.isNull()) {
                        SimConnectSecondaryFlightControl simConnectSecondaryFlightControl;
                        simConnectSecondaryFlightControl.fromSecondaryFlightControlData(secondaryFlightControlData);
                        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftSecondaryFlightControlDefinition),
                                                                            objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                            sizeof(SimConnectSecondaryFlightControl), &simConnectSecondaryFlightControl);
                        ok = res == S_OK;
                    }
                }

                // Aircraft handles & brakes
                if (ok) {
                    const AircraftHandleData &aircraftHandleData = aircraft->getAircraftHandleConst().interpolate(currentTimestamp, access);
                    if (!aircraftHandleData.isNull()) {
                        SimConnectAircraftHandle simConnectAircraftHandle;
                        simConnectAircraftHandle.fromAircraftHandleData(aircraftHandleData);
                        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftHandleDefinition),
                                                                            objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                            sizeof(SimConnectAircraftHandle), &simConnectAircraftHandle);
                        ok = res == S_OK;
                    }
                }

                // Lights
                if (ok) {
                    const LightData &lightData = aircraft->getLightConst().interpolate(currentTimestamp, access);
                    if (!lightData.isNull()) {
                        SimConnectLight simConnectLight;
                        simConnectLight.fromLightData(lightData);
                        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftLightDefinition ),
                                                                            objectId, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                            sizeof(SimConnectLight), &simConnectLight);
                        ok = res == S_OK;
                    }
                }

            } // Valid simulation object ID

        } // User aircraft not sent during recording

    } // All aircrafts

    // Start the elapsed timer after sending the first sample data, but
    // only when not recording (the first received sample will start the timer then)
    if (!isElapsedTimerRunning() && access != TimeVariableData::Access::Seek && getState() != Connect::State::Recording) {
        startElapsedTimer();
    }
    return ok;
}

inline bool FS2020SimConnectPlugin::updateAndSendEngineStartEvent(qint64 objectId, const EngineData &engineData, TimeVariableData::Access access) noexcept
{
    HRESULT res = S_OK;

    if (access == TimeVariableData::Access::Seek) {
        d->engineState = EngineState::Unknown;
    }

    switch (d->engineState) {
    case EngineState::Starting:
        if (engineData.hasCombustion()) {
            d->engineState = EngineState::Started;
#ifdef DEBUG
            qDebug("SkyConnectImpl::updateAndSendEngineState: aircraft ID: %lld: STARTING -> ENGINE STARTED", objectId);
#endif
        } else if (!engineData.hasEngineStarterEnabled()) {
             // STARTING: Engine started disabled, no combustion -> STOPPED
            res = ::SimConnect_TransmitClientEvent(d->simConnectHandle, objectId, Enum::toUnderlyingType(Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            d->engineState = EngineState::Stopped;
#ifdef DEBUG
            qDebug("SkyConnectImpl::updateAndSendEngineState: aircraft ID: %lld STARTING -> ENGINE STOPPED", objectId);
#endif
        }
        break;
    case EngineState::Started:
        if (!engineData.hasCombustion()) {
            // STARTED: No combustion -> STOPPED
            res = ::SimConnect_TransmitClientEvent(d->simConnectHandle, objectId, Enum::toUnderlyingType(Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            d->engineState = EngineState::Stopped;
#ifdef DEBUG
            qDebug("SkyConnectImpl::updateAndSendEngineState: aircraft ID: %lld STARTED -> ENGINE STOPPED", objectId);
#endif
        }
        break;
    case EngineState::Stopped:
        // Either general engine starter has been enabled or combustion has started -> engine start
        // Note: apparently the engine starter can be disabled (false) and yet with an active combustion (= running engine)
        //       specifically in the case when the aircraft has been "auto-started" (CTRL + E)
        if (engineData.hasEngineStarterEnabled() || engineData.hasCombustion()) {
            res = ::SimConnect_TransmitClientEvent(d->simConnectHandle, objectId, Enum::toUnderlyingType(Event::EngineAutoStart), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            d->engineState = EngineState::Starting;
#ifdef DEBUG
            qDebug("SkyConnectImpl::updateAndSendEngineState: aircraft ID: %lld STOPPED -> ENGINE STARTING", objectId);
#endif
        }
        break;
    default:
        // Unknown
        if (engineData.hasEngineStarterEnabled() || engineData.hasCombustion()) {
            res = ::SimConnect_TransmitClientEvent(d->simConnectHandle, objectId, Enum::toUnderlyingType(Event::EngineAutoStart), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            d->engineState = engineData.hasCombustion() ? EngineState::Started : EngineState::Starting;
        } else {
            res = ::SimConnect_TransmitClientEvent(d->simConnectHandle, objectId, Enum::toUnderlyingType(Event::EngineAutoShutdown), 0, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
            d->engineState = EngineState::Stopped;
        }
#ifdef DEBUG
        QString newEngineStateName;
        switch (d->engineState) {
        case EngineState::Starting:
            newEngineStateName = "Starting";
            break;
        case EngineState::Started:
            newEngineStateName = "Started";
            break;
        case EngineState::Stopped:
            newEngineStateName = "Stopped";
            break;
        default:
            newEngineStateName = "Unknown";
            break;
        }
        qDebug("SkyConnectImpl::updateAndSendEngineState: aircraft ID: %lld STATE UNKNOWN -> NEW ENGINE STATE: %s", objectId, qPrintable(newEngineStateName));
#endif
        break;
    }

    return res == S_OK;
}

bool FS2020SimConnectPlugin::isConnectedWithSim() const noexcept
{
    return d->simConnectHandle != nullptr;
}

bool FS2020SimConnectPlugin::connectWithSim() noexcept
{
    HWND hWnd = reinterpret_cast<HWND>(d->eventWidget->winId());
    DWORD userEvent = EventWidget::SimConnnectUserMessage;
    HRESULT result = ::SimConnect_Open(&(d->simConnectHandle), ::ConnectionName, hWnd, userEvent, nullptr, ::SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
    if (result == S_OK) {
        d->simConnectAI = std::make_unique<SimConnectAI>(d->simConnectHandle);
        setupRequestData();
    }
#ifdef DEBUG
    qDebug("SkyConnectImpl::connectWithSim: CONNECT with SIM, handle: %p success: %d", d->simConnectHandle, result == S_OK);
#endif
    const bool ok = result == S_OK;
    if (ok) {
        createAIObjects();
    }
    return ok;
}

bool FS2020SimConnectPlugin::onCreateAIObjects() noexcept
{
    // When "fly with formation" is enabled we also create an AI aircraft for the user aircraft
    // (the user aircraft of the recorded aircrafts in the formation, that is)
    const bool includingUserAircraft = getReplayMode() == ReplayMode::FlyWithFormation;
    return d->simConnectAI->createSimulatedAircrafts(getCurrentFlight(), getCurrentTimestamp(), includingUserAircraft, d->pendingAIAircraftCreationRequests);
}

void FS2020SimConnectPlugin::onDestroyAIObjects() noexcept
{
    d->pendingAIAircraftCreationRequests.clear();
    d->simConnectAI->destroySimulatedAircrafts(getCurrentFlight());
}

void FS2020SimConnectPlugin::onDestroyAIObject(Aircraft &aircraft) noexcept
{
    if (aircraft.getSimulationObjectId() != ::SIMCONNECT_OBJECT_ID_USER) {
        d->simConnectAI->destroySimulatedAircraft(aircraft);
    }
}

// PROTECTED SLOTS

void FS2020SimConnectPlugin::recordData() noexcept
{
    Aircraft &userAircraft = getCurrentFlight().getUserAircraft();
    bool dataStored = false;
    if (!d->currentPositionData.isNull()) {
        userAircraft.getPosition().upsert(std::move(d->currentPositionData));
        // Processed
        dataStored = true;
        d->currentPositionData = PositionData::NullData;
    }
    if (!d->currentEngineData.isNull()) {
        userAircraft.getEngine().upsert(std::move(d->currentEngineData));
        // Processed
        dataStored = true;
        d->currentEngineData = EngineData::NullData;
    }
    if (!d->currentPrimaryFlightControlData.isNull()) {
        userAircraft.getPrimaryFlightControl().upsert(std::move(d->currentPrimaryFlightControlData));
        // Processed
        dataStored = true;
        d->currentPrimaryFlightControlData = PrimaryFlightControlData::NullData;
    }
    if (!d->currentSecondaryFlightControlData.isNull()) {
        userAircraft.getSecondaryFlightControl().upsert(std::move(d->currentSecondaryFlightControlData));
        // Processed
        dataStored = true;
        d->currentSecondaryFlightControlData = SecondaryFlightControlData::NullData;
    }
    if (!d->currentAircraftHandleData.isNull()) {
        userAircraft.getAircraftHandle().upsert(std::move(d->currentAircraftHandleData));
        // Processed
        dataStored = true;
        d->currentAircraftHandleData = AircraftHandleData::NullData;
    }
    if (!d->currentLightData.isNull()) {
        userAircraft.getLight().upsert(std::move(d->currentLightData));
        // Processed
        dataStored = true;
        d->currentLightData = LightData::NullData;
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

void FS2020SimConnectPlugin::frenchConnection() noexcept
{
    connect(d->eventWidget.get(), &EventWidget::simConnectEvent,
            this, &FS2020SimConnectPlugin::processSimConnectEvent);
}

void FS2020SimConnectPlugin::resetCurrentSampleData() noexcept
{
    d->currentPositionData = PositionData::NullData;
    d->currentEngineData = EngineData::NullData;
    d->currentPrimaryFlightControlData = PrimaryFlightControlData::NullData;
    d->currentSecondaryFlightControlData = SecondaryFlightControlData::NullData;
    d->currentAircraftHandleData = AircraftHandleData::NullData;
    d->currentLightData = LightData::NullData;
}

bool FS2020SimConnectPlugin::reconnectWithSim() noexcept
{
    bool res;
    if (close()) {
        res = connectWithSim();
    } else {
        res = false;
    }
    return res;
}

bool FS2020SimConnectPlugin::close() noexcept
{
    HRESULT result;

    if (d->simConnectHandle != nullptr) {
        result = ::SimConnect_Close(d->simConnectHandle);
        d->simConnectHandle = nullptr;
    } else {
        result = S_OK;
    }
    return result == S_OK;
}

void FS2020SimConnectPlugin::setupRequestData() noexcept
{
    // Request data
    SimConnectAircraftInfo::addToDataDefinition(d->simConnectHandle);
    SimConnectPosition::addToDataDefinition(d->simConnectHandle);
    SimConnectEngineReply::addToDataDefinition(d->simConnectHandle);
    SimConnectEngineRequest::addToDataDefinition(d->simConnectHandle);
    SimConnectPrimaryFlightControl::addToDataDefinition(d->simConnectHandle);
    SimConnectSecondaryFlightControl::addToDataDefinition(d->simConnectHandle);
    SimConnectAircraftHandle::addToDataDefinition(d->simConnectHandle);
    SimConnectLight::addToDataDefinition(d->simConnectHandle);
    SimConnectFlightPlan::addToDataDefinition(d->simConnectHandle);
    SimConnectSimulationTime::addToDataDefinition(d->simConnectHandle);

    ::SimConnect_AddToDataDefinition(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInitialPosition), "Initial Position", nullptr, ::SIMCONNECT_DATATYPE_INITPOSITION);

    // System event subscription
    ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::SimStart), "SimStart");
    ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Pause), "Pause");
    ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Crashed), "Crashed");

    // Client events
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::FreezeLatituteLongitude), "FREEZE_LATITUDE_LONGITUDE_SET");
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::FreezeAltitude), "FREEZE_ALTITUDE_SET");
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::FreezeAttitude), "FREEZE_ATTITUDE_SET");

    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::EngineAutoStart), "ENGINE_AUTO_START");
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::EngineAutoShutdown), "ENGINE_AUTO_SHUTDOWN");
}

bool FS2020SimConnectPlugin::setAircraftFrozen(::SIMCONNECT_OBJECT_ID objectId, bool enable) noexcept
{
    const DWORD data = enable ? 1 : 0;
    HRESULT result = ::SimConnect_TransmitClientEvent(d->simConnectHandle, objectId, Enum::toUnderlyingType(Event::FreezeLatituteLongitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    if (result == S_OK) {
        result = ::SimConnect_TransmitClientEvent(d->simConnectHandle, objectId, Enum::toUnderlyingType(Event::FreezeAltitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    }
    if (result == S_OK) {
        result = ::SimConnect_TransmitClientEvent(d->simConnectHandle, objectId, Enum::toUnderlyingType(Event::FreezeAttitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    }
    return result == S_OK;
}

void FS2020SimConnectPlugin::replay() noexcept
{
    const qint64 currentTimestamp = getCurrentTimestamp();
    if (currentTimestamp <= getCurrentFlight().getTotalDurationMSec()) {
        if (!sendAircraftData(currentTimestamp, TimeVariableData::Access::Linear, AircraftSelection::All)) {
            // Connection error
            stopReplay();
        }
    } else {
        // At end of replay
        stopReplay();
    }
}

void FS2020SimConnectPlugin::updateRecordingFrequency(SampleRate::SampleRate sampleRate) noexcept
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

void FS2020SimConnectPlugin::updateRequestPeriod(::SIMCONNECT_PERIOD period) noexcept
{
    if (d->currentRequestPeriod != period) {
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataRequest::AircraftPosition), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftPositionDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataRequest::Engine), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftEngineReplyDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataRequest::PrimaryFlightControl), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftPrimaryFlightControlDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataRequest::SecondaryFlightControl), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftSecondaryFlightControlDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataRequest::AircraftHandle), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftHandleDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataRequest::Light), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftLightDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        // Update the flight plan and simulation time only every second
        ::SIMCONNECT_PERIOD oneSecondPeriod = period != ::SIMCONNECT_PERIOD_NEVER ? ::SIMCONNECT_PERIOD_SECOND : ::SIMCONNECT_PERIOD_NEVER;
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataRequest::FlightPlan), Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightPlanDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, oneSecondPeriod, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataRequest::SimulationTime), Enum::toUnderlyingType(SimConnectType::DataDefinition::SimulationTimeDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, oneSecondPeriod, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        d->currentRequestPeriod = period;
    }
}

void CALLBACK FS2020SimConnectPlugin::dispatch(::SIMCONNECT_RECV *receivedData, DWORD cbData, void *context) noexcept
{
    Q_UNUSED(cbData);

    FS2020SimConnectPlugin *skyConnect = static_cast<FS2020SimConnectPlugin *>(context);
    Flight &flight = skyConnect->getCurrentFlight();
    Aircraft &userAircraft = flight.getUserAircraft();
    ::SIMCONNECT_RECV_SIMOBJECT_DATA *objectData;

    bool dataStored = false;
    switch (receivedData->dwID) {
    case ::SIMCONNECT_RECV_ID_EVENT:
    {
        const ::SIMCONNECT_RECV_EVENT *evt = reinterpret_cast<::SIMCONNECT_RECV_EVENT *>(receivedData);
        switch (static_cast<Event>(evt->uEventID)) {
        case Event::SimStart:
#ifdef DEBUG
            qDebug("SIMCONNECT_RECV_ID_EVENT: SIMSTART event");
#endif
            break;

        case Event::Pause:
#ifdef DEBUG
            qDebug("SIMCONNECT_RECV_ID_EVENT: PAUSE event: %lu", evt->dwData);
#endif
            // It seems that the pause event is currently only triggered by selecting "Pause Simulation"
            // in the developer mode (FS 2020), but neither when "active pause" is selected nor when ESC
            // (in-game meu") is entered; also, we ignore the first "unpause" event (which is always
            // sent by FS 2020 after the initial connect), as we explicitly pause the replay after having
            // loaded a flight: we simply do this by assuming that no "unpause" would normally be sent
            // at the very beginning (timestamp 0) of the replay
            if (evt->dwData > 0 || skyConnect->getCurrentTimestamp() > 0) {
                skyConnect->setPaused(evt->dwData == 1);
            }
            break;

        case Event::Crashed:
#ifdef DEBUG
            qDebug("SIMCONNECT_RECV_ID_EVENT: CRASHED event");
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
        objectData = reinterpret_cast<::SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *>(receivedData);
        switch (static_cast<SimConnectType::DataRequest>(objectData->dwRequestID)) {
        case SimConnectType::DataRequest::AircraftInfo:
        {
            const SimConnectAircraftInfo *simConnectAircraftInfo = reinterpret_cast<const SimConnectAircraftInfo *>(&objectData->dwData);
            AircraftInfo aircraftInfo = simConnectAircraftInfo->toAircraftInfo();
            aircraftInfo.startDate = QDateTime::currentDateTime();
            userAircraft.setAircraftInfo(aircraftInfo);
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
        objectData = reinterpret_cast<::SIMCONNECT_RECV_SIMOBJECT_DATA *>(receivedData);

        const bool storeDataImmediately = skyConnect->d->storeDataImmediately;
        switch (static_cast<SimConnectType::DataRequest>(objectData->dwRequestID)) {
        case SimConnectType::DataRequest::AircraftPosition:
        {
            const SimConnectPosition *simConnectPosition;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectPosition = reinterpret_cast<const SimConnectPosition *>(&objectData->dwData);
                PositionData positionData = simConnectPosition->toPositionData();
                positionData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getPosition().upsert(std::move(positionData));
                    dataStored = true;
                } else {
                    skyConnect->d->currentPositionData = std::move(positionData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::Engine:
        {
            const SimConnectEngineReply *simConnectEngineReply;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectEngineReply = reinterpret_cast<const SimConnectEngineReply *>(&objectData->dwData);
                EngineData engineData = simConnectEngineReply->toEngineData();
                engineData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getEngine().upsert(std::move(engineData));
                    dataStored = true;
                } else {
                    skyConnect->d->currentEngineData = std::move(engineData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::PrimaryFlightControl:
        {
            const SimConnectPrimaryFlightControl *simConnectPrimaryFlightControl;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectPrimaryFlightControl = reinterpret_cast<const SimConnectPrimaryFlightControl *>(&objectData->dwData);
                PrimaryFlightControlData primaryFlightControlData = simConnectPrimaryFlightControl->toPrimaryFlightControlData();
                primaryFlightControlData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getPrimaryFlightControl().upsert(std::move(primaryFlightControlData));
                    dataStored = true;
                } else {
                    skyConnect->d->currentPrimaryFlightControlData = std::move(primaryFlightControlData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::SecondaryFlightControl:
        {
            const SimConnectSecondaryFlightControl *simConnectSecondaryFlightControl;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectSecondaryFlightControl = reinterpret_cast<const SimConnectSecondaryFlightControl *>(&objectData->dwData);
                SecondaryFlightControlData secondaryFlightControlData = simConnectSecondaryFlightControl->toSecondaryFlightControlData();
                secondaryFlightControlData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getSecondaryFlightControl().upsert(std::move(secondaryFlightControlData));
                    dataStored = true;
                } else {
                    skyConnect->d->currentSecondaryFlightControlData = std::move(secondaryFlightControlData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::AircraftHandle:
        {
            const SimConnectAircraftHandle *simConnectAircraftHandle;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectAircraftHandle = reinterpret_cast<const SimConnectAircraftHandle *>(&objectData->dwData);
                AircraftHandleData aircraftHandleData = simConnectAircraftHandle->toAircraftHandleData();
                aircraftHandleData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getAircraftHandle().upsert(std::move(aircraftHandleData));
                    dataStored = true;
                } else {
                    skyConnect->d->currentAircraftHandleData = std::move(aircraftHandleData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::Light:
        {
            const SimConnectLight *simConnectLight;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectLight = reinterpret_cast<const SimConnectLight *>(&objectData->dwData);
                LightData lightData = simConnectLight->toLightData();
                lightData.timestamp = skyConnect->getCurrentTimestamp();
                if (storeDataImmediately) {
                    userAircraft.getLight().upsert(std::move(lightData));
                    dataStored = true;
                } else {
                    skyConnect->d->currentLightData = std::move(lightData);
                }
            }
            break;
        }
        case SimConnectType::DataRequest::FlightPlan:
        {
            const SimConnectFlightPlan *simConnectFlightPlan;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectFlightPlan = reinterpret_cast<const SimConnectFlightPlan *>(&objectData->dwData);
                Waypoint waypoint = simConnectFlightPlan->toPreviousWaypoint();
                if (waypoint.isValid()) {
                    if (skyConnect->d->currentLocalDateTime.isValid()) {
                        waypoint.localTime = skyConnect->d->currentLocalDateTime;
                        waypoint.zuluTime = skyConnect->d->currentZuluDateTime;
                    } else {
                        // No simulation time received yet: set flag for pending update
                        skyConnect->d->pendingWaypointTime = true;
                    }
                    const qint64 currentTimeStamp = skyConnect->getCurrentTimestamp();
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
            const SimConnectSimulationTime *simConnectSimulationTime;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectSimulationTime = reinterpret_cast<const SimConnectSimulationTime *>(&objectData->dwData);
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
            const qint64 currentTimeStamp = skyConnect->getCurrentTimestamp();
            if (!skyConnect->sendAircraftData(currentTimeStamp, TimeVariableData::Access::Linear, AircraftSelection::All)) {
                // Connection error
                skyConnect->stopRecording();
            }
        }
        break;
    }

    case ::SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID:
    {
        SIMCONNECT_RECV_ASSIGNED_OBJECT_ID *objectData = (SIMCONNECT_RECV_ASSIGNED_OBJECT_ID*)receivedData;
        auto it = skyConnect->d->pendingAIAircraftCreationRequests.extract(objectData->dwRequestID);
        if (!it.empty()) {
            Aircraft *aircraft = it.mapped();
            aircraft->setSimulationObjectId(objectData->dwObjectID);
#ifdef DEBUG
            qDebug("SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID: Request ID: %lu, asssigned object ID: %lu, aircraft ID: %lld, remaining pending requests: %lld",
                   objectData->dwRequestID, objectData->dwObjectID, aircraft->getId(), skyConnect->d->pendingAIAircraftCreationRequests.size());
#endif
            ::SimConnect_AIReleaseControl(skyConnect->d->simConnectHandle, aircraft->getSimulationObjectId(), Enum::toUnderlyingType(SimConnectType::DataRequest::AIReleaseControl));
            skyConnect->setAircraftFrozen(objectData->dwObjectID, true);
        } else {
            // No pending request (request has already been removed), so destroy the
            // just generated AI object again
            skyConnect->d->simConnectAI->destroySimulatedObject(objectData->dwObjectID);
#ifdef DEBUG
            qDebug("SIMCONNECT_RECV_ID_ASSIGNED_OBJECT_ID: orphaned AI object response for original request %lu, DESTROYING AI Object again: %lu", objectData->dwRequestID, objectData->dwObjectID);
#endif
        }
        break;
    }

    case ::SIMCONNECT_RECV_ID_QUIT:
#ifdef DEBUG
        qDebug("SIMCONNECT_RECV_ID_QUIT");
#endif
        skyConnect->close();
        break;

    case ::SIMCONNECT_RECV_ID_OPEN:
#ifdef DEBUG
        qDebug("SIMCONNECT_RECV_ID_OPEN");
#endif
        break;

    case ::SIMCONNECT_RECV_ID_EXCEPTION:
#ifdef DEBUG
    {
        SIMCONNECT_RECV_EXCEPTION *exception = static_cast<SIMCONNECT_RECV_EXCEPTION *>(receivedData);
        qDebug("SIMCONNECT_RECV_ID_EXCEPTION: A server exception %lu happened: sender ID: %lu index: %lu data: %lu",
               exception->dwException, exception->dwSendID, exception->dwIndex, cbData);
    }
#endif
        break;
    case ::SIMCONNECT_RECV_ID_NULL:
#ifdef DEBUG
        qDebug("SIMCONNECT_RECV_ID_NULL");
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

void FS2020SimConnectPlugin::processSimConnectEvent() noexcept
{
    updateCurrentTimestamp();
    // Process system events
    ::SimConnect_CallDispatch(d->simConnectHandle, FS2020SimConnectPlugin::dispatch, this);
}
