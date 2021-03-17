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

#include <windows.h>
#include <SimConnect.h>

#include <QTimer>
#include <QElapsedTimer>
#include <QtGlobal>
#include <QApplication>
#include <QWidget>

#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/AircraftInfo.h"
#include "../../Kernel/src/AircraftData.h"
#include "../../Kernel/src/SampleRate.h"
#include "../../Kernel/src/Enum.h"
#include "SimConnectDataDefinition.h"
#include "SimConnectAircraftInfo.h"
#include "SimConnectAircraftData.h"
#include "Connect.h"
#include "EventWidget.h"
#include "SkyConnectImpl.h"

namespace
{
    const char *ConnectionName = "SkyConnect";
    constexpr DWORD UserAirplaneRadiusMeters = 0;

    enum class Event: SIMCONNECT_CLIENT_EVENT_ID {
        SimStart,
        Pause,
        Crashed,
        FreezeLatituteLongitude,
        FreezeAltitude,
        FreezeAttitude
    };

    enum class DataRequest: SIMCONNECT_DATA_REQUEST_ID {
        AircraftInfo,
        AircraftPosition
    };
}

class SkyConnectPrivate
{
public:
    SkyConnectPrivate()
        : simConnectHandle(nullptr),
          frozen(false),
          eventWidget(nullptr)
    {
    }

    HANDLE simConnectHandle;
    bool frozen;
    std::unique_ptr<EventWidget> eventWidget;
};

// PUBLIC

SkyConnectImpl::SkyConnectImpl(QObject *parent)
    : AbstractSkyConnect(parent),
      d(std::make_unique<SkyConnectPrivate>())
{
}

SkyConnectImpl::~SkyConnectImpl()
{
    setSimulationFrozen(false);
    close();
}

// PROTECTED

void SkyConnectImpl::onStartDataSample()
{
    updateRecordFrequency(Settings::getInstance().getRecordSampleRate());
    // Get aircraft information
    ::SimConnect_RequestDataOnSimObjectType(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::AircraftInfo), SkyConnectDataDefinition::AircraftInfoDefinition, ::UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
}

void SkyConnectImpl::onStopDataSample()
{
    // Stop receiving aircraft position
    ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::AircraftPosition), SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_NEVER);
}

void SkyConnectImpl::onStartReplay(qint64 currentTimestamp)
{
    // "Freeze" the simulation: position and attitude only set by (interpolated)
    // sample points
    setSimulationFrozen(true);
    if (currentTimestamp == 0) {
        setupInitialPosition();
#ifdef DEBUG
    qDebug("SkyConnectImpl::onStartReplay: current timestamp is 0, SETUP initial position");
#endif
    }
}

void SkyConnectImpl::onStopReplay()
{
    setSimulationFrozen(false);
}

void SkyConnectImpl::onSeek(qint64 currentTimestamp)
{
    if (currentTimestamp == 0) {
        setupInitialPosition();
#ifdef DEBUG
    qDebug("SkyConnectImpl::onSeek: current timestamp is 0, SETUP initial position");
#endif
    }
};

void SkyConnectImpl::onRecordingPaused(bool paused)
{
    Q_UNUSED(paused)
    updateRecordFrequency(Settings::getInstance().getRecordSampleRate());
}

void SkyConnectImpl::onReplayPaused()
{
}

void SkyConnectImpl::onRecordSampleRateChanged(SampleRate::SampleRate sampleRate)
{
     updateRecordFrequency(sampleRate);
}

void SkyConnectImpl::onPlaybackSampleRateChanged(SampleRate::SampleRate sampleRate)
{
    Q_UNUSED(sampleRate)
}

bool SkyConnectImpl::sendAircraftData(qint64 currentTimestamp)
{
    Q_UNUSED(currentTimestamp)
    return sendAircraftData();
}

bool SkyConnectImpl::connectWithSim()
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
    return result == S_OK;
}

bool SkyConnectImpl::isConnectedWithSim() const
{
    return d->simConnectHandle != nullptr;
}

// PROTECTED SLOTS

