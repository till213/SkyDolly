/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SIMCONNECTAI_H
#define SIMCONNECTAI_H

#include <memory>
#include <cstdint>

#include <windows.h>
#include <SimConnect.h>

class Flight;
class Aircraft;
class SimConnectAIPrivate;

class SimConnectAi
{
public:
    SimConnectAi(HANDLE simConnectHandle);
    SimConnectAi(const SimConnectAi &rhs) = delete;
    SimConnectAi(SimConnectAi &&rhs) = delete;
    SimConnectAi &operator=(const SimConnectAi &rhs) = delete;
    SimConnectAi &operator=(SimConnectAi &&rhs) = delete;
    ~SimConnectAi();

    void addObject(const Aircraft &aircraft, std::int64_t timestamp) noexcept;
    void removeByAircraftId(std::int64_t aircraftId) noexcept;
    void removeAllObjects() noexcept;

    void removeByObjectId(::SIMCONNECT_OBJECT_ID objectId) noexcept;

    /*!
     * Registers the \p objectId, as returned by the server via SimConnect, with the given \p requestId
     * as key. The registration succeeds if the request to create the given simulated object is still active.
     *
     * \param requestId
     *        the SimConnect request ID with which the simulated object creation has been originally requested
     * \param objectId
     *        the ID of the newly created simulation object
     * \return \c true if the registration succeeded; \c false if the original simulated object creation
     *         request has already been removed
     */
    bool registerObjectId(::SIMCONNECT_DATA_REQUEST_ID requestId, ::SIMCONNECT_OBJECT_ID objectId) noexcept;
    ::SIMCONNECT_OBJECT_ID getSimulatedObjectByAircraftId(std::int64_t aircraftId) const noexcept;

    static constexpr ::SIMCONNECT_OBJECT_ID InvalidObjectId = -1;

private:
    const std::unique_ptr<SimConnectAIPrivate> d;

    bool hasRequest(::SIMCONNECT_DATA_REQUEST_ID requestId) const noexcept;
};

#endif // SIMCONNECTAI_H
