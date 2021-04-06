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
    bool applied;
    QString errorMessage;
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
    bool ok;
    QString tag = tagMatch.captured(1);
    qDebug("parseTag: tag: %s", qPrintable(tag));
    // Match the tag's content, e.g. id = 42, descn = "The description", step = 1
    // https://regex101.com/
    const QRegularExpression tagRegExp("([\\w]+)\\s*=\\s*[\"]*([\\w\\s\\-]+)[\"]*");

    QRegularExpressionMatchIterator it = tagRegExp.globalMatch(tag);
    ok = true;
    while (ok && it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        qDebug("parseTag: values: %s %s", qPrintable(match.captured(1)), qPrintable(match.captured(2)));

        if (match.captured(1) == "id") {
            d->migrationId = match.captured(2);
        } else if (match.captured(1) == "descn") {
            d->description = match.captured(2);
        } else if (match.captured(1) == "step_cnt") {
            d->stepCount = match.captured(2).toInt(&ok);
            d->step = 1;
        } else if (match.captured(1) == "step") {
            d->step = match.captured(2).toInt(&ok);
        }
    }

    return ok;
}

bool SqlMigrationStep::checkApplied() noexcept
{
    // TODO Store common queries in private data
    QSqlQuery query;
    query.prepare("select m.success, m.msg from migr m where m.id = :id;");
    query.bindValue("id", d->migrationId);
    query.exec();
    if (query.next()) {
        d->applied = query.value(0).toInt() > 0;
        d->errorMessage = query.value(1).toString();
        if (!d->applied && d->errorMessage.isNull()) {
            // Make sure an error message exists
            d->errorMessage = QString("Migration %s failed.").arg(d->migrationId);
        }
    } else {
        d->applied = false;
    }
    return d->applied;
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
        if (!hasPreviousAttempt()) {
            query.prepare("insert into migr values(:id, 1, :timestamp);");

        } else {
            query.prepare("update migr m set m.success = 1, m.timestamp = :timestamp, m.msg = null where m.id = :id;");
        }
        query.bindValue("id", d->migrationId);
        query.bindValue("timestamp", QDateTime::currentDateTime().toString());
        ok = query.exec();
        QSqlDatabase::database().commit();
        d->applied = true;
        d->errorMessage.clear();
    } else {
        QSqlDatabase::database().rollback();
        d->applied = false;
        d->errorMessage = errorMessage;
        QSqlDatabase::database().transaction();
        QSqlQuery query;
        if (!hasPreviousAttempt()) {
            query.prepare("insert into migr values(:id, 0, :timestamp, :msg);");
        } else {
            query.prepare("update migr m set m.success = 0, m.timestamp = :timestamp , m.msg = :msg where m.id = :id;");
        }
        query.bindValue("id", d->migrationId);
        QString timestamp = QDateTime::currentDateTime().toString();
        query.bindValue("timestamp", timestamp);
        query.bindValue("msg", d->errorMessage);
        ok = query.exec();
    }
    return ok;
}

const QString &SqlMigrationStep::getMigrationId() const noexcept
{
    return d->migrationId;
}

const QString &SqlMigrationStep::getDescription() const noexcept
{
    return d->description;
}

int SqlMigrationStep::getStep() const noexcept
{
    return d->step;
}

int SqlMigrationStep::getStepCount() const noexcept
{
    return d->stepCount;
}

// PRIVATE

bool SqlMigrationStep::hasPreviousAttempt() const noexcept
{
    return !d->errorMessage.isNull();
}
