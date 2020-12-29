#include <windows.h>
#include <strsafe.h>
#include <SimConnect.h>

#include "Aircraft.h"
#include "Position.h"
#include "SimConnectAircraftInfo.h"
#include "SimConnectPosition.h"
#include "SkyConnectImpl.h"

namespace {

    const char *ConnectionName = "SkyDolly";
    const DWORD UserAirplaneRadiusMeters = 0;

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

// Sample the position data at 60 Hz
const int SkyConnectImpl::IntervalMilliseconds = int(1.0 / 60.0 * 1000.0);

// PUBLIC

SkyConnectImpl::SkyConnectImpl(QObject *parent)
    : QObject(parent),
      m_simConnectHandler(nullptr),
      m_currentTimestamp(0)
{
    m_timer.setInterval(IntervalMilliseconds);
    this->frenchConnection();
}

SkyConnectImpl::~SkyConnectImpl()
{
    this->close();
}

bool SkyConnectImpl::open()
{
    HRESULT result = ::SimConnect_Open(&(m_simConnectHandler), ::ConnectionName, nullptr, 0, nullptr, SIMCONNECT_OPEN_CONFIGINDEX_LOCAL);
    setupRequestData();
    return result == S_OK;
}

bool SkyConnectImpl::close()
{
    HRESULT result;

    this->stopDataSample();
    if (m_simConnectHandler != nullptr) {
        result = ::SimConnect_Close(m_simConnectHandler);
        m_simConnectHandler = nullptr;
    } else {
        result = S_OK;
    }
    return result == S_OK;
}

bool SkyConnectImpl::isConnected() const
{
    return m_simConnectHandler != nullptr;
}

void SkyConnectImpl::startDataSample()
{
    HRESULT res;
    if (isConnected()) {
        m_timer.disconnect();
        connect(&(m_timer), &QTimer::timeout,
                this, &SkyConnectImpl::sampleData);

        // Get aircraft position every simulated frame
        res = ::SimConnect_RequestDataOnSimObject(m_simConnectHandler, ::AircraftPositionRequest, ::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_SIM_FRAME, ::SIMCONNECT_DATA_REQUEST_FLAG_CHANGED);

        m_elapsedTimer.start();
        m_timer.start();
    }
}

void SkyConnectImpl::stopDataSample()
{
    HRESULT res;
    // Get aircraft position every simulated frame
    res = ::SimConnect_RequestDataOnSimObject(m_simConnectHandler, ::AircraftPositionRequest, ::AircraftPositionDefinition, ::SIMCONNECT_OBJECT_ID_USER, ::SIMCONNECT_PERIOD_NEVER);
    m_timer.stop();
}

void SkyConnectImpl::startReplay()
{
    if (isConnected()) {
        m_timer.disconnect();
        connect(&(m_timer), &QTimer::timeout,
                this, &SkyConnectImpl::replay);
        m_elapsedTimer.start();
        m_timer.start();
    }
}

void SkyConnectImpl::stopReplay()
{
    m_timer.stop();
}

Aircraft &SkyConnectImpl::getAircraft()
{
    return m_aircraft;
}

const Aircraft &SkyConnectImpl::getAircraft() const
{
    return m_aircraft;
}

// PRIVATE

void SkyConnectImpl::frenchConnection()
{
    connect(&(m_timer), &QTimer::timeout,
            this, &SkyConnectImpl::sampleData);
}

void SkyConnectImpl::setupRequestData()
{
    HRESULT res;

    // Set up the data definition, but do not yet do anything with it
    res = ::SimConnect_AddToDataDefinition(m_simConnectHandler, AircraftInfoDefinition, "title", nullptr, SIMCONNECT_DATATYPE_STRING256);

    res = ::SimConnect_AddToDataDefinition(m_simConnectHandler, AircraftPositionDefinition, "Plane Latitude", "degrees");
    res = ::SimConnect_AddToDataDefinition(m_simConnectHandler, AircraftPositionDefinition, "Plane Longitude", "degrees");
    res = ::SimConnect_AddToDataDefinition(m_simConnectHandler, AircraftPositionDefinition, "Plane Altitude", "feet");
    res = ::SimConnect_AddToDataDefinition(m_simConnectHandler, AircraftPositionDefinition, "Plane Pitch Degrees", "degrees");
    res = ::SimConnect_AddToDataDefinition(m_simConnectHandler, AircraftPositionDefinition, "Plane Bank Degrees", "degrees");
    res = ::SimConnect_AddToDataDefinition(m_simConnectHandler, AircraftPositionDefinition, "Plane Heading Degrees True", "degrees");

    // Request an event when the simulation starts
    res = ::SimConnect_SubscribeToSystemEvent(m_simConnectHandler, SimStartEvent, "SimStart");
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
                        skyConnect->m_aircraft.setName(name);
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
                    position.timestamp = skyConnect->m_elapsedTimer.elapsed();
                    skyConnect->m_aircraft.appendPosition(std::move(position));
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
    m_currentTimestamp = m_elapsedTimer.elapsed();
    const Position *position = m_aircraft.getPosition(m_currentTimestamp);

    if (position != nullptr) {
        SimConnectPosition simConnectPosition;
        simConnectPosition.latitude = position->latitude;
        simConnectPosition.longitude = position->longitude;
        simConnectPosition.altitude = position->altitude;
        simConnectPosition.pitch = position->pitch;
        simConnectPosition.bank = position->bank;
        simConnectPosition.heading = position->heading;
        ::SimConnect_SetDataOnSimObject(m_simConnectHandler, AircraftPositionDefinition, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_DATA_SET_FLAG_DEFAULT, 0, sizeof(SimConnectPosition), &simConnectPosition);
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
    if (m_currentTimestamp == 0) {
        res = ::SimConnect_RequestDataOnSimObjectType(m_simConnectHandler, AircraftInfoRequest, AircraftInfoDefinition, ::UserAirplaneRadiusMeters, SIMCONNECT_SIMOBJECT_TYPE_USER);
    }
    m_currentTimestamp = m_elapsedTimer.elapsed();
    ::SimConnect_CallDispatch(m_simConnectHandler, SkyConnectImpl::sampleDataCallback, this);
}
