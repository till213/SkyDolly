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
#include <QVariant>
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
        : step(0),
          stepCount(0),
          applied(false)
    {
        checkAppliedQuery.prepare("select m.success, m.msg from migr m where m.id = :id;");
        insertMigrQuery.prepare("insert into migr values(:id, :success, :timestamp, :msg);");
        updateMigrQuery.prepare("update migr set success = :success, timestamp = :timestamp, msg = :msg where id = :id;");
    }

    QString migrationId;
    QString description;
    int step;
    int stepCount;
    bool applied;
    QString errorMessage;

    static constexpr char TimestampFormat[] = "yyyy-MM-dd hh:mm:ss.sss";

    QSqlQuery checkAppliedQuery;
    QSqlQuery insertMigrQuery;
    QSqlQuery updateMigrQuery;

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

    // Match the tag's content, e.g. id = 42, descn = "The description", step = 1
    // https://regex101.com/
    const QRegularExpression tagRegExp("([\\w]+)\\s*=\\s*[\"]*([\\w\\s\\-]+)[\"]*");

    QRegularExpressionMatchIterator it = tagRegExp.globalMatch(tag);
    ok = true;
    while (ok && it.hasNext()) {
        QRegularExpressionMatch match = it.next();
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
    d->checkAppliedQuery.bindValue(":id", d->migrationId);
    d->checkAppliedQuery.exec();

    if (d->checkAppliedQuery.next()) {
        d->applied = d->checkAppliedQuery.value(0).toInt() > 0;
        d->errorMessage = d->checkAppliedQuery.value(1).toString();
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
#ifdef DEBUG
        qDebug("SqlMigrationStep::execute: sql: %s", qPrintable(match.captured(1)));
#endif

        QSqlQuery query;
        query.prepare(match.captured(1).trimmed() % ";");
        ok = query.exec();
        if (!ok) {
            errorMessage = query.lastError().databaseText() + " - error code: " + query.lastError().nativeErrorCode();
        }
    }
    QString timestamp = QDateTime::currentDateTime().toString(SqlMigrationStepPrivate::TimestampFormat);
    if (ok) {
        QSqlQuery query;
        if (!hasPreviousAttempt()) {
            query = d->insertMigrQuery;

        } else {
            query = d->updateMigrQuery;
        }
        query.bindValue(":id", d->migrationId);
        query.bindValue(":success", 1);
        query.bindValue(":timestamp", timestamp);
        query.bindValue(":msg", QString());
        ok = query.exec();
        QSqlDatabase::database().commit();
        d->applied = true;
        d->errorMessage.clear();
    } else {
        QSqlDatabase::database().rollback();
        QSqlDatabase::database().transaction();
        QSqlQuery query;
        if (!hasPreviousAttempt()) {
            query = d->insertMigrQuery;
        } else {
            query = d->updateMigrQuery;
        }
        d->applied = false;
        d->errorMessage = errorMessage;

        query.bindValue(":id", d->migrationId);
        query.bindValue(":success", 0);
        query.bindValue(":timestamp", timestamp);
        query.bindValue(":msg", d->errorMessage);
        query.exec();
        QSqlDatabase::database().commit();
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
