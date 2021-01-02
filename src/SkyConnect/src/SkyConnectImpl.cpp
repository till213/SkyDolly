#include <windows.h>
#include <strsafe.h>
#include <SimConnect.h>

#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/Position.h"
#include "SimConnectDataDefinition.h"
#include "SimConnectAircraftInfo.h"
#include "SimConnectPosition.h"
#include "SkyConnectImpl.h"

namespace {

    const char *ConnectionName = "SkyConnect";
    const DWORD UserAirplaneRadiusMeters = 0;

    enum EventID{
        SimStartEvent
    };

    enum DataRequestID {
        AircraftInfoRequest,
        AircraftPositionRequest
    };

    enum SimConnectOnGround {
        SimConnectAirborne = 0,
        SimConnectOnGround = 1
    };
}

class SkyConnectPrivate
{
public:
    SkyConnectPrivate()
        : simConnectHandle(nullptr),
          currentTimestamp(0),
          firstSample(true)
    {
    }

    HANDLE simConnectHandle;
    QTimer timer;
    qint64 currentTimestamp;
    QElapsedTimer elapsedTimer;
    Aircraft aircraft;
    bool firstSample;

    static const int SampleIntervalMSec;
    static const int ReplayIntervalMSec;

};

// Sample the position data at 60 Hz
const int SkyConnectPrivate::SampleIntervalMSec = 1000.0; // static_cast<int>(1.0 / 15.0 * 1000.0);
const int SkyConnectPrivate::ReplayIntervalMSec = static_cast<int>(1.0 / 60.0 * 1000.0);

// PUBLIC

SkyConnectImpl::SkyConnectImpl(QObject *parent)
    : QObject(parent),
      d(new SkyConnectPrivate())
{
    this->frenchConnection();
}

SkyConnectImpl::~SkyConnectImpl()
{
    this->close();
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

    this->stopDataSample();
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
    HRESULT res;
    if (isConnected()) {
        d->timer.disconnect();
        connect(&(d->timer), &QTimer::timeout,
                this, &SkyConnectImpl::sampleData);
        d->timer.setInterval(SkyConnectPrivate::SampleIntervalMSec);

        // Get aircraft position every simulated frame
        res = ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, ::AircraftPositionRequest, ::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_SIM_FRAME, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        d->elapsedTimer.invalidate();
        d->firstSample = true;
        d->timer.start();
    }
}

void SkyConnectImpl::stopDataSample()
{
    HRESULT res;
    // Get aircraft position every simulated frame
    res = ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, ::AircraftPositionRequest, ::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_NEVER);
    d->timer.stop();
}

void SkyConnectImpl::startReplay()
{
    if (isConnected()) {
        d->timer.disconnect();
        connect(&(d->timer), &QTimer::timeout,
                this, &SkyConnectImpl::replay);
        d->timer.setInterval(SkyConnectPrivate::ReplayIntervalMSec);
        d->elapsedTimer.start();
        d->timer.start();
    }
}

void SkyConnectImpl::stopReplay()
{
    d->timer.stop();
}

Aircraft &SkyConnectImpl::getAircraft()
{
    return d->aircraft;
}

const Aircraft &SkyConnectImpl::getAircraft() const
{
    return d->aircraft;
}

// PRIVATE

void SkyConnectImpl::frenchConnection()
{
    connect(&(d->timer), &QTimer::timeout,
            this, &SkyConnectImpl::sampleData);
}

void SkyConnectImpl::setupRequestData()
{
    HRESULT res;

    // Set up the data definition, but do not yet do anything with it
    SimConnectAircraftInfo::addToDataDefinition(d->simConnectHandle);
    SimConnectPosition::addToDataDefinition(d->simConnectHandle);

    // Request an event when the simulation starts
    res = ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, SimStartEvent, "SimStart");
}

