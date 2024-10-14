/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SQLITEDATABASEDAO_H
#define SQLITEDATABASEDAO_H

#include <memory>
#include <cstdint>

class QString;
class QDateTime;

#include <Kernel/Version.h>
#include <Kernel/Const.h>
#include <Connection.h>
#include <Migration.h>
#include "../DatabaseDaoIntf.h"
#include "Metadata.h"

struct DatabaseDaoPrivate;

class SQLiteDatabaseDao final : public DatabaseDaoIntf
{
public:
    SQLiteDatabaseDao(QString connectionName = Const::DefaultConnectionName) noexcept;
    SQLiteDatabaseDao(const SQLiteDatabaseDao &rhs) = delete;
    SQLiteDatabaseDao(SQLiteDatabaseDao &&rhs) noexcept;
    SQLiteDatabaseDao &operator=(const SQLiteDatabaseDao &rhs) = delete;
    SQLiteDatabaseDao &operator=(SQLiteDatabaseDao &&rhs) noexcept;
    ~SQLiteDatabaseDao() override;

    bool connectDb(const QString &logbookPath) noexcept override;
    void disconnectDb(Connection::Default connection) noexcept override;

    bool migrate(Migration::Milestones milestones = Migration::Milestone::All) const noexcept override;
    bool optimise() const noexcept override;
    bool backup(const QString &backupFilePath) const noexcept override;
    bool updateBackupPeriod(std::int64_t backupPeriodId) const noexcept override;
    bool updateNextBackupDate(const QDateTime &date) const noexcept override;
    bool updateBackupDirectoryPath(const QString &backupDirectoryPath) const noexcept override;

    Metadata getMetadata(bool *ok = nullptr) const noexcept override;
    Version getDatabaseVersion(bool *ok = nullptr) const noexcept override;
    QString getBackupDirectoryPath(bool *ok = nullptr) const noexcept override;

private:
    std::unique_ptr<DatabaseDaoPrivate> d;

    void disconnectSQLite(Connection::Default connection) const noexcept;
    bool createMigrationTable() const noexcept;
};

#endif // SQLITEDATABASEDAO_H
