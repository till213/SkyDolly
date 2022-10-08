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
#ifndef SQLITELOGBOOKDAO_H
#define SQLITELOGBOOKDAO_H

#include <memory>
#include <forward_list>
#include <vector>

#include "../LogbookDaoIntf.h"

struct FlightSelector;
struct FlightDate;
struct FlightSummary;

class SQLiteLogbookDao : public LogbookDaoIntf
{
public:
    SQLiteLogbookDao() = default;
    SQLiteLogbookDao(const SQLiteLogbookDao &rhs) = delete;
    SQLiteLogbookDao(SQLiteLogbookDao &&rhs);
    SQLiteLogbookDao &operator=(const SQLiteLogbookDao &rhs) = delete;
    SQLiteLogbookDao &operator=(SQLiteLogbookDao &&rhs);
    ~SQLiteLogbookDao() override;

    bool getFlightDates(std::front_insert_iterator<std::forward_list<FlightDate>> frontInsertIterator) const noexcept override;
    std::vector<FlightSummary> getFlightSummaries(const FlightSelector &flightSelector, bool *ok) const noexcept override;
};

#endif // SQLITELOGBOOKDAO_H
