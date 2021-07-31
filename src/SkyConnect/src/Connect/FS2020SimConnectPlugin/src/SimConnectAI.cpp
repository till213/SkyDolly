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
#include <vector>

#include <windows.h>
#include <SimConnect.h>

#include <QtGlobal>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/AircraftInfo.h"
#include "../../../../../Model/src/AircraftType.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "SimConnectType.h"
#include "SimConnectPosition.h"
#include "SimConnectAI.h"

class SimConnectAIPrivate
{
public:
    SimConnectAIPrivate(::HANDLE handle) noexcept
        : simConnectHandle(handle)
    {}

    ~SimConnectAIPrivate()
    {}

    HANDLE simConnectHandle;
};

// PUBLIC

SimConnectAI::SimConnectAI(::HANDLE simConnectHandle)
    : d(std::make_unique<SimConnectAIPrivate>(simConnectHandle))
{}

SimConnectAI::~SimConnectAI()
{
#ifdef DEBUG
    qDebug("SimConnectAI::~SimConnectAI: DELETED");
#endif
}

bool SimConnectAI::createSimulatedAircrafts(Flight &flight, qint64 timestamp, bool includingUserAircraft, std::unordered_map<::SIMCONNECT_DATA_REQUEST_ID, Aircraft *> &pendingAIAircraftCreationRequests) noexcept
{
    HRESULT result;
    bool ok;
    ::SIMCONNECT_DATA_INITPOSITION initialPosition;
    const Aircraft &userAircraft = flight.getUserAircraftConst();
    int i = 0;
    ok = true;
    for (auto &aircraft : flight) {
        const ::SIMCONNECT_DATA_REQUEST_ID requestId = Enum::toUnderlyingType(SimConnectType::DataRequest::AIObjectBase) + i;
        if (!includingUserAircraft && *aircraft == userAircraft) {
            aircraft->setSimulationObjectId(::SIMCONNECT_OBJECT_ID_USER);
#ifdef DEBUG
            qDebug("SimConnectAI::createSimulatedAircrafts: USER AIRCRAFT: request ID: %ld simulation object ID: %lld aircraft ID: %lld",
                   requestId, aircraft->getSimulationObjectId(), aircraft->getId());
#endif
        } else if (aircraft->getSimulationObjectId() == Aircraft::InvalidSimulationId) {
            pendingAIAircraftCreationRequests[requestId] = aircraft.get();
            const AircraftInfo aircraftInfo = aircraft->getAircraftInfoConst();
            const Position &position = aircraft->getPositionConst();
            const PositionData positioNData = position.interpolate(timestamp, TimeVariableData::Access::Seek);
            initialPosition = SimConnectPosition::toInitialPosition(positioNData, aircraftInfo.startOnGround, aircraftInfo.initialAirspeed);
            result = ::SimConnect_AICreateNonATCAircraft(d->simConnectHandle, aircraftInfo.aircraftType.type.toLocal8Bit(), aircraftInfo.tailNumber.toLocal8Bit(), initialPosition, requestId);
            ok = result == S_OK;
            if (ok) {
                aircraft->setSimulationObjectId(Aircraft::PendingSimulationId);
            } else {
                aircraft->setSimulationObjectId(Aircraft::InvalidSimulationId);
                break;
            }
#ifdef DEBUG
            if (*aircraft != userAircraft) {
                qDebug("SimConnectAI::createSimulatedAircrafts: created AI aircraft: request ID: %ld simulation object ID: %lld aircraft ID: %lld",
                       requestId, aircraft->getSimulationObjectId(), aircraft->getId());
            } else {
                qDebug("SimConnectAI::createSimulatedAircrafts: created AI aircraft for USER AIRCRAFT: request ID: %ld simulation object ID: %lld aircraft ID: %lld",
                       requestId, aircraft->getSimulationObjectId(), aircraft->getId());
            }
        } else {
            qDebug("SimConnectAI::createSimulatedAircrafts: PENDING AI aircraft: request ID: %ld aircraft ID: %lld",
                   requestId, aircraft->getId());
#endif
        }
        ++i;
    }
    return ok;
}

void SimConnectAI::destroySimulatedAircrafts(Flight &flight) noexcept
{
    for (auto &aircraft : flight) {
        destroySimulatedAircraft(*aircraft);
    }
}

void SimConnectAI::destroySimulatedAircraft(Aircraft &aircraft) noexcept
{
    const ::SIMCONNECT_OBJECT_ID objectId = aircraft.getSimulationObjectId();
    if (isValidAIObjectId(objectId)) {
#ifdef DEBUG
    qDebug("SimConnectAI::destroySimulatedAircrafts: destroying AI aircraft: simulation object ID: %lld aircraft ID: %lld",
           aircraft.getSimulationObjectId(), aircraft.getId());
#endif
        destroySimulatedObject(objectId);
    }
    aircraft.setSimulationObjectId(Aircraft::InvalidSimulationId);
}

void SimConnectAI::destroySimulatedObject(qint64 objectId) noexcept
{
    ::SimConnect_AIRemoveObject(d->simConnectHandle, objectId, Enum::toUnderlyingType(SimConnectType::DataRequest::AIRemoveObject));
}

// PRIVATE

bool SimConnectAI::isValidAIObjectId(qint64 objectId) const noexcept
{
    return objectId != ::SIMCONNECT_OBJECT_ID_USER && objectId != Aircraft::InvalidSimulationId && objectId != Aircraft::PendingSimulationId;
}
