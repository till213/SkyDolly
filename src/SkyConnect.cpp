#include <windows.h>
#include <strsafe.h>
#include <SimConnect.h>

#include <QObject>

#include "Aircraft.h"
#include "SkyConnect.h"

namespace {
  const char *ConnectionName = "SkyConnect";
  const int WM_USER_SIMCONNECT = 0x0402;

  const DWORD UserAirplaneRadiusMeters = 0;
  const int IntervalMilliseconds = 200;
}

struct Struct1
{
    char    title[256];
    double  kohlsmann;
    double  altitude;
    double  latitude;
    double  longitude;
};

enum EVENT_ID{
    EVENT_SIM_START
};

enum DATA_DEFINE_ID {
    DEFINITION_1
};

enum DATA_REQUEST_ID {
    REQUEST_1
};

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
    HRESULT result;
    HANDLE hEventHandle = nullptr;
    HWND hWnd = nullptr;

    result = SimConnect_Open(&(d->m_simConnectHandler), ::ConnectionName, hWnd, ::WM_USER_SIMCONNECT, hEventHandle, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
    qDebug("OPEN: m_simConnectHandler: %x", d->m_simConnectHandler);
    this->setupRequestData();
    return result == S_OK;
}

bool SkyConnect::close()
{
    HRESULT result;

    this->stopDataSample();
    if (d->m_simConnectHandler != nullptr) {
        result = SimConnect_Close(d->m_simConnectHandler);
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

void SkyConnect::testDataRequest()
{
    HRESULT hr;

    if (isConnected())
    {
        qDebug("\nConnected to Flight Simulator!");



        int i = 0;
        while (i < 100)
        {
            qDebug("Calling dispatcher on m_simConnectHandler: %x", d->m_simConnectHandler);
            SimConnect_CallDispatch(d->m_simConnectHandler, SkyConnect::MyDispatchProcRD, this);
            Sleep(1000);
            ++i;
        }
    }
}

void SkyConnect::startDataSample()
{
    qDebug("startDataSample: check wheter connected...");
    if (isConnected()) {
        qDebug("Started sampling.");
        d->m_timer.start();
    }

}

void SkyConnect::stopDataSample()
{
    qDebug("Stopped sampling.");
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
    HRESULT hr;

    qDebug("Callback called...");
    switch(pData->dwID)
    {
        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;

            switch(evt->uEventID)
            {
                case EVENT_SIM_START:

                    // Now the sim is running, request information on the user aircraft
                    hr = SimConnect_RequestDataOnSimObjectType(skyConnect->d->m_simConnectHandler, REQUEST_1, DEFINITION_1, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
                    qDebug("\nSkyConnect::MyDispatchProcRD: Simulation started, request sim object: m_simConnectHandler: %lx ", skyConnect->d->m_simConnectHandler);

                    break;

                default:
                   break;
            }
            break;
        }

        case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE:
        {
            SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA_BYTYPE*)pData;

            switch(pObjData->dwRequestID)
            {
                case REQUEST_1:
                {
                    DWORD ObjectID = pObjData->dwObjectID;
                    Struct1 *pS = (Struct1*)&pObjData->dwData;
                    if (SUCCEEDED(StringCbLengthA(&pS->title[0], sizeof(pS->title), NULL))) // security check
                    {
                        skyConnect->d->m_aircraft.setLatitude(pS->latitude);
                        skyConnect->d->m_aircraft.setLongitude(pS->longitude);
                        skyConnect->d->m_aircraft.setAltitude(pS->altitude);
                        skyConnect->aircraftChanged();
                        qDebug("\nObjectID=%lx  title=\"%s\"\nLat=%f  Lon=%f  Alt=%f  Kohlsman=%.2f", ObjectID, pS->title, pS->latitude, pS->longitude, pS->altitude, pS->kohlsmann );
                    }
                   break;
                }

                default:
                   break;
            }
            break;
        }


        case SIMCONNECT_RECV_ID_QUIT:
        {
            skyConnect->stopDataSample();
            break;
        }

        case SIMCONNECT_RECV_ID_OPEN:
        {
            hr = SimConnect_RequestDataOnSimObjectType(skyConnect->d->m_simConnectHandler, REQUEST_1, DEFINITION_1, 0, SIMCONNECT_SIMOBJECT_TYPE_USER);
            qDebug("\nSkyConnect::MyDispatchProcRD: Simulation already open, request sim object: m_simConnectHandler: %lx ", skyConnect->d->m_simConnectHandler);
            break;
        }

        default:
            qDebug("\nReceived:%lx", pData->dwID);
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
    HRESULT hr;

    // Set up the data definition, but do not yet do anything with it
    hr = SimConnect_AddToDataDefinition(d->m_simConnectHandler, DEFINITION_1, "title", NULL, SIMCONNECT_DATATYPE_STRING256);
    hr = SimConnect_AddToDataDefinition(d->m_simConnectHandler, DEFINITION_1, "Kohlsman setting hg", "inHg");
    hr = SimConnect_AddToDataDefinition(d->m_simConnectHandler, DEFINITION_1, "Plane Altitude", "feet");
    hr = SimConnect_AddToDataDefinition(d->m_simConnectHandler, DEFINITION_1, "Plane Latitude", "degrees");
    hr = SimConnect_AddToDataDefinition(d->m_simConnectHandler, DEFINITION_1, "Plane Longitude", "degrees");

    // Request an event when the simulation starts
    hr = SimConnect_SubscribeToSystemEvent(d->m_simConnectHandler, EVENT_SIM_START, "SimStart");
}

// PRIVATE SLOTS

void SkyConnect::sampleData()
{
  HRESULT hr;
  hr = SimConnect_RequestDataOnSimObjectType(d->m_simConnectHandler, REQUEST_1, DEFINITION_1, UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
  SimConnect_CallDispatch(d->m_simConnectHandler, SkyConnect::MyDispatchProcRD, this);
}

