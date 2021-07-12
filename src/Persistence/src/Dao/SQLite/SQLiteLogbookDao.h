/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <vector>

#include <QtGlobal>

#include "../FlightSelector.h"
#include "../LogbookDaoIntf.h"

class FlightDate;
class FlightSummary;

class SQLiteLogbookDao : public LogbookDaoIntf
{
public:
    explicit SQLiteLogbookDao() noexcept;
    virtual ~SQLiteLogbookDao() noexcept;

    virtual std::forward_list<FlightDate> getFlightDates() const noexcept override;
    virtual std::vector<FlightSummary> getFlightSummaries(const FlightSelector &flightSelector) const noexcept override;
};

#endif // SQLITELOGBOOKDAO_H
