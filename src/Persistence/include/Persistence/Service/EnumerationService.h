/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#ifndef ENUMERATIONSERVICE_H
#define ENUMERATIONSERVICE_H

#include <memory>

#include <QString>
#include <QStringLiteral>
#include <QSqlDatabase>

#include <Kernel/Const.h>
#include <Model/Enumeration.h>
#include "../PersistenceLib.h"

struct EnumerationServicePrivate;

class PERSISTENCE_API EnumerationService final
{
public:
    EnumerationService(QString connectionName = Const::DefaultConnectionName) noexcept;
    EnumerationService(const EnumerationService &rhs) = delete;
    EnumerationService(EnumerationService &&rhs) noexcept;
    EnumerationService &operator=(const EnumerationService &rhs) = delete;
    EnumerationService &operator=(EnumerationService &&rhs) noexcept;
    ~EnumerationService();

    /*!
     * Gets the persisted Enumeration by its enumeration \p name.
     *
     * \param name
     *        the name of the enumeration
     * \param order
     *        defines the attribute by which to order the enumarations:
     *        by ID (default), by symbolic ID or name
     * \param ok
     *        if set, \c true if successful; \c false else (unknown Enumeration name, no database connection)
     * \return the persisted Enumeration identified by the \p name
     */
    Enumeration getEnumerationByName(const QString &name, Enumeration::Order order = Enumeration::Order::Id, bool *ok = nullptr);

    // Implementation note 1:
    // Well-known database enumerations: TitleCase name must match with corresponding
    // snake_case name (without the "enum_" prefix) of the corresponding "enumeration table".
    //
    // Implementation note 2:
    // We need to use QStringLiteral here for static inline const QStrings
    // https://forum.qt.io/topic/102312/very-strange-heap-corruption-exit-code-1073740940-0xc0000374-with-static-inline-const-qstring-release-only
    // We cannot use "static constexpr const char *" either, as this does not get exported from the DLL
    static inline const QString BackupPeriod {QStringLiteral("BackupPeriod")};
    static inline const QString LocationType {QStringLiteral("LocationType")};
    static inline const QString LocationCategory {QStringLiteral("LocationCategory")};
    static inline const QString Country {QStringLiteral("Country")};
    static inline const QString EngineEvent {QStringLiteral("EngineEvent")};

    // Well-known symbolic IDs

    // Backup period
    static inline const QString BackupPeriodNeverSymId {QStringLiteral("NEVER")};
    static inline const QString BackupPeriodNowSymId {QStringLiteral("NOW")};
    static inline const QString BackupPeriodNextTimeSymId {QStringLiteral("NEXT")};
    static inline const QString BackupPeriodMonthlySymId {QStringLiteral("MONTH")};
    static inline const QString BackupPeriodWeeklySymId {QStringLiteral("WEEK")};
    static inline const QString BackupPeriodDailySymId {QStringLiteral("DAY")};
    static inline const QString BackupPeriodAlwaysSymId {QStringLiteral("ALWAYS")};

    // Location type
    static inline const QString LocationTypePresetSymId {QStringLiteral("P")};
    static inline const QString LocationTypeImportSymId {QStringLiteral("I")};
    static inline const QString LocationTypeUserSymId {QStringLiteral("U")};

    // Location category
    static inline const QString LocationCategoryNoneSymId {QStringLiteral("00")};

    // Country
    static inline const QString CountryWorldSymId {QStringLiteral("00")};

    // Engine event
    static inline const QString EngineEventStartSymId {QStringLiteral("START")};
    static inline const QString EngineEventStopSymId {QStringLiteral("STOP")};
    static inline const QString EngineEventKeepSymId {QStringLiteral("KEEP")};

private:
    std::unique_ptr<EnumerationServicePrivate> d;
};

#endif // ENUMERATIONSERVICE_H
