#include <windows.h>
#include <strsafe.h>
#include <SimConnect.h>

#include <QByteArray>
#include <QObject>

#include "SimConnectDataDefinition.h"
#include "SimConnectPosition.h"
#include "Aircraft.h"
#include "SkyConnect.h"

namespace {
  const char *ConnectionName = "SkyConnect";

  const DWORD UserAirplaneRadiusMeters = 0;
  // Sample the position data at 60 Hz
  const int IntervalMilliseconds = int(1.0 / 60.0 * 1000.0);


    struct AircraftInfo
    {
        char title[256];
    };

    enum EventID{
        SimStartEvent
    };

    enum DataRequestID {
        AircraftInfoRequest,
        AircraftPositionRequest
    };

}

class SimConnectPrivate
{
public:
    SimConnectPrivate()
        : m_simConnectHandler(nullptr)
    {
        m_timer.setInterval(IntervalMilliseconds);
    }

    HANDLE m_simConnectHandler;
    QTimer m_timer;

    Aircraft m_aircraft;
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
    HRESULT result = ::SimConnect_Open(&(d->m_simConnectHandler), ::ConnectionName, nullptr, 0, nullptr, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
    this->setupRequestData();
    return result == S_OK;
}

bool SkyConnect::close()
{
    HRESULT result;

    this->stopDataSample();
    if (d->m_simConnectHandler != nullptr) {
        result = ::SimConnect_Close(d->m_simConnectHandler);
        d->m_simConnectHandler = nullptr;
    } else {
        result = S_OK;
    }
    return result == S_OK;
}

bool SkyConnect::isConnected() const
{
    return d->m_simConnectHandler != nullptr;
}

void SkyConnect::startDataSample()
{
    if (isConnected()) {
        d->m_timer.start();
    }
}

void SkyConnect::stopDataSample()
{
    d->m_timer.stop();
}

const Aircraft &SkyConnect::getAircraft() const
{
    return d->m_aircraft;
}

// PRIVATE

void CALLBACK SkyConnect::MyDispatchProcRD(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
    Q_UNUSED(cbData);

    SkyConnect *skyConnect = static_cast<SkyConnect *>(pContext);
    HRESULT res;
    DWORD objectID;
    SIMCONNECT_RECV_SIMOBJECT_DATA *objectData;
    AircraftInfo *aircraftInfo;
    QByteArray name;
    Position *position;

    switch(pData->dwID)
    {
        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;

            qDebug("SIMCONNECT_RECV_ID_EVENT");
            switch(evt->uEventID)
            {
                case SimStartEvent:

                    // Now the sim is running, request information on the user aircraft
                    res = ::SimConnect_RequestDataOnSimObjectType(skyConnect->d->m_simConnectHandler, AircraftInfoRequest, AircraftInfoDefinition, UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
                    qDebug("SimStartEvent");

                    break;

                default:
                   break;
            }
            break;
        }

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
            objectData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *>(pData);

            qDebug("SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE");
            switch(objectData->dwRequestID)
            {
                case AircraftInfoRequest:

                    objectID = objectData->dwObjectID;
                    aircraftInfo = reinterpret_cast<AircraftInfo *>(&objectData->dwData);
                    // security check
                    if (SUCCEEDED(StringCbLengthA(&aircraftInfo->title[0], sizeof(aircraftInfo->title), nullptr)))
                    {
                        qDebug("\nObjectID=%lx  title=\"%s\"", objectID, aircraftInfo->title );
                        name = aircraftInfo->title;
                        skyConnect->d->m_aircraft.setName(name);
                        skyConnect->aircraftChanged();
                    }
                    break;


                default:
                   break;
            }
            break;

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
            objectData = reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(pData);

            switch(objectData->dwRequestID)
            {
                case AircraftPositionRequest:
                    position = reinterpret_cast<Position *>(&objectData->dwData);
                    skyConnect->d->m_aircraft.setPosition(*position);
                    skyConnect->aircraftChanged();
                    break;

                default:
                    break;
            }
            break;

        case SIMCONNECT_RECV_ID_QUIT:
            qDebug("SIMCONNECT_RECV_ID_QUIT");
            skyConnect->stopDataSample();
            break;

        case SIMCONNECT_RECV_ID_OPEN:
            qDebug("SIMCONNECT_RECV_ID_OPEN");
            break;

        case SIMCONNECT_RECV_ID_NULL:
            qDebug("SIMCONNECT_RECV_ID_NULL");
            break;

        default:
            qDebug("Default: Received: %lx", pData->dwID);
            break;
    }
}

void SkyConnect::frenchConnection()
{
    connect(&(d->m_timer), &QTimer::timeout,
            this, &SkyConnect::sampleData);
}

void SkyConnect::setupRequestData()
{
    HRESULT res;

    // Set up the data definition, but do not yet do anything with it
    res = ::SimConnect_AddToDataDefinition(d->m_simConnectHandler, AircraftInfoDefinition, "title", NULL, SIMCONNECT_DATATYPE_STRING256);

    SimConnectPosition::addDataDefintion(d->m_simConnectHandler);

    // Request an event when the simulation starts
    res = ::SimConnect_SubscribeToSystemEvent(d->m_simConnectHandler, SimStartEvent, "SimStart");

    // Get aircraft position every simulated frame
    res = ::SimConnect_RequestDataOnSimObject(d->m_simConnectHandler, AircraftPositionRequest, AircraftPositionDefinition, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SIM_FRAME);

}

// PRIVATE SLOTS

void SkyConnect::sampleData()
{
    ::SimConnect_CallDispatch(d->m_simConnectHandler, SkyConnect::MyDispatchProcRD, this);
}

