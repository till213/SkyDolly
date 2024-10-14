/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef PERSISTEDENUMERATIONITEM_H
#define PERSISTEDENUMERATIONITEM_H

#include <memory>
#include <cstdint>

class QString;

#include <Model/SimType.h>
#include "PersistenceLib.h"

struct PersistedEnumerationItemPrivate;

class PERSISTENCE_API PersistedEnumerationItem
{
public:
    /*!
     * Creates this PersistedEnumerationItem, initialising its value with the \p symId
     * of the persisted enumeration \p enumerationName of the \e application database
     * (default connection).
     *
     * \param enumerationName
     *        the name of the persisted enumeration
     * \param symId
     *        the symbolic id
     * \sa QSqlDatabase::database
     */
    PersistedEnumerationItem(const QString &enumerationName, const QString &symId) noexcept;
    PersistedEnumerationItem();
    PersistedEnumerationItem(const PersistedEnumerationItem &rhs) = delete;
    PersistedEnumerationItem(PersistedEnumerationItem &&rhs) noexcept;
    PersistedEnumerationItem &operator=(const PersistedEnumerationItem &rhs) = delete;
    PersistedEnumerationItem &operator=(PersistedEnumerationItem &&rhs) noexcept;
    ~PersistedEnumerationItem();

    std::int64_t id() const noexcept;

private:
    std::unique_ptr<PersistedEnumerationItemPrivate> d;
};

#endif // PERSISTEDENUMERATIONITEM_H
