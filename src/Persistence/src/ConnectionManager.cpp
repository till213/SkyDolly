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

#include <QString>

#include "../../Kernel/src/Settings.h"
#include "Metadata.h"
#include "Dao/DaoFactory.h"
#include "Dao/DatabaseDaoIntf.h"
#include "ConnectionManager.h"

class ConnectionManagerPrivate
{
public:

    std::unique_ptr<DaoFactory> daoFactory;
    std::unique_ptr<DatabaseDaoIntf> databaseDao;
    QString libraryPath;
    bool connected;

    static ConnectionManager *instance;

    ConnectionManagerPrivate() noexcept
        : daoFactory(std::make_unique<DaoFactory>(DaoFactory::DbType::SQLite)),
          databaseDao(daoFactory->createDatabaseDao()),
          connected(false)
    {}
};

ConnectionManager *ConnectionManagerPrivate::instance = nullptr;

// PUBLIC

ConnectionManager &ConnectionManager::getInstance() noexcept
{
    if (ConnectionManagerPrivate::instance == nullptr) {
        ConnectionManagerPrivate::instance = new ConnectionManager();
    }
    return *ConnectionManagerPrivate::instance;
}

void ConnectionManager::destroyInstance() noexcept
{
    if (ConnectionManagerPrivate::instance != nullptr) {
        delete ConnectionManagerPrivate::instance;
        ConnectionManagerPrivate::instance = nullptr;
    }
}

bool ConnectionManager::connectDb(const QString &libraryPath) noexcept
{
    if (d->libraryPath != libraryPath) {
        d->connected = d->databaseDao->connectDb(libraryPath);
        d->libraryPath = libraryPath;
        emit connectionChanged(d->connected);
    }
    return d->connected;
}

void ConnectionManager::disconnectDb() noexcept
{
    d->databaseDao->disconnectDb();
    d->connected = false;
    emit connectionChanged(d->connected);
}

bool ConnectionManager::isConnected() const noexcept
{
    return d->connected;
}

const QString &ConnectionManager::getLibraryPath() const noexcept
{
    return d->libraryPath;
}

bool ConnectionManager::migrate() noexcept
{
    return d->databaseDao->migrate();
}

bool ConnectionManager::optimise() noexcept
{
    return d->databaseDao->optimise();
}

bool ConnectionManager::backup(const QString &backupLibraryPath) noexcept
{
    return d->databaseDao->backup(backupLibraryPath);
}

bool ConnectionManager::getMetadata(Metadata &metadata) noexcept
{
    return d->databaseDao->getMetadata(metadata);
}

// PROTECTED

ConnectionManager::~ConnectionManager() noexcept
{
}

// PRIVATE

ConnectionManager::ConnectionManager() noexcept
    : d(std::make_unique<ConnectionManagerPrivate>())
{
}

