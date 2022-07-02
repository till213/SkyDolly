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
#ifndef SQLMIGRATIONSTEP_H
#define SQLMIGRATIONSTEP_H

#include <memory>

#include <QStringList>
#include <QString>
#include <QStringView>

class QRegularExpressionMatch;
class QStringRef;

class SqlMigrationStepPrivate;

class SqlMigrationStep
{
public:
    SqlMigrationStep();
    ~SqlMigrationStep();

    bool isValid() const noexcept;
    bool parseTag(const QRegularExpressionMatch &tagMatch) noexcept;

    bool checkApplied() noexcept;
    bool execute(QStringView sql) noexcept;
    void registerMigration(bool success, QString errorMessage = QString()) noexcept;

    const QString &getMigrationId() const noexcept;
    void setMigrationId(QString migrationId) noexcept;
    const QString &getDescription() const noexcept;
    void setDescription(QString description) noexcept;
    int getStep() const noexcept;
    void setStep(int step) noexcept;
    int getStepCount() const noexcept;
    void setStepCount(int count) noexcept;

private:
    std::unique_ptr<SqlMigrationStepPrivate> d;

    bool hasPreviousAttempt() const noexcept;
};

#endif // SQLMIGRATIONSTEP_H
