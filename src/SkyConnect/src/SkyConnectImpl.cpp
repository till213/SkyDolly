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
#include <unordered_map>

#include <windows.h>
#include <SimConnect.h>

#include <QTimer>
#include <QtGlobal>
#include <QApplication>
#include <QWidget>
#include <QDateTime>

#include <tsl/ordered_map.h>

#include "../../Kernel/src/SampleRate.h"
#include "../../Kernel/src/Enum.h"
#include "../../Kernel/src/Settings.h"
#include "../../Model/src/Flight.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/AircraftInfo.h"
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
#include "SimConnectType.h"
#include "SimConnectAircraftInfo.h"
#include "SimConnectPosition.h"
#include "SimConnectEngine.h"
#include "SimConnectPrimaryFlightControl.h"
#include "SimConnectSecondaryFlightControl.h"
#include "SimConnectAircraftHandle.h"
#include "SimConnectLight.h"
#include "SimConnectFlightPlan.h"
#include "SimConnectSimulationTime.h"
#include "Connect.h"
#include "EventWidget.h"
#include "SkyConnectImpl.h"

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
        FreezeAttitude
    };

    enum class DataRequest: ::SIMCONNECT_DATA_REQUEST_ID {
        AircraftInfo,
        FlightPlan,
        SimulationTime,
        AircraftPosition,
        Engine,
        PrimaryFlightControl,
        SecondaryFlightControl,
        AircraftHandle,
        Light
    };
}

class SkyConnectPrivate
{
public:
    SkyConnectPrivate() noexcept
        : pendingWaypointTime(false),
          simConnectHandle(nullptr),
          frozen(false),
          eventWidget(nullptr),
          currentRequestPeriod(::SIMCONNECT_PERIOD_NEVER)
    {}

    QDateTime currentLocalDateTime;
    QDateTime currentZuluDateTime;
    bool pendingWaypointTime;
    HANDLE simConnectHandle;
    bool frozen;
    std::unique_ptr<EventWidget> eventWidget;
    ::SIMCONNECT_PERIOD currentRequestPeriod;
    // Insert order is order of flight plan
    tsl::ordered_map<QString, Waypoint> flightPlan;
};

// PUBLIC

SkyConnectImpl::SkyConnectImpl(QObject *parent) noexcept
    : AbstractSkyConnect(parent),
      d(std::make_unique<SkyConnectPrivate>())
{
}

SkyConnectImpl::~SkyConnectImpl() noexcept
{
    setSimulationFrozen(false);
    close();
}

// PROTECTED

bool SkyConnectImpl::onStartRecording() noexcept
{
    updateRecordFrequency(Settings::getInstance().getRecordSampleRate());
    // Initialise flight plan
    d->flightPlan.clear();
    // Get aircraft information
    HRESULT result = ::SimConnect_RequestDataOnSimObjectType(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::AircraftInfo), Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightInformationDefinition), ::UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
    return result == S_OK;
}

void SkyConnectImpl::onRecordingPaused(bool paused) noexcept
{
    Q_UNUSED(paused)
    updateRecordFrequency(Settings::getInstance().getRecordSampleRate());
}

void SkyConnectImpl::onStopRecording() noexcept
{
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
    int waypointCount = flightPlan.getAllConst().count();
    if (waypointCount > 1) {
        Waypoint waypoint = flightPlan.getAllConst().at(waypointCount - 1);
        waypoint.localTime = d->currentLocalDateTime;
        waypoint.zuluTime = d->currentZuluDateTime;
        waypoint.timestamp = getCurrentTimestamp();
        flightPlan.update(waypointCount - 1, waypoint);
    } else if (waypointCount == 0) {
        Waypoint departureWaypoint;
        PositionData position = userAircraft.getPositionConst().getAllConst().at(0);
        departureWaypoint.identifier = "CUSTD";
        departureWaypoint.latitude = position.latitude;
        departureWaypoint.longitude = position.longitude;
        departureWaypoint.altitude = position.altitude;
        departureWaypoint.localTime = flight.getFlightConditionConst().startLocalTime;
        departureWaypoint.zuluTime = flight.getFlightConditionConst().startZuluTime;
        departureWaypoint.timestamp = 0;
        flightPlan.add(departureWaypoint);

        Waypoint arrivalWaypoint;
        position = userAircraft.getPositionConst().getLast();
        arrivalWaypoint.identifier = "CUSTA";
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

bool SkyConnectImpl::onStartReplay(qint64 currentTimestamp) noexcept
{
    if (d->eventWidget == nullptr) {
        d->eventWidget = std::make_unique<EventWidget>();
        connect(d->eventWidget.get(), &EventWidget::simConnectEvent,
                this, &SkyConnectImpl::processEvents);
        reconnectWithSim();
    }

    // "Freeze" the simulation: position and attitude only set by (interpolated) sample points
    setSimulationFrozen(true);    
    if (currentTimestamp == 0) {
        setupInitialPosition();
#ifdef DEBUG
    qDebug("SkyConnectImpl::onStartReplay: current timestamp is 0, SETUP initial position");
#endif
    }

    // Send aircraft position every visual frame
    HRESULT result = ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame), "Frame");
    return result == S_OK;
}

