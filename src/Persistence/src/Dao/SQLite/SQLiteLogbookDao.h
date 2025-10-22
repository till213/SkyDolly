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
#ifndef SQLITELOGBOOKDAO_H
#define SQLITELOGBOOKDAO_H

#include <memory>
#include <forward_list>
#include <vector>

class QString;

#include <Model/FlightDate.h>
#include <Model/FlightSummary.h>
#include <FlightSelector.h>
#include "../LogbookDaoIntf.h"

struct SQLiteLogbookDaoPrivate;

class SQLiteLogbookDao final : public LogbookDaoIntf
{
public:
    SQLiteLogbookDao(QString connectionName) noexcept;
    SQLiteLogbookDao(const SQLiteLogbookDao &rhs) = delete;
    SQLiteLogbookDao(SQLiteLogbookDao &&rhs) noexcept;
    SQLiteLogbookDao &operator=(const SQLiteLogbookDao &rhs) = delete;
    SQLiteLogbookDao &operator=(SQLiteLogbookDao &&rhs) noexcept;
    ~SQLiteLogbookDao() override;

    std::forward_list<FlightDate> getFlightDates(bool *ok = nullptr) const noexcept override;
    std::vector<FlightSummary> getFlightSummaries(const FlightSelector &flightSelector, bool *ok = nullptr) const noexcept override;
    std::vector<std::int64_t> getFlightIds(const FlightSelector &flightSelector, bool *ok = nullptr) const noexcept override;

private:
    std::unique_ptr<SQLiteLogbookDaoPrivate> d;
};

#endif // SQLITELOGBOOKDAO_H
