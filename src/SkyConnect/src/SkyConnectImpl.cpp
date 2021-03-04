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
#include <windows.h>
#include <SimConnect.h>

#include <QtGlobal>

#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/AircraftInfo.h"
#include "../../Kernel/src/AircraftData.h"
#include "../../Kernel/src/SampleRate.h"
#include "SimConnectDataDefinition.h"
#include "SimConnectAircraftInfo.h"
#include "SimConnectAircraftData.h"
#include "Connect.h"
#include "SkyConnectImpl.h"

namespace
{
    const char *ConnectionName = "SkyConnect";
    constexpr DWORD UserAirplaneRadiusMeters = 0;
    constexpr qint64 SkipMSec = 1000;

    enum Event {
        SimStartEvent,
        PauseEvent,
        CrashedEvent,
        FreezeLatituteLongitude,
        FreezeAltitude,
        FreezeAttitude
    };

    enum DataRequest {
        AircraftInfoRequest,
        AircraftPositionRequest
    };
}

class SkyConnectPrivate
{
public:
    SkyConnectPrivate()
        : simConnectHandle(nullptr),
          state(Connect::State::Idle),
          currentTimestamp(0),
          recordSampleRate(Settings::getInstance().getRecordSampleRateValue()),
          recordIntervalMSec(static_cast<int>(1.0 / recordSampleRate * 1000.0)),
          playbackSampleRate(Settings::getInstance().getPlaybackSampleRateValue()),
          playbackIntervalMSec(static_cast<int>(1.0 / playbackSampleRate * 1000.0)),
          timeScale(1.0),
          elapsedTime(0),
          frozen(false)
    {
    }

    HANDLE simConnectHandle;
    Connect::State state;
    QTimer timer;
    qint64 currentTimestamp;
    AircraftData currentAircraftData;
    QElapsedTimer elapsedTimer;
    Aircraft aircraft;
    double recordSampleRate;
    int    recordIntervalMSec;
    double playbackSampleRate;
    int    playbackIntervalMSec;
    double timeScale;
    qint64 elapsedTime;
    bool frozen;
};

// PUBLIC

SkyConnectImpl::SkyConnectImpl(QObject *parent)
    : QObject(parent),
      d(new SkyConnectPrivate())
{
    frenchConnection();
}

SkyConnectImpl::~SkyConnectImpl()
{
    setSimulationFrozen(false);
    close();
    delete d;
}

bool SkyConnectImpl::open()
{
    HRESULT result = ::SimConnect_Open(&(d->simConnectHandle), ::ConnectionName, nullptr, 0, nullptr, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
    setupRequestData();
    return result == S_OK;
}

bool SkyConnectImpl::close()
{
    HRESULT result;

    stopAll();
    if (d->simConnectHandle != nullptr) {
        result = ::SimConnect_Close(d->simConnectHandle);
        d->simConnectHandle = nullptr;
    } else {
        result = S_OK;
    }
    return result == S_OK;
}

bool SkyConnectImpl::isConnected() const
{
    return d->simConnectHandle != nullptr;
}

void SkyConnectImpl::startDataSample()
{
    if (!isConnected()) {
        open();
    }

    if (isConnected()) {
        setState(Connect::State::Recording);
        d->aircraft.clear();
        d->timer.setInterval(d->recordIntervalMSec);

        // Get aircraft position every simulated frame
        ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, ::AircraftPositionRequest, SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_SIM_FRAME, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        // Get aircraft information
        ::SimConnect_RequestDataOnSimObjectType(d->simConnectHandle, AircraftInfoRequest, SkyConnectDataDefinition::AircraftInfoDefinition, ::UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
        d->elapsedTimer.invalidate();
        d->timer.start();
    } else {
        setState(Connect::State::NoConnection);
    }
}

void SkyConnectImpl::stopDataSample()
{
    // Get aircraft position every simulated frame
    ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, ::AircraftPositionRequest, SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_NEVER);
    d->timer.stop();
    setState(Connect::State::Idle);
}

void SkyConnectImpl::startReplay(bool fromStart)
{
    if (!isConnected()) {
        open();
    }
    if (isConnected()) {
        setState(Connect::State::Playback);
        // "Freeze" the simulation: position and attitude only set by (interpolated)
        // sample points
        setSimulationFrozen(true);
        d->timer.setInterval(d->playbackIntervalMSec);

        if (fromStart) {
            setupInitialPosition();
            d->elapsedTime = 0;
            d->currentTimestamp = 0;
        }

        d->elapsedTimer.invalidate();
        d->timer.start();

    } else {
        setState(Connect::State::NoConnection);
    }
}

void SkyConnectImpl::stopReplay()
{
    setState(Connect::State::Idle);
    d->timer.stop();
    // Remember elapsed time since last replay start, in order to continue from
    // current timestamp
    d->elapsedTime = d->currentTimestamp;
    setSimulationFrozen(false);
}

void SkyConnectImpl::setPaused(bool enabled)
{
    Connect::State newState;
    if (enabled) {
        switch (d->state) {
        case Connect::Recording:
            newState = Connect::RecordingPaused;
            // Store the elapsed recording time...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed();
            // ... and stop the elapsed timer
            ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, ::AircraftPositionRequest, SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_NEVER);
            d->elapsedTimer.invalidate();
            break;
        case Connect::Playback:
            newState = Connect::PlaybackPaused;
            // Store the elapsed playback time measured with the current time scale...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed() * d->timeScale;
            // ... and stop the elapsed timer
            d->elapsedTimer.invalidate();
            break;
         default:
            // No state change
            newState = d->state;
        }
    } else {
        switch (d->state) {
        case Connect::RecordingPaused:
            newState = Connect::Recording;
            if (hasRecordingStarted()) {
                // Resume recording (but only if it has already recorded samples before)
                d->elapsedTimer.start();
            }
            ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, ::AircraftPositionRequest, SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_SIM_FRAME, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
            break;
        case Connect::PlaybackPaused:
            newState = Connect::Playback;
            d->elapsedTimer.start();
            break;
         default:
            // No state change
            newState = d->state;
        }
    }
    setState(newState);
}

