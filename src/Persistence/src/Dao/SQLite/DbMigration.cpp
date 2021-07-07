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

#include "DbMigration.h"

class DbMigrationPrivate
{
public:
    DbMigrationPrivate()
    {}
};

// PUBLIC

DbMigration::DbMigration()
    : d(std::make_unique<DbMigrationPrivate>())
{
     Q_INIT_RESOURCE(Migration);
}

DbMigration::~DbMigration()
{}

bool DbMigration::migrateExAnte() noexcept
{
    return true;
}

bool DbMigration::migrateDdl() noexcept
{
    QFile migr(":/dao/sqlite/migr/migr-ddl.sql");
    migr.open(QFile::OpenModeFlag::ReadOnly | QFile::OpenModeFlag::Text);

    QTextStream textStream(&migr);
    QStringList lines;
    while (!textStream.atEnd()) {
        lines += textStream.readLine();
    }

    for (const QString &line : std::as_const(lines)) {
        if (line.trimmed().startsWith("@migr")) {
            // https://regex101.com/
            const QRegularExpression regexp("@migr\\(([\\w=\"\\-,.\\s]+)\\)");
            QRegularExpressionMatch match = regexp.match(line);
            if (match.hasMatch()) {
                QString migrTag = match.captured();
                qDebug("migration: %s", qPrintable(migrTag));
            }
        }
    }

    migr.close();

    return true;
}

bool DbMigration::migrateExPost() noexcept
{
    return true;
}

