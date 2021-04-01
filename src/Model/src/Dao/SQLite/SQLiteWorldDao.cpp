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

#include <QSqlDatabase>
#include <QSqlQuery>

#include "../../../../Kernel/src/Settings.h"
#include "DbMigration.h"
#include "SQLiteWorldDao.h"

constexpr char DbName[] = "QSQLITE";

class WorldDaoPrivate
{
public:
    WorldDaoPrivate()
    {}

    QSqlDatabase db;
};

// PUBLIC

SQLiteWorldDao::SQLiteWorldDao() noexcept
    : d(std::make_unique<WorldDaoPrivate>())
{

}

SQLiteWorldDao::~SQLiteWorldDao() noexcept
{
    disconnectSQLite();
}

bool SQLiteWorldDao::connectDb() noexcept
{
    const QString &dbPath = Settings::getInstance().getDbPath();

    d->db = QSqlDatabase::addDatabase(DbName);
    d->db.setDatabaseName(dbPath);

    return d->db.open();
}

void SQLiteWorldDao::disconnectDb() noexcept
{
    disconnectSQLite();
}

bool SQLiteWorldDao::migrate() noexcept
{
    bool ok = createPatchTable();
    if (ok) {
        DbMigration dbMigration;
        ok = dbMigration.migrateExAnte();
        if (ok) {
            ok = dbMigration.migrateDdl();
        }
        if (ok) {
            ok = dbMigration.migrateExPost();
        }
    }
    return ok;
}

// PRIVATE

void SQLiteWorldDao::disconnectSQLite() noexcept
{
    d->db.close();
}

bool SQLiteWorldDao::createPatchTable() noexcept
{
    QSqlQuery query;
    query.prepare("create table if not exists patch("
    "id	integer integer primary key,"
    "patch_id text not null unique,"
    "success integer not null,"
    "timestamp text not null,"
    "msg text)");
    bool ok = query.exec();
    return ok;
}
