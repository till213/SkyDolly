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
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QString>
#include <QSqlQuery>
#include <QDir>
#include <QCoreApplication>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/Location.h>
#include "SQLiteLocationDao.h"
#include "SqlMigrationStep.h"
#include "SqlMigration.h"

namespace
{
    constexpr char MigrationDirectory[] = "Resources/migr";

    constexpr int LatitudeIndex = 1;
    constexpr int LongitudeIndex = 2;
    constexpr int AltitudeIndex = 3;
    constexpr int PitchIndex = 4;
    constexpr int BankIndex = 5;
    constexpr int HeadingIndex = 6;
    constexpr int IndicatedAirspeedIndex = 7;
    constexpr int OnGroundIndex = 8;
    constexpr int DescriptionIndex = 9;
}

class SqlMigrationPrivate
{
public:
    SqlMigrationPrivate()
    {}

    SQLiteLocationDao locationDao;
};

// PUBLIC

SqlMigration::SqlMigration()
    : d(std::make_unique<SqlMigrationPrivate>())
{
     Q_INIT_RESOURCE(Migration);
#ifdef DEBUG
    qDebug() << "SqlMigration::SqlMigration: CREATED";
#endif
}

SqlMigration::~SqlMigration()
{
#ifdef DEBUG
    qDebug() << "SqlMigration::~SqlMigration: DELETED";
#endif
}

bool SqlMigration::migrate() noexcept
{
    bool ok = migrateSql(":/dao/sqlite/migr/LogbookMigration.sql");
    if (ok) {
        ok = migrateSql(":/dao/sqlite/migr/LocationMigration.sql");
    }
    if (ok) {
        QDir migrationDirectory = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
        if (migrationDirectory.dirName() == "MacOS") {
            // Navigate up the app bundle structure, into the Contents folder
            migrationDirectory.cdUp();
        }
#endif
        const QString locationsFilePath = migrationDirectory.absolutePath().append("/Resources/migr/Locations.csv");
        ok = migrateCsv(locationsFilePath);
    }
    return ok;
}

// PRIVATE

bool SqlMigration::migrateSql(const QString &migrationFilePath) noexcept
{
    // https://regex101.com/
    // @migr(...)
    static const QRegularExpression migrRegExp("@migr\\(([\\w=\"\\-,.\\s]+)\\)");
    QSqlQuery query = QSqlQuery("PRAGMA foreign_keys=0;");
    bool ok = query.exec();
    if (ok) {
        QFile migrationFile(migrationFilePath);
        ok = migrationFile.open(QFile::OpenModeFlag::ReadOnly | QFile::OpenModeFlag::Text);

        if (ok) {
            QTextStream textStream(&migrationFile);
            const QString migration = textStream.readAll();



            QStringList sqlStatements = migration.split(migrRegExp);
            QRegularExpressionMatchIterator it = migrRegExp.globalMatch(migration);

            // The first migration SQL statements start at index 1
            int i = 1;
            ok = true;
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

            migrationFile.close();
        }
    }
    query.prepare("PRAGMA foreign_keys=1;");
    return query.exec() && ok;
}

bool SqlMigration::migrateCsv(const QString &migrationFilePath) noexcept
{
    // https://regex101.com/
    static const QRegularExpression uuidRegExp {"[\"]?([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})[\"]?"};
    static const QRegularExpression csvRegExp {
        "([+-]?[0-9]*[.]?[0-9]+)," // Latitude
        "([+-]?[0-9]*[.]?[0-9]+)," // Longitude
        "([+-]?[0-9]*[.]?[0-9]+)," // Altitude
        "([+-]?[0-9]*[.]?[0-9]+)," // Pitch
        "([+-]?[0-9]*[.]?[0-9]+)," // Bank
        "([+-]?[0-9]*[.]?[0-9]+)," // Heading
        "([+-]?[\\d]+),"           // Indicated airspeed
        "[\"]?(false|true)[\"]?,"  // On Ground
        "[\"]?([^\"]+)[\"]?$"      // Description
    };
    QSqlQuery query = QSqlQuery("PRAGMA foreign_keys=0;");
    bool ok = query.exec();
    if (ok) {
        QFile migrationFile(migrationFilePath);
        ok = migrationFile.open(QFile::OpenModeFlag::ReadOnly | QFile::OpenModeFlag::Text);
        if (ok) {
            QTextStream textStream(&migrationFile);
            QString csv = textStream.readLine();
            if (csv.startsWith("\"MigrationId\"")) {
                // Skip column names
                csv = textStream.readLine();
            }

            while (!csv.isNull()) {
                QRegularExpressionMatch match = uuidRegExp.match(csv);
                const bool hasMatch = match.hasMatch();
                if (hasMatch) {
                    const QString uuid = match.captured(1);
                    SqlMigrationStep step;
                    step.setMigrationId(uuid);
                    step.setStep(1);
                    step.setStepCount(1);
                    if (!step.checkApplied()) {
                        const int offset = match.capturedLength();
                        match = csvRegExp.match(csv, offset);
                        if (match.hasMatch()) {
                            ok = migrateLocation(match);
                            const QString errorMessage = !ok ? QString("The location import %1 failed.").arg(uuid) : QString();
                            step.registerMigration(ok, errorMessage);
                        }
                    }
                }
                csv = textStream.readLine();
            }
            migrationFile.close();
        }
    }
    query.prepare("PRAGMA foreign_keys=1;");
    return query.exec() && ok;
}

bool SqlMigration::migrateLocation(const QRegularExpressionMatch &locationMatch) noexcept
{
    bool ok {true};
    Location location;
    location.latitude = locationMatch.captured(::LatitudeIndex).toDouble(&ok);
    if (ok) {
        location.longitude = locationMatch.captured(::LongitudeIndex).toDouble(&ok);
    }
    if (ok) {
        location.altitude = locationMatch.captured(::AltitudeIndex).toDouble(&ok);
    }
    if (ok) {
        location.pitch = locationMatch.captured(::PitchIndex).toDouble(&ok);
    }
    if (ok) {
        location.bank = locationMatch.captured(::BankIndex).toDouble(&ok);
    }
    if (ok) {
        location.heading = locationMatch.captured(::HeadingIndex).toDouble(&ok);
    }
    if (ok) {
        location.indicatedAirspeed = locationMatch.captured(::IndicatedAirspeedIndex).toInt(&ok);
    }
    if (ok) {
        location.onGround = locationMatch.captured(::OnGroundIndex).toLower() == "true" ? true : false;
    }
    if (ok) {
        location.description = locationMatch.captured(::DescriptionIndex);
    }
    if (ok) {
        ok = d->locationDao.add(location);
    }

    return ok;
}
