#include <windows.h>
#include <SimConnect.h>

#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/AircraftInfo.h"
#include "../../Kernel/src/Position.h"
#include "SimConnectDataDefinition.h"
#include "SimConnectAircraftInfo.h"
#include "SimConnectPosition.h"
#include "Frequency.h"
#include "SkyConnectImpl.h"

namespace {

    const char *ConnectionName = "SkyConnect";
    const DWORD UserAirplaneRadiusMeters = 0;

    enum GroupID {
        InitGroup,
        ClientEventGroup
    };

    enum EventID{
        SimStartEvent,
        InitEvent,
        FreezeLatituteLongitude,
        FreezeAltitude,
        FreezeAttitude
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
          sampleFrequency(10.0),
          sampleIntervalMSec(static_cast<int>(1.0 / sampleFrequency * 1000.0)),
          replayFrequency(30.0),
          replayIntervalMSec(static_cast<int>(1.0 / replayFrequency * 1000.0)),
          timeScale(1.0),
          elapsedScaled(0),
          frozen(false)
    {
    }

    HANDLE simConnectHandle;
    QTimer timer;
    qint64 currentTimestamp;
    QElapsedTimer elapsedTimer;
    Aircraft aircraft;
    double sampleFrequency;
    int    sampleIntervalMSec;
    double replayFrequency;
    int    replayIntervalMSec;
    double timeScale;
    qint64 elapsedScaled;
    bool frozen;
};

// PUBLIC

SkyConnectImpl::SkyConnectImpl(QObject *parent)
    : QObject(parent),
      d(new SkyConnectPrivate())
{
    this->frenchConnection();
}

SkyConnectImpl::~SkyConnectImpl()
{
    this->setSimulationFrozen(false);
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
    if (!isConnected()) {
        this->open();
    }

    if (isConnected()) {
        d->timer.disconnect();
        connect(&(d->timer), &QTimer::timeout,
                this, &SkyConnectImpl::sampleData);
        d->timer.setInterval(d->sampleIntervalMSec);

        // Get aircraft position every simulated frame
        res = ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, ::AircraftPositionRequest, SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_SIM_FRAME, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);
        d->elapsedTimer.invalidate();
        d->timer.start();
    }
}

void SkyConnectImpl::stopDataSample()
{
    HRESULT res;
    // Get aircraft position every simulated frame
    res = ::SimConnect_RequestDataOnSimObject(d->simConnectHandle, ::AircraftPositionRequest, SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_NEVER);
    d->timer.stop();
}

void SkyConnectImpl::startReplay()
{
    if (!isConnected()) {
        this->open();
    }
    if (isConnected()) {
        // "Freeze" the simulation: position and attitude only set by (interpolated)
        // sample points
        setSimulationFrozen(true);
        d->timer.disconnect();
        connect(&(d->timer), &QTimer::timeout,
                this, &SkyConnectImpl::replay);
        d->timer.setInterval(d->replayIntervalMSec);
        this->setupInitialPosition();
        d->elapsedTimer.invalidate();
        d->timer.start();
    }
}

void SkyConnectImpl::stopReplay()
{
    d->timer.stop();
    setSimulationFrozen(false);
}

Aircraft &SkyConnectImpl::getAircraft()
{
    return d->aircraft;
}

const Aircraft &SkyConnectImpl::getAircraft() const
{
    return d->aircraft;
}

void SkyConnectImpl::setSampleFrequency(Frequency::Frequency frequency)
{
    d->sampleFrequency = Frequency::toValue(frequency);
    d->sampleIntervalMSec = static_cast<int>(1.0 / d->sampleFrequency * 1000.0);
    d->timer.setInterval(d->sampleIntervalMSec);
}

Frequency::Frequency SkyConnectImpl::getSampleFrequency() const
{
    return Frequency::fromValue(d->sampleFrequency);
}

void SkyConnectImpl::setReplayFrequency(Frequency::Frequency frequency)
{
    d->replayFrequency = Frequency::toValue(frequency);
    d->replayIntervalMSec = static_cast<int>(1.0 / d->replayFrequency * 1000.0);
    d->timer.setInterval(d->replayIntervalMSec);
}

Frequency::Frequency SkyConnectImpl::getReplayFrequency() const
{
    return Frequency::fromValue(d->replayFrequency);
}

void SkyConnectImpl::setTimeScale(double timeScale)
{
    d->timeScale = timeScale;

    // Store the elapsed time measured with the previous scale...
    if (d->elapsedTimer.isValid()) {
        d->elapsedScaled = d->elapsedScaled + d->elapsedTimer.elapsed();
        // ... and restart timer
        d->elapsedTimer.start();
    }
}

