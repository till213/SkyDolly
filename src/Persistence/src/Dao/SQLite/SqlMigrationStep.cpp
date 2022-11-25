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
#include <utility>

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
#ifdef DEBUG
#include <QDebug>
#endif

#include "SqlMigrationStep.h"

struct SqlMigrationStepPrivate
{
    SqlMigrationStepPrivate()
    {}

    QString migrationId;
    QString description;
    int step {0};
    int stepCount {0};

    QString errorMessage;
};

// PUBLIC

SqlMigrationStep::SqlMigrationStep() noexcept
    : d(std::make_unique<SqlMigrationStepPrivate>())
{}

SqlMigrationStep::SqlMigrationStep(SqlMigrationStep &&rhs) noexcept = default;
SqlMigrationStep &SqlMigrationStep::operator=(SqlMigrationStep &&rhs) noexcept = default;
SqlMigrationStep::~SqlMigrationStep() = default;

bool SqlMigrationStep::isValid() const noexcept
{
    return !d->migrationId.isNull();
}

bool SqlMigrationStep::parseTag(const QRegularExpressionMatch &tagMatch) noexcept
{

    const QString tag = tagMatch.captured(1);

    // Match the tag's content, e.g. id = 42, descn = "The description", step = 1
    // https://regex101.com/
    static const QRegularExpression tagRegExp(R"(([\w]+)\s*=\s*["]*([\w\s\-]+)["]*)");

    QRegularExpressionMatchIterator it = tagRegExp.globalMatch(tag);
    bool ok {true};
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
        if (!ok) {
            break;
        }
    }
    return ok;
}

bool SqlMigrationStep::checkApplied() noexcept
{
    QSqlQuery checkAppliedQuery;
    checkAppliedQuery.prepare("select m.success, m.step, m.msg from migr m where m.id = :id and m.step = :step;");
    checkAppliedQuery.bindValue(":id", d->migrationId);
    checkAppliedQuery.bindValue(":step", d->step);
    checkAppliedQuery.exec();

    bool applied {false};
    if (checkAppliedQuery.next()) {
        applied = checkAppliedQuery.value(0).toInt() > 0;
        d->errorMessage = checkAppliedQuery.value(1).toString();
        if (!applied && d->errorMessage.isNull()) {
            // Make sure an error message exists
            d->errorMessage = QString("Migration %s failed.").arg(d->migrationId);
        }
    }
    return applied;
}

bool SqlMigrationStep::execute(QStringView sql) noexcept
{
    QString errorMessage;
    // Match SQL statements terminated with a semicolon
    static const QRegularExpression sqlRegExp(R"(([\w|\W]+?);)");

    // Note that DDL statements do not require transactions; but for
    // now we execute all queries within a transaction
    bool ok = QSqlDatabase::database().transaction();

    QRegularExpressionMatchIterator it = sqlRegExp.globalMatch(sql);
    while (ok && it.hasNext()) {

        QRegularExpressionMatch match = it.next();
#ifdef DEBUG
        qDebug() << "SqlMigrationStep::execute: SQL:" << match.captured(1) << "\n\n";
#endif
        QSqlQuery query;
        ok = query.exec(match.captured(1).trimmed() % ";");
        if (!ok) {
            errorMessage = query.lastError().text() + " - error code: " + query.lastError().nativeErrorCode();
            QSqlDatabase::database().rollback();
#ifdef DEBUG
            qDebug() << "SqlMigrationStep::execute: FAILED:\n" << errorMessage;
#endif
        }
    }

    registerMigration(ok, errorMessage);
    return ok;
}

void SqlMigrationStep::registerMigration(bool success, QString errorMessage) noexcept
{
    bool ok = success;
    if (ok) {
        QSqlQuery migrQuery;
        if (!hasPreviousAttempt()) {
            migrQuery.prepare("insert into migr (id, step, success, msg) values(:id, :step, :success, :msg);");
        } else {
            migrQuery.prepare("update migr set success = :success, msg = :msg where id = :id and step = :step;");
        }
        migrQuery.bindValue(":id", d->migrationId);
        migrQuery.bindValue(":step", d->step);
        migrQuery.bindValue(":success", 1);
        migrQuery.bindValue(":msg", QString());
        ok = migrQuery.exec();
        if (ok) {
            ok = QSqlDatabase::database().commit();
        } else {
#ifdef DEBUG
            qDebug() << "SqlMigrationStep::registerMigration: update MIGR table FAILED:\n" << migrQuery.lastError().text() << "- error code:" << migrQuery.lastError().nativeErrorCode();
#endif
            QSqlDatabase::database().rollback();
        }
        if (ok) {
            d->errorMessage.clear();
        }
    } else {
        bool migrQueryOk = QSqlDatabase::database().transaction();
        if (migrQueryOk) {
            QSqlQuery migrQuery;
            if (!hasPreviousAttempt()) {
                migrQuery.prepare("insert into migr (id, step, success, msg) values(:id, :step, :success, :msg);");
            } else {
                migrQuery.prepare("update migr set success = :success, msg = :msg where id = :id and step = :step;");
            }
            d->errorMessage = std::move(errorMessage);

            migrQuery.bindValue(":id", d->migrationId);
            migrQuery.bindValue(":step", d->step);
            migrQuery.bindValue(":success", 0);
            migrQuery.bindValue(":msg", d->errorMessage);
            migrQueryOk = migrQuery.exec();
            if (migrQueryOk) {
                QSqlDatabase::database().commit();
            } else {
#ifdef DEBUG
            qDebug() << "SqlMigrationStep::registerMigration: update MIGR table FAILED:\n" << migrQuery.lastError().text() << "- error code:" << migrQuery.lastError().nativeErrorCode();
#endif
                QSqlDatabase::database().rollback();
            }
        }
#ifdef DEBUG
        else {
            qDebug() << "SqlMigrationStep::registerMigration: FAILED to create transaction.";
        }
#endif
    }
}

const QString &SqlMigrationStep::getMigrationId() const noexcept
{
    return d->migrationId;
}

void SqlMigrationStep::setMigrationId(QString migrationId) noexcept
{
    d->migrationId = std::move(migrationId);
}

const QString &SqlMigrationStep::getDescription() const noexcept
{
    return d->description;
}

void SqlMigrationStep::setDescription(QString description) noexcept
{
    d->description = std::move(description);
}

int SqlMigrationStep::getStep() const noexcept
{
    return d->step;
}

void SqlMigrationStep::setStep(int step) noexcept
{
    d->step = step;
}

int SqlMigrationStep::getStepCount() const noexcept
{
    return d->stepCount;
}

void SqlMigrationStep::setStepCount(int count) noexcept
{
    d->stepCount = count;
}

// PRIVATE

bool SqlMigrationStep::hasPreviousAttempt() const noexcept
{
    return !d->errorMessage.isNull();
}
