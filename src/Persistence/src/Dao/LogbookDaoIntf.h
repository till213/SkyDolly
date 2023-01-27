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
#ifndef LOGBOOKDAO_H
#define LOGBOOKDAO_H

#include <forward_list>
#include <vector>

class QString;
class QSqlDatabase;

class FlightSelector;
class Flight;
class FlightDate;
class FlightSummary;

class LogbookDaoIntf
{
public:
    LogbookDaoIntf() = default;
    LogbookDaoIntf(const LogbookDaoIntf &rhs) = delete;
    LogbookDaoIntf(LogbookDaoIntf &&rhs) = default;
    LogbookDaoIntf &operator=(const LogbookDaoIntf &rhs) = delete;
    LogbookDaoIntf &operator=(LogbookDaoIntf &&rhs) = default;
    virtual ~LogbookDaoIntf() = default;

    virtual std::forward_list<FlightDate> getFlightDates(QSqlDatabase &db, bool *ok = nullptr) const noexcept = 0;
    virtual std::vector<FlightSummary> getFlightSummaries(QSqlDatabase &db, const FlightSelector &flightSelector, bool *ok = nullptr) const noexcept = 0;
};

#endif // LOGBOOKDAO_H
