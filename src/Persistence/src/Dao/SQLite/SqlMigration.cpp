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
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QString>
#include <QSqlQuery>
#include <QDir>
#include <QCoreApplication>
#include <QTextCodec>

#include <Kernel/Const.h>
#include <Kernel/CsvParser.h>
#include <Model/Enumeration.h>
#include <Model/Location.h>
#include <Service/EnumerationService.h>
#include "SQLiteLocationDao.h"
#include "SqlMigrationStep.h"
#include "SqlMigration.h"

namespace
{
    // Also refer to Locations.csv
    constexpr int UuidIndex = 0;
    constexpr int TitleIndex = 1;
    constexpr int DescriptionIndex = 2;
    constexpr int CategoryIndex = 3;
    constexpr int CountryIndex = 4;
    constexpr int IdentifierIndex = 5;
    constexpr int LatitudeIndex = 6;
    constexpr int LongitudeIndex = 7;
    constexpr int AltitudeIndex = 8;
    constexpr int PitchIndex = 9;
    constexpr int BankIndex = 10;
    constexpr int TrueHeadingIndex = 11;
    constexpr int IndicatedAirspeedIndex = 12;
    constexpr int OnGroundIndex = 13;
    constexpr int AttributesIndex = 14;
    constexpr int EngineEvent = 15;

    // Depending on the CSV generating application (e.g. Excel or LibreOffice) the column titles may
    // or may not have "quotes"
    constexpr const char *LocationMigrationHeader {R"("MigrationId","Title","Description","Category")"};
    constexpr const char *AlternateLocationMigrationHeader {R"(MigrationId,Title,Description,Category)"};
}

struct SqlMigrationPrivate
{
    SQLiteLocationDao locationDao;
    EnumerationService enumerationService;
};

// PUBLIC

SqlMigration::SqlMigration() noexcept
    : d(std::make_unique<SqlMigrationPrivate>())
{
     Q_INIT_RESOURCE(Migration);
}

SqlMigration::SqlMigration(SqlMigration &&rhs) noexcept = default;
SqlMigration &SqlMigration::operator=(SqlMigration &&rhs) noexcept = default;
SqlMigration::~SqlMigration() = default;

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
        if (QFileInfo::exists(locationsFilePath)) {
            ok = migrateCsv(locationsFilePath);
        }
    }
    return ok;
}

// PRIVATE

bool SqlMigration::migrateSql(const QString &migrationFilePath) noexcept
{
    // https://regex101.com/
    // @migr(...)
    static const QRegularExpression migrRegExp(R"(@migr\(([\w="\-,.\s]+)\))");
    QSqlQuery query = QSqlQuery("PRAGMA foreign_keys=0;");
    bool ok = query.exec();
    if (ok) {
        QFile migrationFile(migrationFilePath);
        ok = migrationFile.open(QFile::OpenModeFlag::ReadOnly | QFile::OpenModeFlag::Text);
        if (ok) {
            QTextStream textStream(&migrationFile);
            textStream.setCodec(QTextCodec::codecForName("UTF-8"));
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
                qDebug() << "SqlMigration::migrate:" << tag;
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
    QSqlQuery query = QSqlQuery("PRAGMA foreign_keys=0;");
    bool ok = query.exec();
    if (ok) {
        QFile migrationFile(migrationFilePath);
        ok = migrationFile.open(QFile::OpenModeFlag::ReadOnly | QFile::OpenModeFlag::Text);
        if (ok) {
            QTextStream textStream(&migrationFile);
            textStream.setCodec(QTextCodec::codecForName("UTF-8"));
            CsvParser csvParser;
            CsvParser::Rows rows = csvParser.parse(textStream, ::LocationMigrationHeader, ::AlternateLocationMigrationHeader);
            for (const auto &row : rows) {
                const QString uuid = row.at(::UuidIndex);
                SqlMigrationStep step;
                step.setMigrationId(uuid);
                step.setStep(1);
                step.setStepCount(1);
                if (!step.checkApplied()) {
                    ok = migrateLocation(row);
                    const QString errorMessage = !ok ? QString("The location import %1 failed.").arg(uuid) : QString();
                    step.registerMigration(ok, errorMessage);
                }
            }
            migrationFile.close();
        }
    }
    query.prepare("PRAGMA foreign_keys=1;");
    return query.exec() && ok;
}

bool SqlMigration::migrateLocation(const CsvParser::Row &row) noexcept
{
    bool ok {true};
    Location location;
    location.title = row.at(::TitleIndex);
    QString description = row.at(::DescriptionIndex);
    location.description = description.replace("\\n", "\n");
    Enumeration locationTypeEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::LocationType, &ok);
    if (ok) {
        location.typeId = locationTypeEnumeration.getItemBySymbolicId(EnumerationService::LocationTypeSystemSymbolicId).id;
    }
    Enumeration locationCategoryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::LocationCategory, &ok);
    if (ok) {
        const QString categorySymbolicId = row.at(::CategoryIndex);
        location.categoryId = locationCategoryEnumeration.getItemBySymbolicId(categorySymbolicId).id;
    }
    Enumeration countryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::Country, &ok);
    if (ok) {
        const QString countrySymbolicId = row.at(::CountryIndex);
        location.countryId = countryEnumeration.getItemBySymbolicId(countrySymbolicId).id;
    }
    if (ok) {
        location.identifier = row.at(::IdentifierIndex);
    }
    if (ok) {
        location.latitude = row.at(::LatitudeIndex).toDouble(&ok);
    }
    if (ok) {
        location.longitude = row.at(::LongitudeIndex).toDouble(&ok);
    }
    if (ok) {
        location.altitude = row.at(::AltitudeIndex).toDouble(&ok);
    }
    if (ok) {
        location.pitch = row.at(::PitchIndex).toDouble(&ok);
    }
    if (ok) {
        location.bank = row.at(::BankIndex).toDouble(&ok);
    }
    if (ok) {
        location.trueHeading = row.at(::TrueHeadingIndex).toDouble(&ok);
    }
    if (ok) {
        location.indicatedAirspeed = row.at(::IndicatedAirspeedIndex).toInt(&ok);
    }
    if (ok) {
        location.attributes = row.at(::AttributesIndex).toLongLong(&ok);
    }
    if (ok) {
        location.onGround = row.at(::OnGroundIndex).toLower() == "true" ? true : false;
    }
    Enumeration engineEventEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::EngineEvent, &ok);
    if (ok) {
        const QString engineEventSymbolicId = row.at(::EngineEvent);
        location.engineEventId = engineEventEnumeration.getItemBySymbolicId(engineEventSymbolicId).id;
    }
    if (ok) {
        ok = d->locationDao.add(location);
    }

    return ok;
}
