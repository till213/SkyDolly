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
#include <memory>

#include <QSqlDatabase>

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
{}

EnumerationService::EnumerationService(EnumerationService &&rhs) = default;
EnumerationService &EnumerationService::operator=(EnumerationService &&rhs) = default;
EnumerationService::~EnumerationService() = default;

Enumeration EnumerationService::getEnumerationByName(QString name, bool *ok)
{
    Enumeration enumeration;
    bool success = QSqlDatabase::database().transaction();
    if (success) {
        enumeration = d->enumerationDao->get(std::move(name), &success);
    }
    QSqlDatabase::database().rollback();
    if (ok != nullptr) {
        *ok = success;
    }
    return enumeration;
}
