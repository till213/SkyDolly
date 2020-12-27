#include <windows.h>
#include <strsafe.h>
#include <SimConnect.h>

#include <QByteArray>
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

#include "Aircraft.h"
#include "SkyConnect.h"

namespace {

    const char *ConnectionName = "SkyConnect";

    const DWORD UserAirplaneRadiusMeters = 0;
    // Sample the position data at 60 Hz
    const int IntervalMilliseconds = int(1.0 / 60.0 * 1000.0);

    struct SimConnectAircraftInfo
    {
        char title[256];
    };

    struct SimConnectPosition
    {
        double latitude;  // degrees
        double longitude; // degrees
        double altitude;  // feet

        double pitch;     // degrees
        double bank;      // degrees
        double heading;   // degrees
    };

    enum EventID{
        SimStartEvent
    };

    enum DataDefinitionID {
        AircraftInfoDefinition,
        AircraftPositionDefinition
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

class SimConnectPrivate
{
public:
    SimConnectPrivate()
        : simConnectHandler(nullptr),
          currentTimestamp(0)
    {
        timer.setInterval(IntervalMilliseconds);
    }

    HANDLE simConnectHandler;
    QTimer timer;
    Aircraft aircraft;
    qint64 currentTimestamp;
    QElapsedTimer elapsedTimer;
};


// PUBLIC

SkyConnect::SkyConnect(QObject *parent)
    : QObject(parent),
      d(new SimConnectPrivate())
{
    this->frenchConnection();
}

SkyConnect::~SkyConnect()
{
    this->close();
    delete d;
}

bool SkyConnect::open()
{
    HRESULT result = ::SimConnect_Open(&(d->simConnectHandler), ::ConnectionName, nullptr, 0, nullptr, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
    this->setupRequestData();
    return result == S_OK;
}

bool SkyConnect::close()
{
    HRESULT result;

    this->stopDataSample();
    if (d->simConnectHandler != nullptr) {
        result = ::SimConnect_Close(d->simConnectHandler);
        d->simConnectHandler = nullptr;
    } else {
        result = S_OK;
    }
    return result == S_OK;
}

bool SkyConnect::isConnected() const
{
    return d->simConnectHandler != nullptr;
}

void SkyConnect::startDataSample()
{
    HRESULT res;
    if (isConnected()) {
        d->timer.disconnect();
        connect(&(d->timer), &QTimer::timeout,
                this, &SkyConnect::sampleData);

        // Get aircraft position every simulated frame
        res = ::SimConnect_RequestDataOnSimObject(d->simConnectHandler, ::AircraftPositionRequest, ::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_SIM_FRAME);

        d->elapsedTimer.start();
        d->timer.start();
    }
}

void SkyConnect::stopDataSample()
{
    HRESULT res;
    // Get aircraft position every simulated frame
    res = ::SimConnect_RequestDataOnSimObject(d->simConnectHandler, ::AircraftPositionRequest, ::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_NEVER);
    d->timer.stop();
}

void SkyConnect::startReplay()
{
    if (isConnected()) {
        d->timer.disconnect();
        connect(&(d->timer), &QTimer::timeout,
                this, &SkyConnect::replay);

        d->elapsedTimer.start();
        d->timer.start();
    }
}

void SkyConnect::stopReplay()
{
    d->timer.stop();
}

Aircraft &SkyConnect::getAircraft()
{
    return d->aircraft;
}

const Aircraft &SkyConnect::getAircraft() const
{
    return d->aircraft;
}

// PRIVATE

void CALLBACK SkyConnect::sampleDataCallback(SIMCONNECT_RECV* receivedData, DWORD cbData, void *context)
{
    Q_UNUSED(cbData);

    SkyConnect *skyConnect = static_cast<SkyConnect *>(context);
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
                    position.latitude = simConnectPosition->latitude;
                    position.longitude = simConnectPosition->longitude;
                    position.altitude = simConnectPosition->altitude;
                    position.pitch = simConnectPosition->pitch;
                    position.bank = simConnectPosition->bank;
                    position.heading = simConnectPosition->heading;
                    position.timestamp = skyConnect->d->elapsedTimer.elapsed();

                    qDebug("position.timestamp: %lld", position.timestamp);

                    skyConnect->d->aircraft.appendPosition(std::move(position));
                    break;

                default:
                    break;
            }
            break;

        case SIMCONNECT_RECV_ID_QUIT:
            qDebug("SIMCONNECT_RECV_ID_QUIT");
            skyConnect->stopDataSample();
            skyConnect->stopReplay();
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

void SkyConnect::frenchConnection()
{
    connect(&(d->timer), &QTimer::timeout,
            this, &SkyConnect::sampleData);
}

void SkyConnect::setupRequestData()
{
    HRESULT res;

    // Set up the data definition, but do not yet do anything with it
    res = ::SimConnect_AddToDataDefinition(d->simConnectHandler, AircraftInfoDefinition, "title", nullptr, SIMCONNECT_DATATYPE_STRING256);

    res = ::SimConnect_AddToDataDefinition(d->simConnectHandler, AircraftPositionDefinition, "Plane Latitude", "degrees");
    res = ::SimConnect_AddToDataDefinition(d->simConnectHandler, AircraftPositionDefinition, "Plane Longitude", "degrees");
    res = ::SimConnect_AddToDataDefinition(d->simConnectHandler, AircraftPositionDefinition, "Plane Altitude", "feet");
    res = ::SimConnect_AddToDataDefinition(d->simConnectHandler, AircraftPositionDefinition, "Plane Pitch Degrees", "degrees");
    res = ::SimConnect_AddToDataDefinition(d->simConnectHandler, AircraftPositionDefinition, "Plane Bank Degrees", "degrees");
    res = ::SimConnect_AddToDataDefinition(d->simConnectHandler, AircraftPositionDefinition, "Plane Heading Degrees True", "degrees");

    // Request an event when the simulation starts
    res = ::SimConnect_SubscribeToSystemEvent(d->simConnectHandler, SimStartEvent, "SimStart");
}

// PRIVATE SLOTS

void SkyConnect::sampleData()
{
    HRESULT res;
    if (d->currentTimestamp == 0) {
        res = ::SimConnect_RequestDataOnSimObjectType(d->simConnectHandler, AircraftInfoRequest, AircraftInfoDefinition, UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
    }
    d->currentTimestamp = d->elapsedTimer.elapsed();
    ::SimConnect_CallDispatch(d->simConnectHandler, SkyConnect::sampleDataCallback, this);
}

void SkyConnect::replay()
{
    d->currentTimestamp = d->elapsedTimer.elapsed();
    const Position *position = d->aircraft.getPosition(d->currentTimestamp);

    if (position != nullptr) {
        SimConnectPosition simConnectPosition;
        simConnectPosition.latitude = position->latitude;
        simConnectPosition.longitude = position->longitude;
        simConnectPosition.altitude = position->altitude;
        simConnectPosition.pitch = position->pitch;
        simConnectPosition.bank = position->bank;
        simConnectPosition.heading = position->heading;
        ::SimConnect_SetDataOnSimObject(d->simConnectHandler, AircraftPositionDefinition, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(SimConnectPosition), &simConnectPosition);
    } else {
        this->stopReplay();
    }
}

