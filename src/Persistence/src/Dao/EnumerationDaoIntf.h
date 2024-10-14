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
#ifndef ENUMERATIONDAOINTF_H
#define ENUMERATIONDAOINTF_H

#include <QString>

#include <Model/Enumeration.h>

class Enumeration;

/*!
 * Database access to enumeration data objects.
 */
class EnumerationDaoIntf
{
public:
    EnumerationDaoIntf() = default;
    EnumerationDaoIntf(const EnumerationDaoIntf &rhs) = delete;
    EnumerationDaoIntf(EnumerationDaoIntf &&rhs) = default;
    EnumerationDaoIntf &operator=(const EnumerationDaoIntf &rhs) = delete;
    EnumerationDaoIntf &operator=(EnumerationDaoIntf &&rhs) = default;
    virtual ~EnumerationDaoIntf() = default;

    /*!
     * Returns the enumeration given by its \p name. The name is to be given in camelCase,
     * without the "enum_" prefix.
     *
     * Examples:
     *
     * | "Enumeration Name" | "Table Name"           |
     * | ----               | ----                   |
     * | BackupPeriod       | enum_backup_period     |
     * | LocationCategory   | enum_location_category |
     *
     * \param name
     *        the name of the enumeration; in camelCase
     * \param order
     *        defines the attribute by which to order the enumarations:
     *        by ID (default), by symbolic ID or name
     * \param ok
     *        if provided, set to \c true if successful; \c false else (enumeration does not exist; no DB connection)
     * \return the Enumeration as defined on the database
     */
    virtual Enumeration get(const QString &name, Enumeration::Order order = Enumeration::Order::Id, bool *ok = nullptr) const noexcept = 0;
};

#endif // ENUMERATIONDAOINTF_H
