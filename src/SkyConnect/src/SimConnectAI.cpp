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
#include <vector>

#include <windows.h>
#include <SimConnect.h>

#include <QtGlobal>

#include "../../Model/src/Flight.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/Position.h"
#include "../../Model/src/PositionData.h"

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

bool SimConnectAI::createSimulatedAircrafts(Flight &flight, std::unordered_map<::SIMCONNECT_DATA_REQUEST_ID, Aircraft *> &aircraftByRequestId, ::SIMCONNECT_DATA_REQUEST_ID baseRequestId) noexcept
{
    HRESULT result;
    bool ok;
    SIMCONNECT_DATA_INITPOSITION initialPosition;
    int i = 0;
    ok = true;
    for (auto &aircraft : flight) {

        const SIMCONNECT_DATA_REQUEST_ID requestId = baseRequestId + i;
        aircraftByRequestId[requestId] = aircraft.get();
        aircraft->setSimulationRequestId(requestId);
        if (aircraft->isUserAircraft()) {
            aircraft->setSimulationRequestId(requestId);
            aircraft->setSimulationObjectId(::SIMCONNECT_OBJECT_ID_USER);
            ++i;
        } else {
            const AircraftInfo aircraftInfo = aircraft->getAircraftInfoConst();
            initialPosition = SimConnectPosition::toInitialPosition(aircraft->getPositionConst().getFirst(), aircraftInfo.startOnGround, aircraftInfo.initialAirspeed);
            result = ::SimConnect_AICreateNonATCAircraft(d->simConnectHandle, aircraftInfo.type.toLatin1(), aircraftInfo.tailNumber.toLatin1(), initialPosition, requestId);
            aircraft->setSimulationObjectId(Aircraft::InvalidSimulationId);
            ok = result == S_OK;
            if (ok) {
                ++i;
            } else {
                break;
            }
        }
#ifdef DEBUG
        qDebug("SimConnectAI::createSimulatedAircrafts: created (AI) aircraft: request ID: %lld simulation object ID: %lld aircraft ID: %lld",
               aircraft->getSimulationRequestId(), aircraft->getSimulationObjectId(), aircraft->getId());
#endif
    }
    return ok;
}

void SimConnectAI::destroySimulatedAircrafts(Flight &flight) noexcept
{
    for (auto &aircraft : flight) {
        const ::SIMCONNECT_OBJECT_ID objectId = aircraft->getSimulationObjectId();
        if (objectId != ::SIMCONNECT_OBJECT_ID_USER) {
#ifdef DEBUG
        qDebug("SimConnectAI::destroySimulatedAircrafts: destroying AI aircraft: request ID: %lld simulation object ID: %lld aircraft ID: %lld",
               aircraft->getSimulationRequestId(), aircraft->getSimulationObjectId(), aircraft->getId());
#endif
            SimConnect_AIRemoveObject(d->simConnectHandle, objectId, aircraft->getSimulationRequestId());
        }
        aircraft->setSimulationRequestId(Aircraft::InvalidSimulationId);
        aircraft->setSimulationObjectId(Aircraft::InvalidSimulationId);
    }
}
