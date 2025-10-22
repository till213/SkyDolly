/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#ifndef SQLITESECONDARYFLIGHTCONTROLDAO_H
#define SQLITESECONDARYFLIGHTCONTROLDAO_H

#include <memory>
#include <vector>
#include <cstdint>

class QString;

#include "../SecondaryFlightControlDaoIntf.h"

struct SecondaryFlightControlData;
struct SQLiteSecondaryFlightControlDaoPrivate;

class SQLiteSecondaryFlightControlDao final : public SecondaryFlightControlDaoIntf
{
public:
    SQLiteSecondaryFlightControlDao(QString connectionName) noexcept;
    SQLiteSecondaryFlightControlDao(const SQLiteSecondaryFlightControlDao &rhs) = delete;
    SQLiteSecondaryFlightControlDao(SQLiteSecondaryFlightControlDao &&rhs) noexcept;
    SQLiteSecondaryFlightControlDao &operator=(const SQLiteSecondaryFlightControlDao &rhs) = delete;
    SQLiteSecondaryFlightControlDao &operator=(SQLiteSecondaryFlightControlDao &&rhs) noexcept;
    ~SQLiteSecondaryFlightControlDao() override;

    bool add(std::int64_t aircraftId, const SecondaryFlightControlData &data) const noexcept override;
    std::vector<SecondaryFlightControlData> getByAircraftId(std::int64_t aircraftId, bool *ok = nullptr) const noexcept override;
    bool deleteByFlightId(std::int64_t flightId) const noexcept override;
    bool deleteByAircraftId(std::int64_t aircraftId) const noexcept override;

private:
    std::unique_ptr<SQLiteSecondaryFlightControlDaoPrivate> d;
};

#endif // SQLITESECONDARYFLIGHTCONTROLDAO_H