void SkyConnectImpl::onReplayPaused(bool paused) noexcept
{
    if (paused) {
        ::SimConnect_UnsubscribeFromSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame));
    } else {
        ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame), "Frame");
    }
}

void SkyConnectImpl::onStopReplay() noexcept
{
    ::SimConnect_UnsubscribeFromSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Frame));
    setSimulationFrozen(false);
}

void SkyConnectImpl::onSeek(qint64 currentTimestamp) noexcept
{
    if (currentTimestamp == 0) {
        setupInitialPosition();
#ifdef DEBUG
    qDebug("SkyConnectImpl::onSeek: current timestamp is 0, SETUP initial position");
#endif
    }
};

void SkyConnectImpl::onRecordSampleRateChanged(SampleRate::SampleRate sampleRate) noexcept
{
     updateRecordFrequency(sampleRate);
}

bool SkyConnectImpl::sendAircraftData(qint64 currentTimestamp, TimeVariableData::Access access) noexcept
{
    bool success;
    const Aircraft &userAircraft = getCurrentFlight().getUserAircraftConst();

    success = true;
    const PositionData &currentPositionData = userAircraft.getPositionConst().interpolate(currentTimestamp, access);
    if (!currentPositionData.isNull()) {
        SimConnectPosition simConnnectPosition;
        simConnnectPosition.fromPositionData(currentPositionData);
        const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftPositionDefinition),
                                                            ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                            sizeof(SimConnectPosition), &simConnnectPosition);
        success = res == S_OK;
    }

    // Engine
    if (success) {
        const EngineData &engineData = userAircraft.getEngineConst().interpolate(currentTimestamp, access);
        if (!engineData.isNull()) {
            SimConnectEngine simConnectEngine;
            simConnectEngine.fromEngineData(engineData);
            const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftEngineDefinition),
                                                                ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                sizeof(SimConnectEngine), &simConnectEngine);
            success = res == S_OK;
        }
    }

    // Primary flight controls
    if (success) {
        const PrimaryFlightControlData &primaryFlightControlData = userAircraft.getPrimaryFlightControlConst().interpolate(currentTimestamp, access);
        if (!primaryFlightControlData.isNull()) {
            SimConnectPrimaryFlightControl simConnectPrimaryFlightControl;
            simConnectPrimaryFlightControl.fromPrimaryFlightControlData(primaryFlightControlData);
            const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftPrimaryFlightControlDefinition),
                                                                ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                sizeof(SimConnectPrimaryFlightControl), &simConnectPrimaryFlightControl);
            success = res == S_OK;
        }
    }

    // Secondary flight controls
    if (success) {
        const SecondaryFlightControlData &secondaryFlightControlData = userAircraft.getSecondaryFlightControlConst().interpolate(currentTimestamp, access);
        if (!secondaryFlightControlData.isNull()) {
            SimConnectSecondaryFlightControl simConnectSecondaryFlightControl;
            simConnectSecondaryFlightControl.fromSecondaryFlightControlData(secondaryFlightControlData);
            const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftSecondaryFlightControlDefinition),
                                                                ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                sizeof(SimConnectSecondaryFlightControl), &simConnectSecondaryFlightControl);
            success = res == S_OK;
        }
    }

    // Aircraft handles & brakes
    if (success) {
        const AircraftHandleData &aircraftHandleData = userAircraft.getAircraftHandleConst().interpolate(currentTimestamp, access);
        if (!aircraftHandleData.isNull()) {
            SimConnectAircraftHandle simConnectAircraftHandle;
            simConnectAircraftHandle.fromAircraftHandleData(aircraftHandleData);
            const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftHandleDefinition),
                                                                ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                sizeof(SimConnectAircraftHandle), &simConnectAircraftHandle);
            success = res == S_OK;
        }
    }

    // Lights
    if (success) {
        const LightData &lightData = userAircraft.getLightConst().interpolate(currentTimestamp, access);
        if (!lightData.isNull()) {
            SimConnectLight simConnectLight;
            simConnectLight.fromLightData(lightData);
            const HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftLightDefinition ),
                                                                ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0,
                                                                sizeof(SimConnectLight), &simConnectLight);
            success = res == S_OK;
        }
    }

    // Start the elapsed timer after sending the first sample data
    if (access != TimeVariableData::Access::Seek && !isElapsedTimerRunning()) {
        startElapsedTimer();
    }
    return success;
}

