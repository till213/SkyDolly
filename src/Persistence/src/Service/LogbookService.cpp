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
#include <memory>
#include <forward_list>
#include <vector>

#include <QSqlDatabase>

#include "../Dao/FlightSelector.h"
#include "../Dao/DaoFactory.h"
#include "../Dao/LogbookDaoIntf.h"
#include "LogbookService.h"

class LogbookServicePrivate
{
public:
    LogbookServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          logbookDao(daoFactory->createLogbookDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<LogbookDaoIntf> logbookDao;
};

// PUBLIC

LogbookService::LogbookService(QObject *parent) noexcept
    : QObject(parent),
      d(std::make_unique<LogbookServicePrivate>())
{}

LogbookService::~LogbookService() noexcept
{
#ifdef DEBUG
    qDebug("LogbookService::~LogbookService: DELETED.");
#endif
}

std::forward_list<FlightDate> LogbookService::getFlightDates() const noexcept
{
    std::forward_list<FlightDate> flightDates;
    if (QSqlDatabase::database().transaction()) {
        flightDates = d->logbookDao->getFlightDates();
        QSqlDatabase::database().rollback();
    }
    return flightDates;
}

std::vector<FlightSummary> LogbookService::getFlightSummaries(const FlightSelector &flightSelector) const noexcept
{
    std::vector<FlightSummary> descriptions;
    if (QSqlDatabase::database().transaction()) {
        descriptions = d->logbookDao->getFlightSummaries(flightSelector);
        QSqlDatabase::database().rollback();
    }
    return descriptions;
}