bool SkyConnectImpl::isPaused() const {
    return d->state == Connect::RecordingPaused || d->state == Connect::PlaybackPaused;
}

void SkyConnectImpl::skipToBegin()
{
    setCurrentTimestamp(0);
}

void SkyConnectImpl::skipBackward()
{
    qint64 newTimeStamp = qMax(this->getCurrentTimestamp() - SkipMSec, 0ll);
    setCurrentTimestamp(newTimeStamp);
}

void SkyConnectImpl::skipForward()
{
    qint64 endTimeStamp = d->aircraft.getLastAircraftData().timestamp;
    qint64 newTimeStamp = qMin(this->getCurrentTimestamp() + SkipMSec, endTimeStamp);
    setCurrentTimestamp(newTimeStamp);
}

void SkyConnectImpl::skipToEnd()
{
    qint64 endTimeStamp  = d->aircraft.getLastAircraftData().timestamp;
    setCurrentTimestamp(endTimeStamp);
}

Aircraft &SkyConnectImpl::getAircraft()
{
    return d->aircraft;
}

const Aircraft &SkyConnectImpl::getAircraft() const
{
    return d->aircraft;
}

void SkyConnectImpl::setTimeScale(double timeScale)
{
    if (!qFuzzyCompare(d->timeScale, timeScale)) {
        // If the elapsed timer is running...
        if (d->elapsedTimer.isValid()) {
            // ... then store the elapsed time measured with the previous scale...
            d->elapsedTime = d->elapsedTime + d->elapsedTimer.elapsed() * d->timeScale;
            // ... and restart timer
            d->elapsedTimer.start();
        }
        d->timeScale = timeScale;
    }
}

double SkyConnectImpl::getTimeScale() const
{
    return d->timeScale;
}

Connect::State SkyConnectImpl::getState() const
{
    return d->state;
}

void SkyConnectImpl::setCurrentTimestamp(qint64 timestamp)
{
    if (d->state != Connect::State::Recording) {
        d->currentTimestamp = timestamp;
        d->elapsedTime = d->currentTimestamp;
        if (sendAircraftPosition()) {
            emit aircraftDataSent(d->currentTimestamp);
            if (d->elapsedTimer.isValid() && d->state == Connect::State::Playback) {
                // Restart the elapsed timer, counting onwards from the newly
                // set timestamp
                d->elapsedTimer.start();
            }
        }
    }
}

qint64 SkyConnectImpl::getCurrentTimestamp() const
{
    return d->currentTimestamp;
}

bool SkyConnectImpl::isAtEnd() const
{
    return d->currentTimestamp >= d->aircraft.getLastAircraftData().timestamp;
}

const AircraftData &SkyConnectImpl::getCurrentAircraftData() const
{
    return d->currentAircraftData;
}

// PRIVATE

