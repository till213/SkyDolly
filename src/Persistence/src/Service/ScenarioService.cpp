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

#include "../../../Model/src/Scenario.h"
#include "../../../Model/src/Aircraft.h"
#include "../Dao/DaoFactory.h"
#include "../Dao/ScenarioDaoIntf.h"
#include "../Dao/AircraftDaoIntf.h"
#include "ScenarioService.h"

class ScenarioServicePrivate
{
public:
    ScenarioServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          scenarioDao(daoFactory->createScenarioDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<ScenarioDaoIntf> scenarioDao;
};

// PUBLIC

ScenarioService::ScenarioService() noexcept
    : d(std::make_unique<ScenarioServicePrivate>())
{}

ScenarioService::~ScenarioService() noexcept
{
#ifdef DEBUG
    qDebug("ScenarioService::~ScenarioService: DESTROYED.");
#endif
}

bool ScenarioService::store(Scenario &scenario) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        d->scenarioDao->addScenario(scenario);
        if (ok) {
            QSqlDatabase::database().commit();
            emit scenarioStored(scenario.getId());
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

bool ScenarioService::restore(qint64 id, Scenario &scenario) noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->scenarioDao->getScenarioById(id, scenario);
        if (ok) {
            emit scenarioRestored(scenario.getId());
        }
    }
    QSqlDatabase::database().rollback();
    return ok;
}

bool ScenarioService::deleteById(qint64 id)  noexcept
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->scenarioDao->deleteById(id);
        if (ok) {
            QSqlDatabase::database().commit();
        } else {
            QSqlDatabase::database().rollback();
        }
    }
    return ok;
}

QVector<ScenarioDescription> ScenarioService::getScenarioDescriptions() const noexcept
{
    QVector<ScenarioDescription> descriptions;
    if (QSqlDatabase::database().transaction()) {
        descriptions = d->scenarioDao->getScenarioDescriptions();
        QSqlDatabase::database().rollback();
    }
    return descriptions;
}
