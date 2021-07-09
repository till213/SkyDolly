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
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QStringRef>

#include "SqlMigrationStep.h"
#include "SqlMigration.h"

class SqlMigrationPrivate
{
public:
    SqlMigrationPrivate()
    {}
};

// PUBLIC

SqlMigration::SqlMigration()
    : d(std::make_unique<SqlMigrationPrivate>())
{
     Q_INIT_RESOURCE(Migration);
}

SqlMigration::~SqlMigration()
{}

bool SqlMigration::migrateExAnte() noexcept
{
    return migrate(":/dao/sqlite/migr/migr-ex-ante.sql");
}

bool SqlMigration::migrateDdl() noexcept
{
    return migrate(":/dao/sqlite/migr/migr-ddl.sql");
}

bool SqlMigration::migrateExPost() noexcept
{
    return migrate(":/dao/sqlite/migr/migr-ex-post.sql");
}

// PRIVATE

bool SqlMigration::migrate(const QString &migrationFilePath) noexcept
{
    QFile migr(migrationFilePath);
    bool ok = migr.open(QFile::OpenModeFlag::ReadOnly | QFile::OpenModeFlag::Text);

    if (ok) {
        QTextStream textStream(&migr);
        const QString migration = textStream.readAll();

        // https://regex101.com/
        // @migr(...)
        const QRegularExpression migrRegExp("@migr\\(([\\w=\"\\-,.\\s]+)\\)");

        QStringList sqlStatements = migration.split(migrRegExp);
        QRegularExpressionMatchIterator it = migrRegExp.globalMatch(migration);

        // The first migration SQL statements start at index 1
        int i = 1;
        bool ok = true;
        while (ok && it.hasNext()) {
            const QRegularExpressionMatch tagMatch = it.next();
            QString tag = tagMatch.captured(1);
#ifdef DEBUG
            qDebug("SqlMigration::migrate: %s", qPrintable(tag));
#endif

            SqlMigrationStep step;
            ok = step.parseTag(tagMatch);
            if (ok && !step.checkApplied()) {
                ok = step.execute(sqlStatements.at(i));
            }
            ++i;
        }

        migr.close();
    }
    return ok;
}
