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
#ifndef LOCATIONDAOINTF_H
#define LOCATIONDAOINTF_H

#include <vector>
#include <cstdint>

struct LocationSelector;
struct Location;

class LocationDaoIntf
{
public:
    LocationDaoIntf() = default;
    LocationDaoIntf(const LocationDaoIntf &rhs) = delete;
    LocationDaoIntf(LocationDaoIntf &&rhs) = default;
    LocationDaoIntf &operator=(const LocationDaoIntf &rhs) = delete;
    LocationDaoIntf &operator=(LocationDaoIntf &&rhs) = default;
    virtual ~LocationDaoIntf() = default;

    virtual bool add(Location &location) const noexcept = 0;
    virtual bool exportLocation(const Location &location) const noexcept = 0;
    virtual bool update(const Location &location) const noexcept = 0;

    /*!
     * Gets the locations that are approximately within the given \p distance from the position
     * given by its \p latitude and \p longitude.
     *
     * \param latitude
     *        the latitude of the desired Location [degrees]
     * \param longitude
     *        the longitude of the desired Location [degrees]
     * \param distance
     *        the search distance from the given position [meters]
     * \param ok
     *        \c true if the query was successful; \c false else (an error occurred)
     * \return the locations that are within the given \p distance of the given \p latitude and \p longitude
     */
    virtual std::vector<Location> getByPosition(double latitude, double longitude, double distance = 0.0, bool *ok = nullptr) const noexcept = 0;
    virtual bool deleteById(std::int64_t id) const noexcept = 0;
    virtual std::vector<Location> getAll(bool *ok = nullptr) const noexcept = 0;
    virtual std::vector<Location> getSelectedLocations(const LocationSelector &selector, bool *ok = nullptr) const noexcept = 0;
};

#endif // LOCATIONDAOINTF_H
