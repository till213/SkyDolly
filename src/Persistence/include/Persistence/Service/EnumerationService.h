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
#ifndef ENUMERATIONSERVICE_H
#define ENUMERATIONSERVICE_H

#include <memory>

#include <QString>

#include <Model/Enumeration.h>
#include "../PersistenceLib.h"

struct EnumerationServicePrivate;

class PERSISTENCE_API EnumerationService
{
public:
    EnumerationService() noexcept;
    EnumerationService(const EnumerationService &rhs) = delete;
    EnumerationService(EnumerationService &&rhs);
    EnumerationService &operator=(const EnumerationService &rhs) = delete;
    EnumerationService &operator=(EnumerationService &&rhs);
    ~EnumerationService();

    /*!
     * Gets the persisted Enumeration by its enumeration \c name.
     *
     * \param name
     *        the name of the enumeration
     * \param ok
     *        if set, \c true if successful; \c false else (unknown Enumeration name, no database connection)
     * \return the persisted Enumeration identified by the \c name
     */
    Enumeration getEnumerationByName(QString name, bool *ok = nullptr);

    // Implementation note:
    // Well-known database enumerations: TitleCase name must match with corresponding
    // snake_case name (without the "enum_" prefix) of the corresponding "enumeration table".
    //
    // At least with MinGW 8.1 (Windows) using "static constexpr const char *" does not seem
    // to link "beyond DLL boundaries", so we use QString for now
    static inline const QString BackupPeriod {QStringLiteral("BackupPeriod")};
    static inline const QString LocationType {QStringLiteral("LocationType")};
    static inline const QString LocationCategory {QStringLiteral("LocationCategory")};
    static inline const QString Country {QStringLiteral("Country")};

    // Well-known symbolic IDs

    // Location type
    static constexpr const char *LocationTypeSystemSymbolicId {"S"};
    static constexpr const char *LocationTypeImportSymbolicId {"I"};
    static constexpr const char *LocationTypeUserSymbolicId {"U"};

    // Location category
    static constexpr const char *LocationCategoryNoneSymbolicId {"00"};

    // Country
    static constexpr const char *CountryWorldSymbolicId {"00"};

private:
    std::unique_ptr<EnumerationServicePrivate> d;
};

#endif // ENUMERATIONSERVICE_H
