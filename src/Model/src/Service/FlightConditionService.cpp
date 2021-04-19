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
#include <QSqlDatabase>

#include <memory>
#include <utility>

#include "../Dao/DaoFactory.h"
#include "../Dao/FlightConditionDaoIntf.h"
#include "../FlightCondition.h"
#include "FlightConditionService.h"

class FlightConditionServicePrivate
{
public:
    FlightConditionServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          flightConditionDao(daoFactory->createFlightConditionDao())
    {
    }

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<FlightConditionDaoIntf> flightConditionDao;
};

// PUBLIC

FlightConditionService::FlightConditionService() noexcept
    : d(std::make_unique<FlightConditionServicePrivate>())
{}

FlightConditionService::~FlightConditionService() noexcept
{}

bool FlightConditionService::store(qint64 scenarioId, FlightCondition &flightCondition) noexcept
{
    QSqlDatabase::database().transaction();
    bool ok = d->flightConditionDao->addFlightCondition(scenarioId, flightCondition);
    if (ok) {
        QSqlDatabase::database().commit();
    } else {
        QSqlDatabase::database().rollback();
    }
    return ok;
}

FlightCondition FlightConditionService::restore(qint64 id) noexcept
{
    // TODO IMPLEMENT ME!!!
    return FlightCondition();
}