void CALLBACK SkyConnectImpl::sampleDataCallback(SIMCONNECT_RECV* receivedData, DWORD cbData, void *context)
{
    Q_UNUSED(cbData);

    SkyConnectImpl *skyConnect = static_cast<SkyConnectImpl *>(context);
    DWORD objectID;
    SIMCONNECT_RECV_SIMOBJECT_DATA *objectData;
    SimConnectAircraftInfo *simConnectAircraftInfo;
    SimConnectPosition *simConnectPosition;
    QByteArray name;
    Position position;

    switch(receivedData->dwID)
    {
        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT *evt = reinterpret_cast<SIMCONNECT_RECV_EVENT *>(receivedData);
            switch(evt->uEventID)
            {
                case SimStartEvent:
                    qDebug("SimStartEvent");
                    break;

                default:
                   qDebug("Unhandled event");
                   break;
            }
            break;
        }

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
            objectData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *>(receivedData);
            switch(objectData->dwRequestID)
            {
                case AircraftInfoRequest:

                    objectID = objectData->dwObjectID;
                    simConnectAircraftInfo = reinterpret_cast<SimConnectAircraftInfo *>(&objectData->dwData);
                    // security check
                    if (SUCCEEDED(StringCbLengthA(&simConnectAircraftInfo->title[0], sizeof(simConnectAircraftInfo->title), nullptr)))
                    {
                        name = simConnectAircraftInfo->title;
                        skyConnect->d->aircraft.setName(name);
                    }
                    break;


                default:
                   break;
            }
            break;

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
            objectData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(receivedData);

            switch(objectData->dwRequestID)
            {
                case AircraftPositionRequest:
                    simConnectPosition = reinterpret_cast<::SimConnectPosition *>(&objectData->dwData);
                    position = simConnectPosition->toPosition();
                    position.timestamp = skyConnect->d->currentTimestamp;
                    skyConnect->d->aircraft.appendPosition(std::move(position));
                    if (skyConnect->d->firstSample) {
                        skyConnect->d->elapsedTimer.start();
                        skyConnect->d->firstSample = false;
                    }
                    break;

                default:
                    break;
            }
            break;

        case SIMCONNECT_RECV_ID_QUIT:
            qDebug("SIMCONNECT_RECV_ID_QUIT");
            skyConnect->stopAll();
            break;

        case SIMCONNECT_RECV_ID_OPEN:
            qDebug("SIMCONNECT_RECV_ID_OPEN");
            break;

        case SIMCONNECT_RECV_ID_NULL:
            qDebug("SIMCONNECT_RECV_ID_NULL");
            break;

        default:
            qDebug("Default: Received: %lx", receivedData->dwID);
            break;
    }
}

// PRIVATE SLOTS

void SkyConnectImpl::replay()
{
    d->currentTimestamp = d->elapsedTimer.elapsed();
    const Position &position = d->aircraft.getPosition(d->currentTimestamp);

    if (position.isValid()) {
        SimConnectPosition simConnectPosition;
        simConnectPosition.fromPosition(position);
        qDebug("Replay: lon: %f lat: %f alt: %f pitch: %f bank: %f head: %f time: %lli",
               simConnectPosition.longitude, simConnectPosition.latitude, simConnectPosition.altitude,
               simConnectPosition.pitch, simConnectPosition.bank, simConnectPosition.heading,
               d->currentTimestamp);
        ::SimConnect_SetDataOnSimObject(d->simConnectHandle, AircraftPositionDefinition, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(SimConnectPosition), &simConnectPosition);
    } else {
        this->stopReplay();
    }
}

void SkyConnectImpl::stopAll()
{
    stopDataSample();
    stopReplay();
}

void SkyConnectImpl::sampleData()
{
    HRESULT res;
    if (d->elapsedTimer.isValid()) {
        d->currentTimestamp = d->elapsedTimer.elapsed();
    } else {
        d->currentTimestamp = 0;
        // First sample: request aircraft information
        res = ::SimConnect_RequestDataOnSimObjectType(d->simConnectHandle, AircraftInfoRequest, AircraftInfoDefinition, ::UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
    }
    ::SimConnect_CallDispatch(d->simConnectHandle, SkyConnectImpl::sampleDataCallback, this);
}
