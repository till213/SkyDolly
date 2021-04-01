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

bool SqlMigrationStep::parseTag(const QString &tag) noexcept
{
    const QStringList elements = tag.split(",");
    for (const QString &element : elements) {

        // TODO Use [\w]+\s*=["]*([\w]+)["]
        const QStringList values = element.split("=");
        qDebug("parseTag: values: %s %s", qPrintable(values.at(0)), qPrintable(values.at(1)));

    }
    return true;
}

bool SqlMigrationStep::parseSql(QStringList &sql) noexcept
{
return true;
}

bool SqlMigrationStep::execute() noexcept
{
return true;
}

const QString &SqlMigrationStep::getMigrationId() const noexcept
{
return std::move(QString());
}

const QString &SqlMigrationStep::getDescription() const noexcept
{
return std::move(QString());
}

int SqlMigrationStep::getStep() const noexcept
{
return 0;
}

int SqlMigrationStep::getStepCount() const noexcept
{
return 0;
}

const QString &SqlMigrationStep::getSqlStatement() const noexcept
{
return QString();
}
