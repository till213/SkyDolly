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
#include <memory>
#include <forward_list>
#include <vector>

#include <QSqlDatabase>

#include <Model/FlightDate.h>
#include <Model/FlightSummary.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/LogbookDaoIntf.h"
#include <FlightSelector.h>
#include <Service/LogbookService.h>

struct LogbookServicePrivate
{
    LogbookServicePrivate(QString connectionName) noexcept
        : connectionName(connectionName),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite, std::move(connectionName))),
          logbookDao(daoFactory->createLogbookDao())
    {}

    QString connectionName;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<LogbookDaoIntf> logbookDao;
};

// PUBLIC

LogbookService::LogbookService(QString connectionName) noexcept
    : d {std::make_unique<LogbookServicePrivate>(std::move(connectionName))}
{}

LogbookService::LogbookService(LogbookService &&rhs) noexcept = default;
LogbookService &LogbookService::operator=(LogbookService &&rhs) noexcept = default;
LogbookService::~LogbookService() = default;

std::forward_list<FlightDate> LogbookService::getFlightDates(bool *ok) const noexcept
{
    std::forward_list<FlightDate> flightDates;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    if (db.transaction()) {
        flightDates = d->logbookDao->getFlightDates(ok);
        db.rollback();
    }
    return flightDates;
}

std::vector<FlightSummary> LogbookService::getFlightSummaries(const FlightSelector &flightSelector, bool *ok) const noexcept
{
    std::vector<FlightSummary> descriptions;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    if (db.transaction()) {
        descriptions = d->logbookDao->getFlightSummaries(flightSelector, ok);
        db.rollback();
    }
    return descriptions;
}

std::vector<std::int64_t> LogbookService::getFlightIds(const FlightSelector &flightSelector, bool *ok) const noexcept
{
    std::vector<std::int64_t> flightIds;
    QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    if (db.transaction()) {
        flightIds = d->logbookDao->getFlightIds(flightSelector, ok);
        db.rollback();
    }
    return flightIds;
}
