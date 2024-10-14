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
#ifndef SQLITELOCATIONDAO_H
#define SQLITELOCATIONDAO_H

#include <memory>
#include <cstdint>
#include <vector>

class QString;
class QSqlQuery;

#include "../LocationDaoIntf.h"

struct Location;
struct SQLiteLocationDaoPrivate;

class SQLiteLocationDao final : public LocationDaoIntf
{
public:
    SQLiteLocationDao(QString connectionName) noexcept;
    SQLiteLocationDao(const SQLiteLocationDao &rhs) = delete;
    SQLiteLocationDao(SQLiteLocationDao &&rhs) noexcept;
    SQLiteLocationDao &operator=(const SQLiteLocationDao &rhs) = delete;
    SQLiteLocationDao &operator=(SQLiteLocationDao &&rhs) noexcept;
    ~SQLiteLocationDao() override;

    bool add(Location &location) const noexcept override;
    bool exportLocation(const Location &location) const noexcept override;
    bool update(const Location &location) const noexcept override;

    /*!
     * Returns all positions that are within \p distanceKm [kilometers] from position
     * \p latitude and \p longitude away.
     *
     * The distance calculation uses a simple but fast formula with an error rate less
     * than 1% for distances <= 5 kilometers, even at the poles.
     *
     * Also refer to: https://jonisalonen.com/2014/computing-distance-between-coordinates-can-be-simple-and-fast/
     *
     * \param latitude
     *        the latitude of the source location [degrees]
     * \param longitude
     *        the longitude of the source location [degrees]
     * \param distanceKm
     *        the distance [kilometers]
     * \param ok
     *        set to \c true when successful; \c false else (database error)
     * \return the locations that are within the given \p distanceKm from the source location (\c latitude, \p longitude)
     */
    std::vector<Location> getByPosition(double latitude, double longitude, double distanceKm = 0.0, bool *ok = nullptr) const noexcept override;
    bool deleteById(std::int64_t id) const noexcept override;
    std::vector<Location> getAll(bool *ok = nullptr) const noexcept override;
    std::vector<Location> getSelectedLocations(const LocationSelector &selector, bool *ok = nullptr) const noexcept override;

private:
    std::unique_ptr<SQLiteLocationDaoPrivate> d;

    inline std::vector<Location> executeGetLocationQuery(QSqlQuery &query, bool *ok = nullptr) const noexcept;
    std::int64_t insert(const Location &location) const noexcept;
};

#endif // SQLITELOCATIONDAO_H