void SkyConnectImpl::processEvents()
{
    switch (getState()) {
    case Connect::State::Recording:
        updateCurrentTimestamp();
        break;
    case Connect::State::Playback:
        replay();
        break;
    default:
        break;
    }

    // Process system events
    ::SimConnect_CallDispatch(d->simConnectHandle, SkyConnectImpl::dispatch, this);
}

// PRIVATE

bool SkyConnectImpl::reconnectWithSim()
{
    bool res;
    if (close()) {
        res = connectWithSim();
    } else {
        res = false;
    }
    return res;
}

bool SkyConnectImpl::close()
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

void SkyConnectImpl::setupRequestData()
{
    // Set up the data definition, but do not yet do anything with it
    SimConnectAircraftInfo::addToDataDefinition(d->simConnectHandle);
    SimConnectAircraftData::addToDataDefinition(d->simConnectHandle);

    ::SimConnect_AddToDataDefinition(d->simConnectHandle, SkyConnectDataDefinition::AircraftInitialPosition, "Initial Position", nullptr, ::SIMCONNECT_DATATYPE_INITPOSITION);

    // System event subscription
    ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::SimStart), "SimStart");
    ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Pause), "Pause");
    ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::Crashed), "Crashed");

    // Client events
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::FreezeLatituteLongitude), "FREEZE_LATITUDE_LONGITUDE_SET");
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::FreezeAltitude), "FREEZE_ALTITUDE_SET");
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, Enum::toUnderlyingType(Event::FreezeAttitude), "FREEZE_ATTITUDE_SET");
}

void SkyConnectImpl::setupInitialPosition()
{
    const AircraftData &aircraftData = getAircraft().getAircraftData(0);
    if (!aircraftData.isNull()) {
        // Set initial position
        SIMCONNECT_DATA_INITPOSITION initialPosition;

        initialPosition.Latitude = aircraftData.latitude;
        initialPosition.Longitude = aircraftData.longitude;
        initialPosition.Altitude = aircraftData.altitude;
        initialPosition.Pitch = aircraftData.pitch;
        initialPosition.Bank = aircraftData.bank;
        initialPosition.Heading = aircraftData.heading;
        initialPosition.OnGround = getAircraft().getAircraftInfo().startOnGround ? 1 : 0;
        initialPosition.Airspeed = getAircraft().getAircraftInfo().initialAirspeed;

        ::SimConnect_SetDataOnSimObject(d->simConnectHandle, SkyConnectDataDefinition::AircraftInitialPosition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(::SIMCONNECT_DATA_INITPOSITION), &initialPosition);
    } else {
        stopReplay();
    }
}

void SkyConnectImpl::setSimulationFrozen(bool enable) {
    DWORD data;

    d->frozen = enable;
    data = enable ? 1 : 0;
    ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::toUnderlyingType(Event::FreezeLatituteLongitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::toUnderlyingType(Event::FreezeAltitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, Enum::toUnderlyingType(Event::FreezeAttitude), data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}

bool SkyConnectImpl::isSimulationFrozen() const {
    return d->frozen;
}

bool SkyConnectImpl::sendAircraftData()
{
    bool success;

    const AircraftData &currentAircraftData = updateCurrentAircraftData();
    if (!currentAircraftData.isNull()) {
        SimConnectAircraftData simConnectAircraftData;
        simConnectAircraftData.fromAircraftData(currentAircraftData);
#ifdef DEBUG
        qDebug("%f, %f, %f, %f, %f, %f, %lli",
               simConnectAircraftData.longitude, simConnectAircraftData.latitude, simConnectAircraftData.altitude,
               simConnectAircraftData.pitch, simConnectAircraftData.bank, simConnectAircraftData.heading,
               getCurrentTimestamp());
#endif
        HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(SimConnectAircraftData), &simConnectAircraftData);
        success = res == S_OK;

        // Start the elapsed timer after sending the first sample data
        if (!isElapsedTimerRunning()) {
            startElapsedTimer();
        }
    } else {
        success = false;
    }
    return success;
}

void SkyConnectImpl::replay()
{
    updateCurrentTimestamp();
    if (sendAircraftData()) {
        emit aircraftDataSent(getCurrentTimestamp());
    } else {
        stopReplay();
    }
}

void SkyConnectImpl::updateRecordFrequency(SampleRate::SampleRate sampleRate)
{
    if (getState() == Connect::State::Recording) {
        switch (sampleRate) {
        case SampleRate::SampleRate::Hz1:
            if (d->eventWidget != nullptr) {
                d->eventWidget.reset();
                d->eventWidget = nullptr;
                reconnectWithSim();
            }
            // Get aircraft position @1Hz
            ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::AircraftPosition), SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_SECOND, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
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
            // Get aircraft position every simulated frame
            ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::AircraftPosition), SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_SIM_FRAME, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
            break;
        }
    } else {
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, Enum::toUnderlyingType(DataRequest::AircraftPosition), SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_NEVER);
    }
}

