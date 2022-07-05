/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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

#include <QSqlDatabase>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/Enumeration.h>
#include "../Dao/DaoFactory.h"
#include "../Dao/EnumerationDaoIntf.h"
#include <Service/EnumerationService.h>

struct EnumerationServicePrivate
{
    EnumerationServicePrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          enumerationDao(daoFactory->createEnumerationDao())
    {}

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<EnumerationDaoIntf> enumerationDao;
};

// PUBLIC

EnumerationService::EnumerationService() noexcept
    : d(std::make_unique<EnumerationServicePrivate>())
{
#ifdef DEBUG
    qDebug() << "EnumerationService::EnumerationService: CREATED.";
#endif
}

EnumerationService::~EnumerationService() noexcept
{
#ifdef DEBUG
    qDebug() << "EnumerationService::~EnumerationService: DELETED.";
#endif
}

bool EnumerationService::getEnumerationByName(Enumeration &enumeration)
{
    bool ok = QSqlDatabase::database().transaction();
    if (ok) {
        ok = d->enumerationDao->get(enumeration);
    }
    QSqlDatabase::database().rollback();
    return ok;
}