bool SkyConnectImpl::connectWithSim() noexcept
{
    HWND hWnd;
    DWORD userEvent;
    if (d->eventWidget != nullptr) {
        hWnd = reinterpret_cast<HWND>(d->eventWidget->winId());
        userEvent = EventWidget::SimConnnectUserMessage;
    } else {
        hWnd = nullptr;
        userEvent = 0;
    }
    HRESULT result = ::SimConnect_Open(&(d->simConnectHandle), ::ConnectionName, hWnd, userEvent, nullptr, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
    if (result == S_OK) {
        setupRequestData();
    }
#ifdef DEBUG
    qDebug("SkyConnectImpl::connectWithSim: CONNECT with SIM, handle: %p success: %d", d->simConnectHandle, result == S_OK);
#endif
    return result == S_OK;
}

bool SkyConnectImpl::isConnectedWithSim() const noexcept
{
    return d->simConnectHandle != nullptr;
}

// PROTECTED SLOTS

void SkyConnectImpl::processEvents() noexcept
{
    updateCurrentTimestamp();
    // Process system events
    ::SimConnect_CallDispatch(d->simConnectHandle, SkyConnectImpl::dispatch, this);
}

// PRIVATE

bool SkyConnectImpl::reconnectWithSim() noexcept
{
    bool res;
    if (close()) {
        res = connectWithSim();
    } else {
        res = false;
    }
    return res;
}

bool SkyConnectImpl::close() noexcept
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

void SkyConnectImpl::setupRequestData() noexcept
{
    // Request data
    SimConnectAircraftInfo::addToDataDefinition(d->simConnectHandle);
    SimConnectPosition::addToDataDefinition(d->simConnectHandle);
    SimConnectEngine::addToDataDefinition(d->simConnectHandle);
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
}

void SkyConnectImpl::setupInitialPosition() noexcept
{
    const Aircraft &userAircraft = getCurrentFlight().getUserAircraftConst();
    const PositionData &positionData = userAircraft.getPositionConst().interpolate(0, TimeVariableData::Access::Seek);
    if (!positionData.isNull()) {
        // Set initial position
        SIMCONNECT_DATA_INITPOSITION initialPosition;

        initialPosition.Latitude = positionData.latitude;
        initialPosition.Longitude = positionData.longitude;
        initialPosition.Altitude = positionData.altitude;
        initialPosition.Pitch = positionData.pitch;
        initialPosition.Bank = positionData.bank;
        initialPosition.Heading = positionData.heading;
        initialPosition.OnGround = userAircraft.getAircraftInfoConst().startOnGround ? 1 : 0;
        initialPosition.Airspeed = userAircraft.getAircraftInfoConst().initialAirspeed;

        ::SimConnect_SetDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftInitialPosition),
                                        ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(::SIMCONNECT_DATA_INITPOSITION), &initialPosition);
    } else {
        stopReplay();
    }
}

