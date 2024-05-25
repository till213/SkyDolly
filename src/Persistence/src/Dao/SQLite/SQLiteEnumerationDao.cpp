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
#include <utility>

#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>
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

struct SQLiteEnumerationDaoPrivate
{
    SQLiteEnumerationDaoPrivate(QString connectionName) noexcept
        : connectionName {std::move(connectionName)}
    {}

    QString connectionName;
};

// PUBLIC

SQLiteEnumerationDao::SQLiteEnumerationDao(QString connectionName) noexcept
    : d {std::make_unique<SQLiteEnumerationDaoPrivate>(std::move(connectionName))}
{}

SQLiteEnumerationDao::SQLiteEnumerationDao(SQLiteEnumerationDao &&rhs) noexcept = default;
SQLiteEnumerationDao &SQLiteEnumerationDao::operator=(SQLiteEnumerationDao &&rhs) noexcept = default;
SQLiteEnumerationDao::~SQLiteEnumerationDao() = default;

Enumeration SQLiteEnumerationDao::get(const QString &name, Enumeration::Order order, bool *ok) const noexcept
{
    Enumeration enumeration {name};
    const QString enumerationTableName = QStringLiteral("enum_") % Name::fromCamelCase(enumeration.getName());

    const QSqlDatabase db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query {db};
    query.setForwardOnly(true);

    QString orderColumn;
    switch (order) {
    case Enumeration::Order::Id:
        orderColumn = QStringLiteral("id");
        break;
    case Enumeration::Order::SymId:
        orderColumn = QStringLiteral("sym_id");
        break;
    case Enumeration::Order::Name:
        orderColumn = QStringLiteral("name");
        break;
    }

    query.prepare(
        "select e.id, e.sym_id, e.name "
        "from   " % enumerationTableName % " e "
        "order by e." % orderColumn % " asc;"
    );

    const bool success = query.exec();
    if (success) {
        QSqlRecord record = query.record();
        const int idIdx = record.indexOf(QStringLiteral("id"));
        const int symIdIdx = record.indexOf(QStringLiteral("sym_id"));
        const int nameIdx = record.indexOf(QStringLiteral("name"));
        while (query.next()) {
            const std::int64_t id = query.value(idIdx).toLongLong();
            const QString symId = query.value(symIdIdx).toString();
            const QString itemName = query.value(nameIdx).toString();
            enumeration.addItem({id, symId, itemName});
        }
#ifdef DEBUG
    } else {
        qDebug() << "SQLiteEnumerationDao::getByName: SQL error:" << query.lastError().text() << "- error code:" << query.lastError().nativeErrorCode();
#endif
    }

    if (ok != nullptr) {
        *ok = success;
    }
    return enumeration;
}
