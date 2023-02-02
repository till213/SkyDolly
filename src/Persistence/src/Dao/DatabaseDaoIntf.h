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
#ifndef DATABASEDAOINTF_H
#define DATABASEDAOINTF_H

#include <cstdint>

#include <QString>

class QDateTime;

#include <Kernel/Version.h>
#include <Kernel/Const.h>
#include "Metadata.h"

class DatabaseDaoIntf
{
public:
    DatabaseDaoIntf() = default;
    DatabaseDaoIntf(const DatabaseDaoIntf &rhs) = delete;
    DatabaseDaoIntf(DatabaseDaoIntf &&rhs) = default;
    DatabaseDaoIntf &operator=(const DatabaseDaoIntf &rhs) = delete;
    DatabaseDaoIntf &operator=(DatabaseDaoIntf &&rhs) = default;
    virtual ~DatabaseDaoIntf() = default;

    virtual bool connectDb(const QString &logbookPath) noexcept = 0;
    virtual void disconnectDb() noexcept = 0;
    virtual const QString &connectionName() const noexcept = 0;

    virtual bool migrate() noexcept = 0;
    virtual bool optimise() noexcept = 0;
    virtual bool backup(const QString &backupFilePath) noexcept= 0;
    virtual bool updateBackupPeriod(std::int64_t backupPeriodId) noexcept = 0;
    virtual bool updateNextBackupDate(const QDateTime &date) noexcept = 0;
    virtual bool updateBackupDirectoryPath(const QString &backupDirectoryPath) noexcept = 0;

    virtual Metadata getMetadata(bool *ok = nullptr) const noexcept = 0;
    virtual Version getDatabaseVersion(bool *ok = nullptr) const noexcept = 0;
    virtual QString getBackupDirectoryPath(bool *ok = nullptr) const noexcept = 0;
};

#endif // DATABASEDAOINTF_H