void SkyConnectImpl::setSimulationFrozen(bool enable) noexcept
{
    d->frozen = enable;
    const DWORD data = enable ? 1 : 0;
    ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::toUnderlyingType(Event::FreezeLatituteLongitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::toUnderlyingType(Event::FreezeAltitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::toUnderlyingType(Event::FreezeAttitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}

bool SkyConnectImpl::isSimulationFrozen() const noexcept
{
    return d->frozen;
}

void SkyConnectImpl::replay() noexcept
{
    const qint64 currentTimestamp = getCurrentTimestamp();
    if (currentTimestamp <= getCurrentFlight().getTotalDurationMSec()) {
        if (!sendAircraftData(currentTimestamp, TimeVariableData::Access::Linear)) {
            // Connection error
            stopReplay();
        }
    } else {
        // At end of replay
        stopReplay();
    }
}

void SkyConnectImpl::updateRecordFrequency(SampleRate::SampleRate sampleRate) noexcept
{
    if (getState() == Connect::State::Recording) {
        switch (sampleRate) {
        case SampleRate::SampleRate::Hz1:
            if (d->eventWidget != nullptr) {
                d->eventWidget.reset();
                d->eventWidget = nullptr;
                reconnectWithSim();
            }
            // Get aircraft data @1Hz
            updateRequestPeriod(::SIMCONNECT_PERIOD_SECOND);
            break;
        case SampleRate::SampleRate::Auto:
            // Fall-thru intented
        default:
            if (sampleRate == SampleRate::SampleRate::Auto) {
                // Samples are picked up upon availability, using an event-based notification (variable frequency)
                if (d->eventWidget == nullptr) {
                    d->eventWidget = std::make_unique<EventWidget>();
                    connect(d->eventWidget.get(), &EventWidget::simConnectEvent,
                            this, &SkyConnectImpl::processEvents);
                    reconnectWithSim();
                }
            } else if (d->eventWidget != nullptr) {
                // Samples are picked up using timer-based polling, with a fixed frequency
                d->eventWidget.reset();
                d->eventWidget = nullptr;
                reconnectWithSim();
            }
            // Get aircraft data every simulated frame
            updateRequestPeriod(::SIMCONNECT_PERIOD_SIM_FRAME);
            break;
        }
    } else {
        updateRequestPeriod(::SIMCONNECT_PERIOD_NEVER);
    }
}

void SkyConnectImpl::updateRequestPeriod(::SIMCONNECT_PERIOD period)
{
    if (d->currentRequestPeriod != period) {
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::AircraftPosition), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftPositionDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::Engine), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftEngineDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::PrimaryFlightControl), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftPrimaryFlightControlDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::SecondaryFlightControl), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftSecondaryFlightControlDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::AircraftHandle), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftHandleDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::Light), Enum::toUnderlyingType(SimConnectType::DataDefinition::AircraftLightDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, period, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        // Update the flight plan and simulation time only every second
        ::SIMCONNECT_PERIOD oneSecondPeriod = period != ::SIMCONNECT_PERIOD_NEVER ? ::SIMCONNECT_PERIOD_SECOND : ::SIMCONNECT_PERIOD_NEVER;
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::FlightPlan), Enum::toUnderlyingType(SimConnectType::DataDefinition::FlightPlanDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, oneSecondPeriod, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::SimulationTime), Enum::toUnderlyingType(SimConnectType::DataDefinition::SimulationTimeDefinition),
                                            ::SIMCONNECT_OBJECT_ID_USER, oneSecondPeriod, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        d->currentRequestPeriod = period;
    }
}