void SkyConnectImpl::frenchConnection()
{
    connect(&(d->timer), &QTimer::timeout,
            this, &SkyConnectImpl::processEvents);
    connect(&Settings::getInstance(), &Settings::recordSampleRateChanged,
            this, &SkyConnectImpl::handleRecordSampleRateChanged);
    connect(&Settings::getInstance(), &Settings::playbackSampleRateChanged,
            this, &SkyConnectImpl::handlePlaybackSampleRateChanged);
}

void SkyConnectImpl::setupRequestData()
{
    // Set up the data definition, but do not yet do anything with it
    SimConnectAircraftInfo::addToDataDefinition(d->simConnectHandle);
    SimConnectAircraftData::addToDataDefinition(d->simConnectHandle);

    ::SimConnect_AddToDataDefinition(d->simConnectHandle, SkyConnectDataDefinition::AircraftInitialPosition, "Initial Position", nullptr, ::SIMCONNECT_DATATYPE_INITPOSITION);

    // System event subscription
    ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, SimStartEvent, "SimStart");
    ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, PauseEvent, "Pause");
    ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, CrashedEvent, "Crashed");

    // Client events
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, FreezeLatituteLongitude, "FREEZE_LATITUDE_LONGITUDE_SET");
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, FreezeAltitude, "FREEZE_ALTITUDE_SET");
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, FreezeAttitude, "FREEZE_ATTITUDE_SET");
}

void SkyConnectImpl::setupInitialPosition()
{
    const AircraftData &aircraftData = d->aircraft.getAllAircraftData(0);
    if (!aircraftData.isNull()) {
        // Set initial position
        SIMCONNECT_DATA_INITPOSITION initialPosition;

        initialPosition.Latitude = aircraftData.latitude;
        initialPosition.Longitude = aircraftData.longitude;
        initialPosition.Altitude = aircraftData.altitude;
        initialPosition.Pitch = aircraftData.pitch;
        initialPosition.Bank = aircraftData.bank;
        initialPosition.Heading = aircraftData.heading;
        initialPosition.OnGround = d->aircraft.getAircraftInfo().startOnGround ? 1 : 0;
        initialPosition.Airspeed = d->aircraft.getAircraftInfo().initialAirspeed;

        ::SimConnect_SetDataOnSimObject(d->simConnectHandle, SkyConnectDataDefinition::AircraftInitialPosition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(::SIMCONNECT_DATA_INITPOSITION), &initialPosition);
    } else {
        stopReplay();
    }
}

