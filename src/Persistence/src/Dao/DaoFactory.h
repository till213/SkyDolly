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
#ifndef DAOFACTORY_H
#define DAOFACTORY_H

#include <memory>

#include "ScenarioDaoIntf.h"
#include "AircraftDaoIntf.h"
#include "PositionDaoIntf.h"
#include "EngineDaoIntf.h"
#include "PrimaryFlightControlDaoIntf.h"
#include "SecondaryFlightControlDaoIntf.h"
#include "HandleDaoIntf.h"
#include "LightDaoIntf.h"
#include "FlightPlanDaoIntf.h"
#include "DatabaseDaoIntf.h"

class DaoFactoryPrivate;

class DaoFactory
{
public:

    enum class DbType
    {
        SQLite = 0
    };

    DaoFactory(DbType dbType);
    ~DaoFactory();

    std::unique_ptr<DatabaseDaoIntf> createDatabaseDao() noexcept;
    std::unique_ptr<ScenarioDaoIntf> createScenarioDao() noexcept;
    std::unique_ptr<AircraftDaoIntf> createAircraftDao() noexcept;
    std::unique_ptr<PositionDaoIntf> createPositionDao() noexcept;
    std::unique_ptr<EngineDaoIntf> createEngineDao() noexcept;
    std::unique_ptr<PrimaryFlightControlDaoIntf> createPrimaryFlightControlDao() noexcept;
    std::unique_ptr<SecondaryFlightControlDaoIntf> createSecondaryFlightControlDao() noexcept;
    std::unique_ptr<HandleDaoIntf> createHandleDao() noexcept;
    std::unique_ptr<LightDaoIntf> createLightDao() noexcept;
    std::unique_ptr<FlightPlanDaoIntf> createFlightPlanDao() noexcept;

private:
    std::unique_ptr<DaoFactoryPrivate> d;
};

#endif // DAOFACTORY_H