void CALLBACK SkyConnectImpl::dispatch(SIMCONNECT_RECV *receivedData, DWORD cbData, void *context) noexcept
{
    Q_UNUSED(cbData);

    SkyConnectImpl *skyConnect = static_cast<SkyConnectImpl *>(context);
    Flight &currentFlight = skyConnect->getCurrentFlight();
    Aircraft &userAircraft = currentFlight.getUserAircraft();
    SIMCONNECT_RECV_SIMOBJECT_DATA *objectData;

    bool dataReceived = false;
    switch (receivedData->dwID) {
    case SIMCONNECT_RECV_ID_EVENT:
    {
        const SIMCONNECT_RECV_EVENT *evt = reinterpret_cast<SIMCONNECT_RECV_EVENT *>(receivedData);
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

    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
        objectData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *>(receivedData);
        switch (static_cast<DataRequest>(objectData->dwRequestID)) {
        case DataRequest::AircraftInfo:
        {
            const SimConnectAircraftInfo *simConnectAircraftInfo = reinterpret_cast<const SimConnectAircraftInfo *>(&objectData->dwData);
            AircraftInfo aircraftInfo = simConnectAircraftInfo->toAircraftInfo();
            aircraftInfo.startDate = QDateTime::currentDateTime();
            userAircraft.setAircraftInfo(aircraftInfo);
            FlightCondition flightCondition = simConnectAircraftInfo->toFlightCondition();
            currentFlight.setFlightCondition(flightCondition);
            break;
        }
        default:
            break;
        }
        break;

    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
        objectData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(receivedData);

        switch (static_cast<DataRequest>(objectData->dwRequestID)) {
        case DataRequest::AircraftPosition:
        {
            const SimConnectPosition *simConnectPosition;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectPosition = reinterpret_cast<const SimConnectPosition *>(&objectData->dwData);
                PositionData positionData = simConnectPosition->toPositionData();
                positionData.timestamp = skyConnect->getCurrentTimestamp();
                userAircraft.getPosition().upsert(positionData);
                dataReceived = true;
            }
            break;
        }
        case DataRequest::Engine:
        {
            const SimConnectEngine *simConnectEngine;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectEngine = reinterpret_cast<const SimConnectEngine *>(&objectData->dwData);
                EngineData engineData = simConnectEngine->toEngineData();
                engineData.timestamp = skyConnect->getCurrentTimestamp();
                userAircraft.getEngine().upsert(std::move(engineData));
                dataReceived = true;
            }
            break;
        }
        case DataRequest::PrimaryFlightControl:
        {
            const SimConnectPrimaryFlightControl *simConnectPrimaryFlightControl;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectPrimaryFlightControl = reinterpret_cast<const SimConnectPrimaryFlightControl *>(&objectData->dwData);
                PrimaryFlightControlData primaryFlightControlData = simConnectPrimaryFlightControl->toPrimaryFlightControlData();
                primaryFlightControlData.timestamp = skyConnect->getCurrentTimestamp();
                userAircraft.getPrimaryFlightControl().upsert(std::move(primaryFlightControlData));
                dataReceived = true;
            }
            break;
        }
        case DataRequest::SecondaryFlightControl:
        {
            const SimConnectSecondaryFlightControl *simConnectSecondaryFlightControl;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectSecondaryFlightControl = reinterpret_cast<const SimConnectSecondaryFlightControl *>(&objectData->dwData);
                SecondaryFlightControlData secondaryFlightControlData = simConnectSecondaryFlightControl->toSecondaryFlightControlData();
                secondaryFlightControlData.timestamp = skyConnect->getCurrentTimestamp();
                userAircraft.getSecondaryFlightControl().upsert(std::move(secondaryFlightControlData));
                dataReceived = true;
            }
            break;
        }
        case DataRequest::AircraftHandle:
        {
            const SimConnectAircraftHandle *simConnectAircraftHandle;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectAircraftHandle = reinterpret_cast<const SimConnectAircraftHandle *>(&objectData->dwData);
                AircraftHandleData aircraftHandleData = simConnectAircraftHandle->toAircraftHandleData();
                aircraftHandleData.timestamp = skyConnect->getCurrentTimestamp();
                userAircraft.getAircraftHandle().upsert(std::move(aircraftHandleData));
                dataReceived = true;
            }
            break;
        }
        case DataRequest::Light:
        {
            const SimConnectLight *simConnectLight;
            if (skyConnect->getState() == Connect::State::Recording) {
                simConnectLight = reinterpret_cast<const SimConnectLight *>(&objectData->dwData);
                LightData lightData = simConnectLight->toLightData();
                lightData.timestamp = skyConnect->getCurrentTimestamp();
                userAircraft.getLight().upsert(std::move(lightData));
                dataReceived = true;
            }
            break;
        }
        case DataRequest::FlightPlan:
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
                dataReceived = true;
            }
            break;
        }
        case DataRequest::SimulationTime:
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
                dataReceived = true;
            }
            break;
        }
        default:
            break;
        }
        break;

    case SIMCONNECT_RECV_ID_EVENT_FRAME:
        if (skyConnect->getState() == Connect::State::Replay) {
            skyConnect->replay();
        }
        break;

    case SIMCONNECT_RECV_ID_QUIT:
#ifdef DEBUG
        qDebug("SIMCONNECT_RECV_ID_QUIT");
#endif
        skyConnect->close();
        break;

    case SIMCONNECT_RECV_ID_OPEN:
#ifdef DEBUG
        qDebug("SIMCONNECT_RECV_ID_OPEN");
#endif
        break;

    case SIMCONNECT_RECV_ID_EXCEPTION:
#ifdef DEBUG
    {
        SIMCONNECT_RECV_EXCEPTION *exception = static_cast<SIMCONNECT_RECV_EXCEPTION *>(receivedData);
        qDebug("SIMCONNECT_RECV_ID_EXCEPTION: A server exception %lu happened: sender ID: %lu index: %lu data: %lu",
               exception->dwException, exception->dwSendID, exception->dwIndex, cbData);
    }
#endif
        break;
    case SIMCONNECT_RECV_ID_NULL:
#ifdef DEBUG
        qDebug("SIMCONNECT_RECV_ID_NULL");
#endif
        break;

    default:
        break;
    }

    if (dataReceived) {
        if (!skyConnect->isElapsedTimerRunning()) {
            // Start the elapsed timer with the arrival of the first sample data
            skyConnect->setCurrentTimestamp(0);
            skyConnect->resetElapsedTime(true);
        }
    }
}