void SkyConnectImpl::setSimulationFrozen(bool enable) {
    DWORD data;

    d->frozen = enable;
    if (enable) {
        data = 1;
    } else {
        data = 0;
    }
    ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, ::FreezeLatituteLongitude, data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, ::FreezeAltitude, data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
    ::SimConnect_TransmitClientEvent(d->simConnectHandle, ::SIMCONNECT_OBJECT_ID_USER, ::FreezeAttitude, data, ::SIMCONNECT_GROUP_PRIORITY_HIGHEST, ::SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
}

bool SkyConnectImpl::isSimulationFrozen() const {
    return d->frozen;
}

bool SkyConnectImpl::sendAircraftPosition() const
{
    bool success;
    d->currentAircraftData = std::move(d->aircraft.getAllAircraftData(d->currentTimestamp));

    if (!d->currentAircraftData.isNull()) {
        SimConnectAircraftData simConnectAircraftData;
        simConnectAircraftData.fromAircraftData(d->currentAircraftData);
#ifdef DEBUG
        qDebug("%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %d, %lli",
               simConnectAircraftData.longitude, simConnectAircraftData.latitude, simConnectAircraftData.altitude,
               simConnectAircraftData.pitch, simConnectAircraftData.bank, simConnectAircraftData.heading,
               simConnectAircraftData.yokeXPosition,
               simConnectAircraftData.yokeYPosition,
               simConnectAircraftData.rudderPosition,
               simConnectAircraftData.elevatorPosition,
               simConnectAircraftData.aileronPosition,
               simConnectAircraftData.flapsHandleIndex,
               d->currentTimestamp);
#endif
        HRESULT res = ::SimConnect_SetDataOnSimObject(d->simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(SimConnectAircraftData), &simConnectAircraftData);
        success = res == S_OK;

        // Start the elapsed timer after sending the first sample data
        if (!d->elapsedTimer.isValid()) {
            d->elapsedTimer.start();
        }
    } else {
        success = false;
    }
    return success;
}

void SkyConnectImpl::replay()
{
    if (d->elapsedTimer.isValid()) {
        d->currentTimestamp = d->elapsedTime + static_cast<qint64>(d->elapsedTimer.elapsed() * d->timeScale);
    }

    if (sendAircraftPosition()) {
        emit aircraftDataSent(d->currentTimestamp);
    } else {
        stopReplay();
    }
}

void SkyConnectImpl::stopAll()
{
    stopDataSample();
    stopReplay();
}

void SkyConnectImpl::updateCurrentTimestamp()
{
    if (d->elapsedTimer.isValid()) {
        d->currentTimestamp = d->elapsedTime + d->elapsedTimer.elapsed();
    }
}

void SkyConnectImpl::setState(Connect::State state)
{
    if (d->state != state) {
        d->state = state;
        emit stateChanged(state);
    }
}

bool SkyConnectImpl::hasRecordingStarted() const
{
    return d->aircraft.getAllAircraftData().count();
}

void SkyConnectImpl::handleRecordSampleRateChanged(double sampleRateValue)
{
    d->recordSampleRate = sampleRateValue;
    d->recordIntervalMSec = static_cast<int>(1.0 / d->recordSampleRate * 1000.0);
    d->timer.setInterval(d->recordIntervalMSec);
}

void SkyConnectImpl::handlePlaybackSampleRateChanged(double sampleRateValue)
{
    d->playbackSampleRate = sampleRateValue;
    d->playbackIntervalMSec = static_cast<int>(1.0 / d->playbackSampleRate * 1000.0);
    d->timer.setInterval(d->playbackIntervalMSec);
}

void CALLBACK SkyConnectImpl::dispatch(SIMCONNECT_RECV *receivedData, DWORD cbData, void *context)
{
    Q_UNUSED(cbData);

    SkyConnectImpl *skyConnect = static_cast<SkyConnectImpl *>(context);
    SIMCONNECT_RECV_SIMOBJECT_DATA *objectData;
    SIMCONNECT_RECV_EXCEPTION *exception;
    SimConnectAircraftInfo *simConnectAircraftInfo;
    SimConnectAircraftData *simConnectAircraftData;

    switch (receivedData->dwID)
    {
        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT *evt = reinterpret_cast<SIMCONNECT_RECV_EVENT *>(receivedData);
            switch (evt->uEventID)
            {
                case SimStartEvent:
#ifdef DEBUG
                    qDebug("SIMCONNECT_RECV_ID_EVENT: SIMSTART event");
#endif
                    break;

                case PauseEvent:
#ifdef DEBUG
                    qDebug("SIMCONNECT_RECV_ID_EVENT: PAUSE event: %lu", evt->dwData);
#endif
                    // It seems that the pause event is currently only triggered by selecting "Pause Simulation"
                    // in the developer mode (FS 2020), but neither when "active pause" is selected nor when ESC
                    // (in-game meu") is entered
                    skyConnect->setPaused(evt->dwData == 1);
                    break;

                case CrashedEvent:
#ifdef DEBUG
                qDebug("SIMCONNECT_RECV_ID_EVENT: CRASHED event");
#endif
                switch (skyConnect->d->state) {
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
            switch (objectData->dwRequestID)
            {
                case AircraftInfoRequest:
                {
                    simConnectAircraftInfo = reinterpret_cast<SimConnectAircraftInfo *>(&objectData->dwData);
                    AircraftInfo aircraftInfo;
                    aircraftInfo = std::move(simConnectAircraftInfo->toAircraftInfo());
                    skyConnect->d->aircraft.setAircraftInfo(std::move(aircraftInfo));
                    break;
                }

                default:
                   break;
            }
            break;

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
            objectData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(receivedData);

            switch (objectData->dwRequestID)
            {
                case AircraftPositionRequest:
                {
                    if (skyConnect->d->state == Connect::State::Recording) {
                        if (!skyConnect->d->elapsedTimer.isValid()) {
                            // Start the elapsed timer with the arrival of the first sample data
                            skyConnect->d->currentTimestamp = 0;
                            skyConnect->d->elapsedTime = 0;
                            skyConnect->d->elapsedTimer.start();
                        }
                        simConnectAircraftData = reinterpret_cast<::SimConnectAircraftData *>(&objectData->dwData);
                        AircraftData aircraftData = simConnectAircraftData->toAircraftData();
                        aircraftData.timestamp = skyConnect->d->currentTimestamp;
                        skyConnect->d->aircraft.upsertAircraftData(std::move(aircraftData));
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

// PRIVATE SLOTS

void SkyConnectImpl::processEvents()
{
    switch (d->state) {
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
