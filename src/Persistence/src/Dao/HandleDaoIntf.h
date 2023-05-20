/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#ifndef HANDLEDAOINTF_H
#define HANDLEDAOINTF_H

#include <vector>
#include <cstdint>

struct AircraftHandleData;

class HandleDaoIntf
{
public:
    HandleDaoIntf() = default;
    HandleDaoIntf(const HandleDaoIntf &rhs) = delete;
    HandleDaoIntf(HandleDaoIntf &&rhs) = default;
    HandleDaoIntf &operator=(const HandleDaoIntf &rhs) = delete;
    HandleDaoIntf &operator=(HandleDaoIntf &&rhs) = default;
    virtual ~HandleDaoIntf() = default;

    /*!
     * Persists the \c data.
     *
     * \param aircraftId
     *        the aircraft the \c data belongs to
     * \param data
     *        the AircraftHandleData to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool add(std::int64_t aircraftId, const AircraftHandleData &data) const noexcept = 0;
    virtual std::vector<AircraftHandleData> getByAircraftId(std::int64_t aircraftId, bool *ok = nullptr) const noexcept = 0;
    virtual bool deleteByFlightId(std::int64_t flightId) const noexcept = 0;
    virtual bool deleteByAircraftId(std::int64_t aircraftId) const noexcept = 0;
};

#endif // HANDLEDAOINTF_H
