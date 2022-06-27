/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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

#include "../LocationDaoIntf.h"

struct Location;

class SQLiteLocationDao : public LocationDaoIntf
{
public:
    SQLiteLocationDao();

    bool add(Location &location) noexcept override;
    bool get(std::int64_t id, Location &location) const noexcept override;
    bool deleteById(std::int64_t id) noexcept override;
    bool getAll(std::back_insert_iterator<std::vector<Location>> backInsertIterator) const noexcept override;
    bool getSelectedLocations(const LocationSelector &selector, std::back_insert_iterator<std::vector<Location>> backInsertIterator) const noexcept override;
};

#endif // SQLITELOCATIONDAO_H
