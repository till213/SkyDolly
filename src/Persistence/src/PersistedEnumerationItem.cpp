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
#include <memory.h>
#include <cstdint>

#include <QString>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Model/Data.h>
#include <Model/Enumeration.h>
#include "Service/EnumerationService.h"
#include "PersistedEnumerationItem.h"

struct PersistedEnumerationItemPrivate
{
    PersistedEnumerationItemPrivate(QString enumerationName, QString symbolicId)
        : enumeration(enumerationName)
    {
        if (enumerationService.getEnumerationByName(enumeration)) {
            id = enumeration.getItemBySymbolicId(symbolicId).id;
        }
    }

    Enumeration enumeration;
    std::int64_t id {Data::InvalidId};
    EnumerationService enumerationService;
};

// PUBLIC

PersistedEnumerationItem::PersistedEnumerationItem(QString enumerationName, QString symbolicId) noexcept
    : d(std::make_unique<PersistedEnumerationItemPrivate>(enumerationName, symbolicId))
{
#ifdef DEBUG
    qDebug() << "PersistedEnumerationItem::PersistedEnumerationItem: CREATED, name:" << enumerationName << "ID:" << d->id;
#endif
}

PersistedEnumerationItem::~PersistedEnumerationItem() noexcept
{
#ifdef DEBUG
    qDebug() << "PersistedEnumerationItem::~PersistedEnumerationItem: DELETED, name:" << d->enumeration.getName() << "ID:" << d->id;
#endif
}

std::int64_t PersistedEnumerationItem::id() const noexcept
{
    return d->id;
}