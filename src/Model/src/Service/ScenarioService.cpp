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
#include <utility>

#include "../Dao/DaoFactory.h"
#include "../Dao/ScenarioDaoIntf.h"
#include "TransactionCallableIntf.h"
#include "ScenarioService.h"

class ScenarioServicePrivate
{
public:
    ScenarioServicePrivate() noexcept
    {
    }
};

class ScenarioStore : public TransactionCallableIntf<TransactionCallable::Void>
{
public:

    ScenarioStore(Scenario &theScenario)
        : scenario(theScenario),
          daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          scenarioDao(daoFactory->createScenarioDao())
    {}

    virtual std::pair<TransactionCallable::Void, bool> execute() noexcept override
    {
        bool ok = scenarioDao->addScenario(scenario);
        return std::pair{nullptr, ok};
    }

private:
    Scenario &scenario;
    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<ScenarioDaoIntf> scenarioDao;
};

// PUBLIC

ScenarioService::ScenarioService() noexcept
    : d(std::make_unique<ScenarioServicePrivate>())
{}

ScenarioService::~ScenarioService() noexcept
{}

bool ScenarioService::store(Scenario &scenario) noexcept
{
    ScenarioStore scenarioStore(scenario);
    std::pair<TransactionCallable::Void, bool> result = scenarioStore.execute();
    return result.second;
}

Scenario ScenarioService::restore(qint64 id) noexcept
{
    // TODO IMPLEMENT ME!!!
    return Scenario();
}
