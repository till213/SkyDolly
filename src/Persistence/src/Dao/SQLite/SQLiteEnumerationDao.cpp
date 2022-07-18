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
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QString>
#include <QVariant>
#include <QStringBuilder>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Name.h>
#include <Model/Enumeration.h>
#include "SQLiteEnumerationDao.h"

// PUBIC

SQLiteEnumerationDao::SQLiteEnumerationDao() noexcept
{}

SQLiteEnumerationDao::~SQLiteEnumerationDao() noexcept
{}

bool SQLiteEnumerationDao::get(Enumeration &enumeration) const noexcept
{
    const QString enumerationTableName = QStringLiteral("enum_") % Name::fromCamelCase(enumeration.getName());

    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare(
        "select e.id, e.sym_id, e.name "
        "from   " % enumerationTableName % " e "
        "order by e.id asc;"
    );

    const bool ok = query.exec();
    if (ok) {
        QSqlRecord record = query.record();
        const int idIdx = record.indexOf("id");
        const int symbolicIdIdx = record.indexOf("sym_id");
        const int nameIdx = record.indexOf("name");
        while (query.next()) {
            const std::int64_t id = query.value(idIdx).toLongLong();
            const QString symbolicId = query.value(symbolicIdIdx).toString();
            const QString name = query.value(nameIdx).toString();
            enumeration.addItem({id, symbolicId, name});
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteEnumerationDao::getByName: SQL error:" << query.lastError().databaseText() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    return ok;
}
