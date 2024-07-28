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
#ifndef LOCATIONSERVICE_H
#define LOCATIONSERVICE_H

#include <memory>
#include <vector>
#include <cstdint>

#include <QSqlDatabase>

#include <Kernel/Const.h>
#include <Model/Location.h>
#include "../PersistenceLib.h"

struct LocationSelector;
struct LocationServicePrivate;

class PERSISTENCE_API LocationService final
{
public:
    /*!
     * Defines how to deal with existing locations upon import.
     *
     * Implementation note: these values are peristed in the application settings.
     */
    enum struct Mode: std::uint8_t {
        First = 0,
        /*! The location to be imported is skipped; the existing position is left unmodified */
        Skip = First,
        /*! The existing position is updated with the values from the newly imported location */
        Update,
        /*! The newly imported location is unconditionally inserted: duplicate positions may result */
        Insert,
        Last = Insert
    };
    LocationService(QString connectionName = Const::DefaultConnectionName) noexcept;
    LocationService(const LocationService &rhs) = delete;
    LocationService(LocationService &&rhs) noexcept;
    LocationService &operator=(const LocationService &rhs) = delete;
    LocationService &operator=(LocationService &&rhs) noexcept;
    ~LocationService();

    bool store(Location &location) noexcept;
    bool exportLocation(const Location &location) noexcept;
    bool storeAll(std::vector<Location> &locations, Mode mode, double distanceKm) noexcept;

    /*!
     * Exports the \p locations, but does not emit any signal. The \p id of the locations is
     * left unchanged (possibly still Const#InvalidId).
     *
     * \param locations
     *        the locations that will be exported, typically into another logbook
     * \return \c true upon success; \c false else
     * \sa storeAll
     */
    bool exportAll(const std::vector<Location> &locations) noexcept;

    bool update(const Location &location) noexcept;
    bool deleteById(std::int64_t id) noexcept;
    std::vector<Location> getAll(bool *ok = nullptr) const noexcept;
    std::vector<Location> getSelectedLocations(const LocationSelector &locationSelector, bool *ok = nullptr) const noexcept;

private:
    std::unique_ptr<LocationServicePrivate> d;
};

#endif // LOCATIONSERVICE_H
