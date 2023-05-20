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
#ifndef SQLITEAIRCRAFTDAO_H
#define SQLITEAIRCRAFTDAO_H

#include <memory>
#include <vector>
#include <cstdint>
#include <cstddef>

class QString;

#include "../AircraftDaoIntf.h"

class Aircraft;
struct AircraftInfo;
struct SQLiteAircraftDaoPrivate;

class SQLiteAircraftDao final : public AircraftDaoIntf
{
public:
    SQLiteAircraftDao(QString connectionName) noexcept;
    SQLiteAircraftDao(const SQLiteAircraftDao &rhs) = delete;
    SQLiteAircraftDao(SQLiteAircraftDao &&rhs) noexcept;
    SQLiteAircraftDao &operator=(const SQLiteAircraftDao &rhs) = delete;
    SQLiteAircraftDao &operator=(SQLiteAircraftDao &&rhs) noexcept;
    ~SQLiteAircraftDao() override;

    bool add(std::int64_t flightId, std::size_t sequenceNumber, Aircraft &aircraft) const noexcept override;
    bool exportAircraft(std::int64_t flightId, std::size_t sequenceNumber, const Aircraft &aircraft) const noexcept override;
    std::vector<Aircraft> getByFlightId(std::int64_t flightId, bool *ok = nullptr) const noexcept override;
    bool adjustAircraftSequenceNumbersByFlightId(std::int64_t flightId, std::size_t sequenceNumber) const noexcept override;
    bool deleteAllByFlightId(std::int64_t flightId) const noexcept override;
    bool deleteById(std::int64_t id) const noexcept override;
    std::vector<AircraftInfo> getAircraftInfosByFlightId(std::int64_t flightId, bool *ok = nullptr) const noexcept override;
    bool updateTimeOffset(std::int64_t id, std::int64_t timeOffset) const noexcept override;
    bool updateTailNumber(std::int64_t id, const QString &tailNumber) const noexcept override;

private:
    std::unique_ptr<SQLiteAircraftDaoPrivate> d;

    // Inserts the aircraft and returns the generated aircraft ID if successful; Const::InvalidId upon failure
    inline std::int64_t insertAircraft(std::int64_t flightId, std::size_t sequenceNumber, const Aircraft &aircraft) const noexcept;
    inline bool insertAircraftData(std::int64_t aircraftId, const Aircraft &aircraft) const noexcept;
};

#endif // SQLITEAIRCRAFTDAO_H
