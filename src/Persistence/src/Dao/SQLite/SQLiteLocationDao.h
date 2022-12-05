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
#ifndef SQLITELOCATIONDAO_H
#define SQLITELOCATIONDAO_H

#include <cstdint>
#include <vector>

class QSqlQuery;

#include "../LocationDaoIntf.h"

struct Location;

class SQLiteLocationDao : public LocationDaoIntf
{
public:
    SQLiteLocationDao() = default;
    SQLiteLocationDao(const SQLiteLocationDao &rhs) = delete;
    SQLiteLocationDao(SQLiteLocationDao &&rhs) noexcept;
    SQLiteLocationDao &operator=(const SQLiteLocationDao &rhs) = delete;
    SQLiteLocationDao &operator=(SQLiteLocationDao &&rhs) noexcept;
    ~SQLiteLocationDao() override;

    bool add(Location &location) noexcept override;
    bool update(const Location &location) noexcept override;
    std::vector<Location> getByPosition(double latitude, double longitude, double distance = 0.0, bool *ok = nullptr) const noexcept override;
    bool deleteById(std::int64_t id) noexcept override;
    std::vector<Location> getAll(bool *ok = nullptr) const noexcept override;
    std::vector<Location> getSelectedLocations(const LocationSelector &selector, bool *ok = nullptr) const noexcept override;

private:
    inline std::vector<Location> executeGetLocationQuery(QSqlQuery &query, bool *ok = nullptr) const noexcept;
};

#endif // SQLITELOCATIONDAO_H
