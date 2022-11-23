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
#include <utility>
#include <vector>
#include <unordered_map>

#include <QString>
#include <QHash>

#include "Enumeration.h"

struct EnumerationPrivate
{
    EnumerationPrivate(QString name = QString()) noexcept
        : name(std::move(name))
    {}

    QString name;
    std::vector<Enumeration::Item> items;
    // Stores the index into items, indexed by the symbolic ID
    std::unordered_map<QString, std::size_t> itemsBySymbolicId;
    // Stores the index into items, indexed by the internal ID
    std::unordered_map<std::int64_t, std::size_t> itemsById;
};

// PUBLIC

Enumeration::Enumeration(QString name) noexcept
    : Enumeration()
{
    d->name = std::move(name);
}

Enumeration::Enumeration() noexcept
    : d(std::make_unique<EnumerationPrivate>())
{}

Enumeration::Enumeration(Enumeration &&rhs) noexcept = default;
Enumeration &Enumeration::operator=(Enumeration &&rhs) noexcept = default;
Enumeration::~Enumeration() = default;

QString Enumeration::getName() const noexcept
{
    return d->name;
}

void Enumeration::setName(QString name) noexcept
{
    d->name = std::move(name);
}

void Enumeration::addItem(const Item& item) noexcept
{
    d->items.push_back(item);
    const std::size_t index = d->items.size() - 1;
    d->itemsBySymbolicId[item.symbolicId] = index;
    d->itemsById[item.id] = index;
}

const std::vector<Enumeration::Item> &Enumeration::items() const noexcept
{
    return d->items;
}

Enumeration::Item Enumeration::getItemBySymbolicId(const QString &symbolicId) const noexcept
{
    auto it = d->itemsBySymbolicId.find(symbolicId);
    if (it != d->itemsBySymbolicId.end()) {
        return d->items[it->second];
    }
    return {};
}

Enumeration::Item Enumeration::getItemById(std::int64_t id) const noexcept
{
    auto it = d->itemsById.find(id);
    if (it != d->itemsById.end()) {
        return d->items[it->second];
    }
    return {};
}

QString Enumeration::getSymbolicIdById(std::int64_t id) const noexcept
{
    return getItemById(id).symbolicId;
}

std::size_t Enumeration::count() const noexcept
{
    return d->items.size();
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
