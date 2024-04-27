/**
 * Sky Dolly - The black sheep for your fFlightPlan recordings
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
#ifndef SQLITEWAYPOINTDAO_H
#define SQLITEWAYPOINTDAO_H

#include <memory>
#include <cstdint>

class QString;

#include <Model/Waypoint.h>
#include "../WaypointDaoIntf.h"

class FlightPlan;
struct SQLiteWaypointDaoPrivate;

class SQLiteWaypointDao final : public WaypointDaoIntf
{
public:
    SQLiteWaypointDao(QString connectionName) noexcept;
    SQLiteWaypointDao(const SQLiteWaypointDao &rhs) = delete;
    SQLiteWaypointDao(SQLiteWaypointDao &&rhs) noexcept;
    SQLiteWaypointDao &operator=(const SQLiteWaypointDao &rhs) = delete;
    SQLiteWaypointDao &operator=(SQLiteWaypointDao &&rhs) noexcept;
    ~SQLiteWaypointDao() override;

    bool add(std::int64_t aircraftId, const FlightPlan &flightPlan) const noexcept override;
    bool getByAircraftId(std::int64_t aircraftId, FlightPlan &flightPlan) const noexcept override;
    bool deleteByFlightId(std::int64_t flightId) const noexcept override;
    bool deleteByAircraftId(std::int64_t aircraftId) const noexcept override;

private:
    std::unique_ptr<SQLiteWaypointDaoPrivate> d;
};

#endif // SQLITEWAYPOINTDAO_H
