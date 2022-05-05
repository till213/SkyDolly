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
#ifndef SQLITEDATABASEDAO_H
#define SQLITEDATABASEDAO_H

#include <memory>

#include <QtGlobal>

class QString;
class QDateTime;

#include <Metadata.h>
#include "../DatabaseDaoIntf.h"

class DatabaseDaoPrivate;

class SQLiteDatabaseDao : public DatabaseDaoIntf
{
public:
    SQLiteDatabaseDao() noexcept;
    virtual ~SQLiteDatabaseDao() noexcept;

    virtual bool connectDb(const QString &logbookPath) noexcept override;
    virtual void disconnectDb() noexcept override;

    virtual bool migrate() noexcept override;
    virtual bool optimise() noexcept override;
    virtual bool backup(const QString &backupFilePath) noexcept override;
    virtual bool updateBackupPeriod(const QString &backupPeriodIntlId) noexcept override;
    virtual bool updateNextBackupDate(const QDateTime &date) noexcept override;
    virtual bool updateBackupDirectoryPath(const QString &backupDirectoryPath) noexcept override;

    virtual bool getMetadata(Metadata &metadata) const noexcept override;
    virtual bool getDatabaseVersion(Version &databaseVersion) const noexcept override;
    virtual bool getBackupDirectoryPath(QString &backupDirectoryPath) const noexcept override;

private:
    Q_DISABLE_COPY(SQLiteDatabaseDao)
    std::unique_ptr<DatabaseDaoPrivate> d;

    void disconnectSQLite() noexcept;
    bool createMigrationTable() noexcept;
};

#endif // SQLITEDATABASEDAO_H
