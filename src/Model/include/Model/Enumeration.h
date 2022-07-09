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
#ifndef ENUMERATION_H
#define ENUMERATION_H

#include <memory>
#include <vector>
#include <unordered_map>

#include <QString>

#include "InitialPosition.h"
#include "Data.h"
#include "ModelLib.h"

struct EnumerationPrivate;

/*!
 * The model for the corresonding database enumeration tables.
 *
 * For C++ enumeration class support also refer to Enum.h
 *
 * \sa Enum#toUnderlyingType
 */
class MODEL_API Enumeration
{
public:

    Enumeration(QString name) noexcept;
    Enumeration(Enumeration &&other) noexcept = default;
    ~Enumeration() noexcept;
    Enumeration &operator=(Enumeration &&rhs) noexcept = default;

    using Item = struct Item_ : public Data
    {
        Item_() noexcept
            : Data()
        {}

        Item_(std::atomic_int64_t id, QString theInternalId, QString theName) noexcept
            : Data(id), internalId(theInternalId), name(theName)
        {}
        QString internalId;
        QString name;
    };

    QString getName() const noexcept;
    void addItem(Item item) noexcept;
    const std::vector<Item> &items() const noexcept;
    Item itemByInternalId(QString internalId) const noexcept;

private:
    std::unique_ptr<EnumerationPrivate> d;
};

#endif // ENUMERATION_H
