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
#ifndef LOGBOOKSERVICE_H
#define LOGBOOKSERVICE_H

#include <memory>
#include <forward_list>
#include <vector>

#include <QSqlDatabase>

#include <Kernel/Const.h>
#include <Model/FlightDate.h>
#include <Model/FlightSummary.h>
#include "../FlightSelector.h"
#include "../PersistenceLib.h"

struct LogbookServicePrivate;

class PERSISTENCE_API LogbookService final
{
public:
    LogbookService(QString connectionName = Const::DefaultConnectionName) noexcept;
    LogbookService(const LogbookService &rhs) = delete;
    LogbookService(LogbookService &&rhs) noexcept;
    LogbookService &operator=(const LogbookService &rhs) = delete;
    LogbookService &operator=(LogbookService &&rhs) noexcept;
    ~LogbookService();

    std::forward_list<FlightDate> getFlightDates(bool *ok = nullptr) const noexcept;
    std::vector<FlightSummary> getFlightSummaries(const FlightSelector &flightSelector, bool *ok = nullptr) const noexcept;
    std::vector<std::int64_t> getFlightIds(const FlightSelector &flightSelector = {}, bool *ok = nullptr) const noexcept;

private:
    std::unique_ptr<LogbookServicePrivate> d;
};

#endif // LOGBOOKSERVICE_H
