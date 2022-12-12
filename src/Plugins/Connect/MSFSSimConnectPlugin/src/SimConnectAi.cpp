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
#include <cstdint>
#include <unordered_map>
#include <algorithm>

#include <QDebug>

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include <Model/Aircraft.h>
#include <Model/AircraftInfo.h>
#include <Model/AircraftType.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include "SimConnectType.h"
#include "SimConnectPositionRequest.h"
#include "SimConnectAi.h"

using RequestByAircraftId = std::unordered_map<std::int64_t, ::SIMCONNECT_DATA_REQUEST_ID>;
using RequestByAircraftValueType = RequestByAircraftId::value_type;
using SimulatedObjectByRequestId = std::unordered_map<::SIMCONNECT_DATA_REQUEST_ID, std::int64_t>;

class SimConnectAIPrivate
{
public:
    SimConnectAIPrivate(::HANDLE handle) noexcept
        : simConnectHandle(handle)
    {}

    ::HANDLE simConnectHandle;
    // Key: aircraft ID - value: SimConnect request ID
    RequestByAircraftId requestByAircraftId;
    // Key: SimConnect request ID - value: SimConnect object ID
    SimulatedObjectByRequestId simulatedObjectByRequestId;
    ::SIMCONNECT_DATA_REQUEST_ID lastAiCreateRequestId {0};
};

// PUBLIC

SimConnectAi::SimConnectAi(::HANDLE simConnectHandle)
    : d(std::make_unique<SimConnectAIPrivate>(simConnectHandle))
{
#ifdef DEBUG
    qDebug() << "SimConnectAI::SimConnectAI: CREATED";
#endif
}

SimConnectAi::~SimConnectAi()
{
#ifdef DEBUG
    qDebug() << "SimConnectAI::~SimConnectAI: DELETED";
#endif
}

void SimConnectAi::addObject(const Aircraft &aircraft, std::int64_t timestamp) noexcept
{
    // Check if the newly added aircraft has any recording yet
    // (otherwise it is the new user aircraft being added for a new recording)
    if (aircraft.getId() != Const::InvalidId) {
        const AircraftInfo aircraftInfo = aircraft.getAircraftInfo();
        Position &position = aircraft.getPosition();
        const PositionData &positioNData = position.interpolate(timestamp, TimeVariableData::Access::Seek);
        const ::SIMCONNECT_DATA_INITPOSITION initialPosition = SimConnectPositionRequest::toInitialPosition(positioNData, aircraftInfo.startOnGround, aircraftInfo.initialAirspeed);

        const ::SIMCONNECT_DATA_REQUEST_ID requestId = Enum::underly(SimConnectType::DataRequest::AiObjectBase) + d->lastAiCreateRequestId;
        HRESULT result = ::SimConnect_AICreateNonATCAircraft(d->simConnectHandle, aircraftInfo.aircraftType.type.toLocal8Bit(), aircraftInfo.tailNumber.toLocal8Bit(), initialPosition, requestId);
        if (result == S_OK) {
            d->requestByAircraftId[aircraft.getId()] = requestId;
            ++d->lastAiCreateRequestId;
#ifdef DEBUG
        qDebug() << "SimConnectAi::addObject: pending CreateNonATCAircraft request:" << requestId << "for aircraft ID:" << aircraft.getId();
#endif
        }
    }
#ifdef DEBUG
    else {
        qDebug() << "SimConnectAi::addObject: ignoring user aircraft, ID:" << aircraft.getId();
    }
#endif
}

void SimConnectAi::removeByAircraftId(std::int64_t aircraftId) noexcept
{
    const auto it = d->requestByAircraftId.find(aircraftId);
    if (it != d->requestByAircraftId.end()) {
        const ::SIMCONNECT_DATA_REQUEST_ID requestId = it->second;
        const auto it2 = d->simulatedObjectByRequestId.find(requestId);
        if (it2 != d->simulatedObjectByRequestId.end()) {
            const ::SIMCONNECT_OBJECT_ID objectId = it2->second;
            removeByObjectId(objectId);
#ifdef DEBUG
        qDebug() << "SimConnectAi::removeByAircraftId: removing simulation object:" << objectId << "for aircraft ID:" << aircraftId;
#endif
            d->simulatedObjectByRequestId.erase(it2);
            d->requestByAircraftId.erase(it);
        }
    }
}

void SimConnectAi::removeAllObjects() noexcept {
    for (const auto it : d->simulatedObjectByRequestId) {
        const ::SIMCONNECT_OBJECT_ID objectId = it.second;
        removeByObjectId(objectId);
#ifdef DEBUG
        qDebug() << "SimConnectAi::removeAllObjects: removing simulation object:" << objectId << "(all objects)";
#endif
    }
    d->simulatedObjectByRequestId.clear();
    d->requestByAircraftId.clear();
}

void SimConnectAi::removeByObjectId(::SIMCONNECT_OBJECT_ID objectId) noexcept
{
    ::SimConnect_AIRemoveObject(d->simConnectHandle, objectId, Enum::underly(SimConnectType::DataRequest::AiRemoveObject));
}

bool SimConnectAi::registerObjectId(::SIMCONNECT_DATA_REQUEST_ID requestId, ::SIMCONNECT_OBJECT_ID objectId) noexcept
{
    bool ok {false};
    if (hasRequest(requestId)) {
        d->simulatedObjectByRequestId[requestId] = objectId;
#ifdef DEBUG
        qDebug() << "SimConnectAi::registerObjectId: registering simulation object ID:" << objectId << "for original request ID:" << requestId;
#endif
        ok = true;
    }
#ifdef DEBUG
    else {
        qDebug() << "SimConnectAi::registerObjectId: original request ID:" << requestId << "has already been discarded -> remove simulated object again, ID:" << objectId;
    }
#endif
    return ok;
}

::SIMCONNECT_OBJECT_ID SimConnectAi::getSimulatedObjectByAircraftId(std::int64_t aircraftId) const noexcept
{
    ::SIMCONNECT_OBJECT_ID objectId {InvalidObjectId};
    const auto it =  d->requestByAircraftId.find(aircraftId);
    if (it != d->requestByAircraftId.end()) {
        const ::SIMCONNECT_DATA_REQUEST_ID requestId = it->second;
        const auto it2 = d->simulatedObjectByRequestId.find(requestId);
        if (it2 != d->simulatedObjectByRequestId.end()) {
             objectId = it2->second;
        }
    }
    return objectId;
}

// PRIVATE

bool SimConnectAi::hasRequest(::SIMCONNECT_DATA_REQUEST_ID requestId) const noexcept
{
    return std::find_if(d->requestByAircraftId.begin(),
                        d->requestByAircraftId.end(),
                        [&requestId](RequestByAircraftValueType &valueType)
    {
        return valueType.second == requestId;
    }) != d->requestByAircraftId.end();
}
