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
#include <memory>
#include <utility>

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QString>
#include <QStringLiteral>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDir>
#include <QCoreApplication>
#include <QStringConverter>

#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include <Kernel/CsvParser.h>
#include <Model/Enumeration.h>
#include <Model/Location.h>
#include <Service/EnumerationService.h>
#include <Migration.h>
#include "SQLiteLocationDao.h"
#include "SqlMigrationStep.h"
#include "SqlMigration.h"

namespace
{
    // Also refer to res/migr/Locations.csv
    enum Index
    {
        Uuid = 0,
        Title,
        Description,
        Category,
        Country,
        Identifier,
        Latitude,
        Longitude,
        Altitude,
        Pitch,
        Bank,
        TrueHeading,
        IndicatedAirspeed,
        OnGround,        
        EngineEvent,
        LocalSimulationDate,
        LocalSimulationTime,
        // Last index
        Count
    };

    // Depending on the CSV generating application (e.g. Excel or LibreOffice) the column titles may
    // or may not have "quotes"
    constexpr const char *LocationMigrationHeader {R"("MigrationId","Title","Description","Category","Country","Identifier","Latitude","Longitude")"};
    constexpr const char *AlternateLocationMigrationHeader {R"(MigrationId,Title,Description,Category,Country,Identifier,Latitude,Longitude)"};
}

struct SqlMigrationPrivate
{
    SqlMigrationPrivate(QString connectionName) noexcept
        : connectionName(connectionName),
          locationDao(std::make_unique<SQLiteLocationDao>(connectionName)),
          enumerationService(std::make_unique<EnumerationService>(std::move(connectionName)))
    {}

    QString connectionName;
    std::unique_ptr<SQLiteLocationDao> locationDao;
    std::unique_ptr<EnumerationService> enumerationService;
};

// PUBLIC

SqlMigration::SqlMigration(QString connectionName) noexcept
    : d {std::make_unique<SqlMigrationPrivate>(std::move(connectionName))}
{}

SqlMigration::SqlMigration(SqlMigration &&rhs) noexcept = default;
SqlMigration &SqlMigration::operator=(SqlMigration &&rhs) noexcept = default;
SqlMigration::~SqlMigration() = default;

