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
#ifndef SQLITEPRIMARYFLIGHTCONTROLDAO_H
#define SQLITEPRIMARYFLIGHTCONTROLDAO_H

#include <memory>
#include <vector>
#include <cstdint>

class QString;

#include "../PrimaryFlightControlDaoIntf.h"

struct PrimaryFlightControlData;
struct SQLitePrimaryFlightControlDaoPrivate;

class SQLitePrimaryFlightControlDao : public PrimaryFlightControlDaoIntf
{
public:
    SQLitePrimaryFlightControlDao(QString connectionName) noexcept;
    SQLitePrimaryFlightControlDao(const SQLitePrimaryFlightControlDao &rhs) = delete;
    SQLitePrimaryFlightControlDao(SQLitePrimaryFlightControlDao &&rhs) noexcept;
    SQLitePrimaryFlightControlDao &operator=(const SQLitePrimaryFlightControlDao &rhs) = delete;
    SQLitePrimaryFlightControlDao &operator=(SQLitePrimaryFlightControlDao &&rhs) noexcept;
    ~SQLitePrimaryFlightControlDao() override;

    bool add(std::int64_t aircraftId, const PrimaryFlightControlData &data) noexcept override;
    std::vector<PrimaryFlightControlData> getByAircraftId(std::int64_t aircraftId, bool *ok = nullptr) const noexcept override;
    bool deleteByFlightId(std::int64_t flightId) noexcept override;
    bool deleteByAircraftId(std::int64_t aircraftId) noexcept override;

private:
    std::unique_ptr<SQLitePrimaryFlightControlDaoPrivate> d;
};

#endif // SQLITEPRIMARYFLIGHTCONTROLDAO_H