void CALLBACK SkyConnectImpl::dispatch(SIMCONNECT_RECV *receivedData, DWORD cbData, void *context)
{
    Q_UNUSED(cbData);

    SkyConnectImpl *skyConnect = static_cast<SkyConnectImpl *>(context);
    SIMCONNECT_RECV_SIMOBJECT_DATA *objectData;
    SIMCONNECT_RECV_EXCEPTION *exception;
    const SimConnectAircraftInfo *simConnectAircraftInfo;
    const SimConnectAircraftData *simConnectAircraftData;

    switch (receivedData->dwID)
    {
        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT *evt = reinterpret_cast<SIMCONNECT_RECV_EVENT *>(receivedData);
            switch (static_cast<Event>(evt->uEventID))
            {
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
                    // (in-game meu") is entered
                    skyConnect->setPaused(evt->dwData == 1);
                    break;

                case Event::Crashed:
#ifdef DEBUG
                qDebug("SIMCONNECT_RECV_ID_EVENT: CRASHED event");
#endif
                switch (skyConnect->getState()) {
                case Connect::State::Recording:
                    skyConnect->stopDataSample();
                    break;
                case Connect::State::Playback:
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
            switch (static_cast<DataRequest>(objectData->dwRequestID))
            {
                case DataRequest::AircraftInfo:
                {
                    simConnectAircraftInfo = reinterpret_cast<const SimConnectAircraftInfo *>(&objectData->dwData);
                    AircraftInfo aircraftInfo;
                    aircraftInfo = std::move(simConnectAircraftInfo->toAircraftInfo());
                    skyConnect->getAircraft().setAircraftInfo(std::move(aircraftInfo));
                    break;
                }

                default:
                   break;
            }
            break;

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
            objectData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(receivedData);

            switch (static_cast<DataRequest>(objectData->dwRequestID))
            {
                case DataRequest::AircraftPosition:
                {
                    if (skyConnect->getState() == Connect::State::Recording) {
                        if (!skyConnect->isElapsedTimerRunning()) {
                            // Start the elapsed timer with the arrival of the first sample data
                            skyConnect->setCurrentTimestamp(0);
                            skyConnect->resetElapsedTime(true);
                        }
                        simConnectAircraftData = reinterpret_cast<const SimConnectAircraftData *>(&objectData->dwData);
                        AircraftData aircraftData = simConnectAircraftData->toAircraftData();
                        aircraftData.timestamp = skyConnect->getCurrentTimestamp();
                        skyConnect->getAircraft().upsertAircraftData(std::move(aircraftData));
                    }

                    break;
                }
                default:
                    break;
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
        exception = static_cast<SIMCONNECT_RECV_EXCEPTION *>(receivedData);

            qDebug("SIMCONNECT_RECV_ID_EXCEPTION: A server exception %lu happened: sender ID: %lu index: %lu data: %lu",
                   exception->dwException, exception->dwSendID, exception->dwIndex, cbData);
#endif

        case SIMCONNECT_RECV_ID_NULL:
#ifdef DEBUG
            qDebug("SIMCONNECT_RECV_ID_NULL");
#endif
            break;

        default:
            break;
    }
}
