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
#ifndef AIRCRAFTDAOINTF_H
#define AIRCRAFTDAOINTF_H

#include <memory>
#include <vector>
#include <cstdint>
#include <cstddef>

class QString;

class Aircraft;
struct AircraftInfo;

class AircraftDaoIntf
{
public:
    AircraftDaoIntf() = default;
    AircraftDaoIntf(const AircraftDaoIntf &rhs) = delete;
    AircraftDaoIntf(AircraftDaoIntf &&rhs) = default;
    AircraftDaoIntf &operator=(const AircraftDaoIntf &rhs) = delete;
    AircraftDaoIntf &operator=(AircraftDaoIntf &&rhs) = default;
    virtual ~AircraftDaoIntf() = default;

    /*!
     * Persists the \c aircraft. The \c id in \c aircraft is updated.
     *
     * \param flightId
     *        the flight the \c aircraft belongs to
     * \param sequenceNumber
     *        the sequence number of the aircraft
     * \param aircraft
     *        the aircraft to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool add(std::int64_t flightId, std::size_t sequenceNumber, Aircraft &aircraft) noexcept = 0;
    virtual std::vector<Aircraft> getByFlightId(std::int64_t flightId, bool *ok = nullptr) const noexcept = 0;
    virtual bool adjustAircraftSequenceNumbersByFlightId(std::int64_t id, std::size_t sequenceNumber) noexcept = 0;
    virtual bool deleteAllByFlightId(std::int64_t flightId) noexcept = 0;
    virtual bool deleteById(std::int64_t id) noexcept = 0;
    virtual std::vector<AircraftInfo> getAircraftInfosByFlightId(std::int64_t flightId, bool *ok = nullptr) const noexcept = 0;
    virtual bool updateTimeOffset(std::int64_t id, std::int64_t timeOffset) noexcept = 0;
    virtual bool updateTailNumber(std::int64_t id, const QString &tailNumber) noexcept = 0;
};

#endif // AIRCRAFTDAOINTF_H
