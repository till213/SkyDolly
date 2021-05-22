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

bool SimConnectAI::createSimulatedAircrafts(const std::vector<const Aircraft *> &aircrafts, ::SIMCONNECT_DATA_REQUEST_ID baseRequestId) noexcept
{
    HRESULT result;
    bool ok;
    SIMCONNECT_DATA_INITPOSITION initialPosition;
    int i = 0;
    ok = true;
    for (const Aircraft *aircraft: aircrafts) {

        const AircraftInfo aircraftInfo = aircraft->getAircraftInfoConst();
        initialPosition = SimConnectPosition::toInitialPosition(aircraft->getPositionConst().getFirst(), aircraftInfo.startOnGround, aircraftInfo.initialAirspeed);
        result = ::SimConnect_AICreateNonATCAircraft(d->simConnectHandle, aircraftInfo.type.toLatin1(), aircraftInfo.tailNumber.toLatin1(), initialPosition, baseRequestId + i);
        ok = result == S_OK;
        if (ok) {
            ++i;
        } else {
            break;
        }

    }
    return ok;
}

void SimConnectAI::destroySimulatedAircrafts(const std::vector<::SIMCONNECT_OBJECT_ID> &objectIDs, ::SIMCONNECT_DATA_REQUEST_ID baseRequestId) noexcept
{
    int i = 0;
    for (SIMCONNECT_OBJECT_ID objectID : objectIDs) {
        SimConnect_AIRemoveObject(d->simConnectHandle, objectID, baseRequestId + i);
        ++i;
    }
}
