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
#ifndef ENUMERATION_H
#define ENUMERATION_H

#include <memory>
#include <utility>
#include <vector>
#include <cstdint>

#include <QString>

#include "Data.h"
#include "ModelLib.h"

struct EnumerationPrivate;

/*!
 * The model for the corresonding database enumeration tables.
 *
 * For C++ enumeration class support also refer to Enum.h
 *
 * \sa Enum#underly
 */
class MODEL_API Enumeration final
{
public:
    Enumeration(QString name) noexcept;
    Enumeration() noexcept;
    Enumeration(const Enumeration &rhs) = delete;
    Enumeration(Enumeration &&rhs) noexcept;
    Enumeration &operator=(const Enumeration &rhs) = delete;
    Enumeration &operator=(Enumeration &&rhs) noexcept;
    ~Enumeration();

    enum struct Order: std::uint8_t {
        Id,
        SymId,
        Name
    };

    using Item = struct Item_ : public Data
    {
        Item_() noexcept
            : Data()
        {}

        Item_(std::atomic_int64_t id, QString symId, QString name) noexcept
            : Data(id), symId(std::move(symId)), name(std::move(name))
        {}
        QString symId;
        QString name;
    };

    QString getName() const noexcept;
    void setName(QString name) noexcept;
    void addItem(const Item& item) noexcept;
    const std::vector<Item> &items() const noexcept;
    Item getItemBySymId(const QString &symId) const noexcept;
    Item getItemById(std::int64_t id) const noexcept;
    QString getSymIdById(std::int64_t id) const noexcept;

    std::size_t count() const noexcept;

    using Iterator = std::vector<Item>::iterator;
    Iterator begin() noexcept;
    Iterator end() noexcept;
    const Iterator begin() const noexcept;
    const Iterator end() const noexcept;

private:
    std::unique_ptr<EnumerationPrivate> d;
};

#endif // ENUMERATION_H