bool SqlMigration::migrate(Migration::Milestones milestones) const noexcept
{
    bool ok {true};
    if (milestones.testFlag(Migration::Milestone::Schema)) {
        ok = migrateSql(":/dao/sqlite/migr/LogbookMigration.sql");
        if (ok) {
            ok = migrateSql(":/dao/sqlite/migr/LocationMigration.sql");
        }
    }
    if (ok && milestones.testFlag(Migration::Milestone::Location)) {
        QDir migrationDirectory {QDir(QCoreApplication::applicationDirPath())};
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

bool SqlMigration::migrateSql(const QString &migrationFilePath) const noexcept
{
    // https://regex101.com/
    // @migr(...)
    static const QRegularExpression migrRegExp(R"(@migr\(([\w="\-,.\s]+)\))");
    const auto db {QSqlDatabase::database(d->connectionName)};
    auto query = QSqlQuery("PRAGMA foreign_keys=0;", db);
    bool ok = query.exec();
    if (ok) {
        QFile migrationFile(migrationFilePath);
        ok = migrationFile.open(QFile::OpenModeFlag::ReadOnly | QFile::OpenModeFlag::Text);
        if (ok) {
            QTextStream textStream(&migrationFile);
            textStream.setEncoding(QStringConverter::Utf8);
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
                SqlMigrationStep step {d->connectionName};
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

bool SqlMigration::migrateCsv(const QString &migrationFilePath) const noexcept
{
    const auto db {QSqlDatabase::database(d->connectionName)};
    QSqlQuery query = QSqlQuery("PRAGMA foreign_keys=0;", db);
    bool ok = query.exec();
    if (ok) {
        QFile migrationFile(migrationFilePath);
        ok = migrationFile.open(QFile::OpenModeFlag::ReadOnly | QFile::OpenModeFlag::Text);
        if (ok) {
            QTextStream textStream(&migrationFile);
            textStream.setEncoding(QStringConverter::Utf8);
            CsvParser csvParser;
            CsvParser::Rows rows = csvParser.parse(textStream, ::LocationMigrationHeader, ::AlternateLocationMigrationHeader);
            if (CsvParser::validate(rows, Enum::underly(::Index::Count))) {
                for (const auto &row : rows) {
                    const QString uuid = row.at(::Index::Uuid);
                    SqlMigrationStep step {d->connectionName};
                    step.setMigrationId(uuid);
                    step.setStep(1);
                    step.setStepCount(1);
                    if (!step.checkApplied()) {
                        ok = migrateLocation(row);
                        const QString errorMessage= !ok ? QStringLiteral("The location import %1 failed.").arg(uuid) : QString();
                        step.registerMigration(ok, errorMessage);
                    }
                }
            }
#ifdef DEBUG
            else {
                qDebug() << "SqlMigration::migrateCsv: invalid location CSV file:" << migrationFilePath;
            }
#endif
            migrationFile.close();
        }
    }
    query.prepare("PRAGMA foreign_keys=1;");
    return query.exec() && ok;
}

bool SqlMigration::migrateLocation(const CsvParser::Row &row) const noexcept
{
    bool ok {true};
    Location location;
    location.title = row.at(::Index::Title);
    QString description = row.at(::Index::Description);
    location.description = description.replace("\\n", "\n");
    Enumeration locationTypeEnumeration = d->enumerationService->getEnumerationByName(EnumerationService::LocationType, Enumeration::Order::Id, &ok);
    if (ok) {
        location.typeId = locationTypeEnumeration.getItemBySymId(EnumerationService::LocationTypePresetSymId).id;
    }
    Enumeration locationCategoryEnumeration = d->enumerationService->getEnumerationByName(EnumerationService::LocationCategory, Enumeration::Order::Id, &ok);
    if (ok) {
        const QString &categorySymId = row.at(::Index::Category);
        location.categoryId = locationCategoryEnumeration.getItemBySymId(categorySymId).id;
    }
    Enumeration countryEnumeration = d->enumerationService->getEnumerationByName(EnumerationService::Country, Enumeration::Order::Id, &ok);
    if (ok) {
        const QString &countrySymId = row.at(::Index::Country);
        location.countryId = countryEnumeration.getItemBySymId(countrySymId).id;
    }
    if (ok) {
        location.identifier = row.at(::Index::Identifier);
    }
    if (ok) {
        location.latitude = row.at(::Index::Latitude).toDouble(&ok);
    }
    if (ok) {
        location.longitude = row.at(::Index::Longitude).toDouble(&ok);
    }
    if (ok) {
        location.altitude = row.at(::Index::Altitude).toDouble(&ok);
    }
    if (ok) {
        location.pitch = row.at(::Index::Pitch).toDouble(&ok);
    }
    if (ok) {
        location.bank = row.at(::Index::Bank).toDouble(&ok);
    }
    if (ok) {
        location.trueHeading = row.at(::Index::TrueHeading).toDouble(&ok);
    }
    if (ok) {
        location.indicatedAirspeed = row.at(::Index::IndicatedAirspeed).toInt(&ok);
    }
    if (ok) {
        location.onGround = row.at(::Index::OnGround).toLower() == "true" ? true : false;
    }
    Enumeration engineEventEnumeration = d->enumerationService->getEnumerationByName(EnumerationService::EngineEvent, Enumeration::Order::Id, &ok);
    if (ok) {
        const QString &engineEventSymId = row.at(::Index::EngineEvent);
        location.engineEventId = engineEventEnumeration.getItemBySymId(engineEventSymId).id;
    }
    const auto dateValue = row.at(::Index::LocalSimulationDate);
    if (!dateValue.isEmpty()) {
        location.localSimulationDate = QDate::fromString(dateValue, Qt::DateFormat::ISODate);
        ok = location.localSimulationDate.isValid();
    }
    if (ok) {
        const auto timeValue = row.at(::Index::LocalSimulationTime);
        if (!timeValue.isEmpty()) {
            location.localSimulationTime = QTime::fromString(timeValue, Qt::DateFormat::ISODate);
            ok = location.localSimulationTime.isValid();
        }
    }
    if (ok) {
        ok = d->locationDao->add(location);
    }

    return ok;
}
