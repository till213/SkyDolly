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
#ifndef SQLITEFLIGHTDAO_H
#define SQLITEFLIGHTDAO_H

#include <memory>
#include <cstdint>

class QString;

#include "../FlightDaoIntf.h"

struct FlightData;
class SQLiteFlightDaoPrivate;

class SQLiteFlightDao final : public FlightDaoIntf
{
public:
    SQLiteFlightDao(QString connectionName) noexcept;
    SQLiteFlightDao(const SQLiteFlightDao &rhs) = delete;
    SQLiteFlightDao(SQLiteFlightDao &&rhs) noexcept;
    SQLiteFlightDao &operator=(const SQLiteFlightDao &rhs) = delete;
    SQLiteFlightDao &operator=(SQLiteFlightDao &&rhs) noexcept;
    ~SQLiteFlightDao() override;

    bool add(FlightData &flight) const noexcept override;
    bool exportFlightData(const FlightData &flightData) const noexcept override;
    bool get(std::int64_t id, FlightData &flightData) const noexcept override;
    bool deleteById(std::int64_t id) const noexcept override;
    bool updateTitle(std::int64_t id, const QString &title) const noexcept override;
    bool updateFlightNumber(std::int64_t id, const QString &flightNumber) const noexcept override;
    bool updateDescription(std::int64_t id, const QString &description) const noexcept override;
    bool updateUserAircraftIndex(std::int64_t id, int index) const noexcept override;

private:
    std::unique_ptr<SQLiteFlightDaoPrivate> d;

    // Inserts the flight and returns the generated aircraft ID if successful; Const::InvalidId upon failure
    inline std::int64_t insertFlight(const FlightData &flightData) const noexcept;
    inline bool addAircraft(std::int64_t flightId, FlightData &flightData) const noexcept;
    inline bool exportAircraft(std::int64_t flightId, const FlightData &flightData) const noexcept;
};

#endif // SQLITEFLIGHTDAO_H