double SkyConnectImpl::getTimeScale() const
{
    return d->timeScale;
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

    res = SimConnect_AddToDataDefinition(d->simConnectHandle, SkyConnectDataDefinition::AircraftInitialPosition, "Initial Position", nullptr, ::SIMCONNECT_DATATYPE_INITPOSITION);

    // Request an event when the simulation starts
    res = ::SimConnect_SubscribeToSystemEvent(d->simConnectHandle, SimStartEvent, "SimStart");

    // Events
    res = ::SimConnect_AddClientEventToNotificationGroup(d->simConnectHandle, InitGroup, InitEvent);

    // Client events
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, FreezeLatituteLongitude, "FREEZE_LATITUDE_LONGITUDE_SET");
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, FreezeAltitude, "FREEZE_ALTITUDE_SET");
    ::SimConnect_MapClientEventToSimEvent(d->simConnectHandle, FreezeAttitude, "FREEZE_ATTITUDE_SET");

}

void SkyConnectImpl::setupInitialPosition()
{
    const Position &position = d->aircraft.getPosition(0);
    if (position.isValid()) {
        // Set initial position
        SIMCONNECT_DATA_INITPOSITION initialPosition;

        initialPosition.Latitude = position.latitude;
        initialPosition.Longitude = position.longitude;
        initialPosition.Altitude = position.altitude;
        initialPosition.Pitch = position.pitch;
        initialPosition.Bank = position.bank;
        initialPosition.Heading = position.heading;
        initialPosition.OnGround = d->aircraft.getAircraftInfo().startOnGround ? 1 : 0;
        initialPosition.Airspeed = d->aircraft.getAircraftInfo().initialAirspeed;

        ::SimConnect_SetDataOnSimObject(d->simConnectHandle, SkyConnectDataDefinition::AircraftInitialPosition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(::SIMCONNECT_DATA_INITPOSITION), &initialPosition);
    } else {
        this->stopReplay();
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

void CALLBACK SkyConnectImpl::sampleDataCallback(SIMCONNECT_RECV* receivedData, DWORD cbData, void *context)
{
    Q_UNUSED(cbData);

    SkyConnectImpl *skyConnect = static_cast<SkyConnectImpl *>(context);
    DWORD objectID;
    SIMCONNECT_RECV_SIMOBJECT_DATA *objectData;
    SimConnectAircraftInfo *simConnectAircraftInfo;
    SimConnectPosition *simConnectPosition;

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
                {
                    objectID = objectData->dwObjectID;
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

            switch(objectData->dwRequestID)
            {
                case AircraftPositionRequest:
                {
                    if (!skyConnect->d->elapsedTimer.isValid()) {
                        // Start the elapsed timer with the arrival of the first sample data
                        qDebug("DATA CALLBACK: Elapsed timer started...");
                        skyConnect->d->currentTimestamp = 0;
                        skyConnect->d->elapsedTimer.start();
                    }
                    simConnectPosition = reinterpret_cast<::SimConnectPosition *>(&objectData->dwData);
                    Position position = simConnectPosition->toPosition();
                    position.timestamp = skyConnect->d->currentTimestamp;
                    qDebug("DATA CALLBACK: UPSERT data with timestamp: %lli", skyConnect->d->currentTimestamp);
                    skyConnect->d->aircraft.upsertPosition(std::move(position));

                    break;
                }

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
    if (d->elapsedTimer.isValid()) {
        d->currentTimestamp = d->elapsedScaled + static_cast<qint64>(d->elapsedTimer.elapsed() * d->timeScale);
    } else {
        d->elapsedScaled = 0;
        d->currentTimestamp = 0;
        d->elapsedTimer.start();
    }
    const Position &position = d->aircraft.getPosition(d->currentTimestamp);

    if (position.isValid()) {
        SimConnectPosition simConnectPosition;
        simConnectPosition.fromPosition(position);
        //qDebug("Replay: lon: %f lat: %f alt: %f pitch: %f bank: %f head: %f time: %lli",
        qDebug("%f, %f, %f, %f, %f, %f, %lli",
               simConnectPosition.longitude, simConnectPosition.latitude, simConnectPosition.altitude,
               simConnectPosition.pitch, simConnectPosition.bank, simConnectPosition.heading,
               d->currentTimestamp);
        ::SimConnect_SetDataOnSimObject(d->simConnectHandle, SkyConnectDataDefinition::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(SimConnectPosition), &simConnectPosition);
    } else {
        this->stopReplay();
    }
    emit playPositionChanged(d->currentTimestamp);
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
        qDebug("SAMPLE DATA: elapsed timer: %lli", d->currentTimestamp);
    } else {
        // First sample: request aircraft information...
        res = ::SimConnect_RequestDataOnSimObjectType(d->simConnectHandle, AircraftInfoRequest, SkyConnectDataDefinition::AircraftInfoDefinition, ::UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
    }

    ::SimConnect_CallDispatch(d->simConnectHandle, SkyConnectImpl::sampleDataCallback, this);
}
