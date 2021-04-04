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
#include <QString>
#include <QStringList>
#include <QStringBuilder>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlError>
#include <QDateTime>

#include "SqlMigrationStep.h"

class SqlMigrationStepPrivate
{
public:
    SqlMigrationStepPrivate()
    {}

    QString migrationId;
    QString description;
    int step;
    int stepCount;
    QString sqlStatement;
};

// PUBLIC

SqlMigrationStep::SqlMigrationStep()
    : d(std::make_unique<SqlMigrationStepPrivate>())
{
}

SqlMigrationStep::~SqlMigrationStep()
{
}

bool SqlMigrationStep::isValid() const noexcept
{
    return !d->migrationId.isNull();
}

bool SqlMigrationStep::parseTag(const QRegularExpressionMatch &tagMatch) noexcept
{
    QString tag = tagMatch.captured(1);
    qDebug("parseTag: tag: %s", qPrintable(tag));
    // Match the tag's content, e.g. id = 42, descn = "The description", step = 1
    // https://regex101.com/
    const QRegularExpression tagRegExp("([\\w]+)\\s*=\\s*[\"]*([\\w\\s\\-]+)[\"]*");


    QRegularExpressionMatchIterator it = tagRegExp.globalMatch(tag);
    while (it.hasNext()) {

        QRegularExpressionMatch match = it.next();
        qDebug("parseTag: values: %s %s", qPrintable(match.captured(1)), qPrintable(match.captured(2)));


        // TODO Set tag values
    }

    return true;
}

bool SqlMigrationStep::isApplied() const noexcept
{
    bool applied;
    // TODO Store common queries in private data
    QSqlQuery query;
    query.prepare("select count(1) from migr where id = :id and success = 1;");
    query.bindValue("id", d->migrationId);
    query.exec();
    if (query.next()) {
        applied = query.value(0).toInt() > 0;
    } else {
        applied = false;
    }
    return applied;
}

bool SqlMigrationStep::execute(const QString &sql) noexcept
{
    QString errorMessage;
    // Match SQL statements terminated with a semicolon
    const QRegularExpression sqlRegExp("([\\w\\s\\(\\),\\*=:$@#|\\/\\<\\>\\~!\\^\\-'\\+\\.]+);");

    // Note that DDL statements do not require transactions; but for
    // now we execute all queries within a transaction
    QSqlDatabase::database().transaction();

    QRegularExpressionMatchIterator it = sqlRegExp.globalMatch(sql);
    bool ok = true;
    while (ok && it.hasNext()) {

        QRegularExpressionMatch match = it.next();
        qDebug("execute: sql: %s", qPrintable(match.captured(1)));

        QSqlQuery query;
        query.prepare(match.captured(1).trimmed() % ";");
        ok = query.exec();
        if (!ok) {
            errorMessage = query.lastError().databaseText();
        }

    }
    if (ok) {

        QSqlQuery query;
        query.prepare("insert into migr values(:id, 1, :timestamp);");
        query.bindValue("id", d->migrationId);
        query.bindValue("timestamp", QDateTime::currentDateTime().toString());
        ok = query.exec();

        QSqlDatabase::database().commit();
    } else {
        QSqlDatabase::database().rollback();
        QSqlDatabase::database().transaction();
        QSqlQuery query;
        query.prepare("insert into migr values(:id, 0, :timestamp, :msg);");
        query.bindValue("id", d->migrationId);
        query.bindValue("timestamp", QDateTime::currentDateTime().toString());
        query.bindValue("msg", errorMessage);
        ok = query.exec();
    }
    return ok;
}

const QString &SqlMigrationStep::getMigrationId() const noexcept
{
    // TODO IMPLEMENT ME
    return std::move(QString());
}

const QString &SqlMigrationStep::getDescription() const noexcept
{
    // TODO IMPLEMENT ME
    return std::move(QString());
}

int SqlMigrationStep::getStep() const noexcept
{
    // TODO IMPLEMENT ME
    return 0;
}

int SqlMigrationStep::getStepCount() const noexcept
{
    // TODO IMPLEMENT ME
    return 0;
}

