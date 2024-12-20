/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#ifndef AIRCRAFTDAOINTF_H
#define AIRCRAFTDAOINTF_H

#include <memory>
#include <vector>
#include <cstdint>
#include <cstddef>

class QString;
class QSqlDatabase;

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
     * Persists the \p aircraft. The \p id in \p aircraft is updated.
     *
     * \param flightId
     *        the flight the \p aircraft belongs to
     * \param sequenceNumber
     *        the sequence number of the aircraft
     * \param aircraft
     *        the aircraft to be persisted
     * \return \c true on success; \c false else
     */
    virtual bool add(std::int64_t flightId, std::size_t sequenceNumber, Aircraft &aircraft) const noexcept = 0;

    /*!
     * Exports the \p aircraft. While a new \p id for the exported Aircraft
     * is generated the \p id of the given \p aircraft (to be exported) is not updated.
     *
     * \param flightId
     *        the flight the \p aircraft belongs to
     * \param sequenceNumber
     *        the sequence number of the aircraft
     * \param aircraft
     *        the aircraft to be exported (persisted)
     * \return \c true on success; \c false else
     */
    virtual bool exportAircraft(std::int64_t flightId, std::size_t sequenceNumber, const Aircraft &aircraft) const noexcept = 0;

    virtual std::vector<Aircraft> getByFlightId(std::int64_t flightId, bool *ok = nullptr) const noexcept = 0;
    virtual bool adjustAircraftSequenceNumbersByFlightId(std::int64_t id, std::size_t sequenceNumber) const noexcept = 0;
    virtual bool deleteAllByFlightId(std::int64_t flightId) const noexcept = 0;
    virtual bool deleteById(std::int64_t id) const noexcept = 0;
    virtual std::vector<AircraftInfo> getAircraftInfosByFlightId(std::int64_t flightId, bool *ok = nullptr) const noexcept = 0;
    virtual bool updateTimeOffset(std::int64_t id, std::int64_t timeOffset) const noexcept = 0;
    virtual bool updateTailNumber(std::int64_t id, const QString &tailNumber) const noexcept = 0;
};

#endif // AIRCRAFTDAOINTF_H
