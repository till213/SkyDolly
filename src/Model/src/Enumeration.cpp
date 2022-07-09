/**
 * Sky Dolly - The Black Sheep for your Enumeration Recordings
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
#include <vector>
#include <unordered_map>

#include <QString>
#ifdef DEBUG
#include <QDebug>
#endif

#include "Enumeration.h"

struct EnumerationPrivate
{
    EnumerationPrivate(QString theName) noexcept
        : name(theName)
    {}

    QString name;
    std::vector<Enumeration::Item> items;
    // Stores the index into items, indexed by the internal ID
    std::unordered_map<QString, std::size_t> itemsByInternalId;
};

// PUBLIC

Enumeration::Enumeration(QString name) noexcept
    : d(std::make_unique<EnumerationPrivate>(name))
{
#ifdef DEBUG
    qDebug() << "Enumeration::Enumeration: CREATED, name:" << d->name;
#endif
}

Enumeration::~Enumeration() noexcept
{
#ifdef DEBUG
    qDebug() << "Enumeration::Enumeration: DELETED, name:" << d->name;
#endif
}

QString Enumeration::getName() const noexcept
{
    return d->name;
}

void Enumeration::addItem(Item item) noexcept
{
    d->items.push_back(item);
    d->itemsByInternalId[item.internalId] = d->items.size() - 1;
}

const std::vector<Enumeration::Item> &Enumeration::items() const noexcept
{
    return d->items;
}

Enumeration::Item Enumeration::itemByInternalId(QString internalId) const noexcept
{
    auto it = d->itemsByInternalId.find(internalId);
    if (it != d->itemsByInternalId.end()) {
        return d->items[it->second];
    }
    return Enumeration::Item();
}

Enumeration::Iterator Enumeration::begin() noexcept
{
    return d->items.begin();
}

Enumeration::Iterator Enumeration::end() noexcept
{
    return d->items.end();
}

const Enumeration::Iterator Enumeration::begin() const noexcept
{
    return d->items.begin();
}

const Enumeration::Iterator Enumeration::end() const noexcept
{
    return d->items.end();
}
