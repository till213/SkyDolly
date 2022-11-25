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

#include <QComboBox>

#include <Model/Enumeration.h>
#include <Persistence/Service/EnumerationService.h>
#include "EnumerationComboBox.h"

struct EnumerationComboBoxPrivate
{
    EnumerationComboBoxPrivate(QString enumerationName = QString())
        : enumeration(std::move(enumerationName))
    {}

    Enumeration enumeration;
    EnumerationService enumerationService;
    EnumerationComboBox::IgnoredIds ignoredIds;
};

// PUBLIC

EnumerationComboBox::EnumerationComboBox(QString enumerationName, QWidget *parent) noexcept
    : EnumerationComboBox(parent)

{
    d->enumeration = {std::move(enumerationName)};
    initUi();
}

EnumerationComboBox::EnumerationComboBox(QWidget *parent) noexcept
    : QComboBox(parent),
      d(std::make_unique<EnumerationComboBoxPrivate>())
{}

EnumerationComboBox::~EnumerationComboBox() = default;

QString EnumerationComboBox::getEnumerationName() const
{
    return d->enumeration.getName();
}

void EnumerationComboBox::setEnumerationName(QString name) noexcept
{
    d->enumeration.setName(std::move(name));
    initUi();
}

std::int64_t EnumerationComboBox::getCurrentId() const noexcept
{
    return currentData().toLongLong();
}

void EnumerationComboBox::setCurrentId(std::int64_t id) noexcept
{
    for (int index = 0; index < count(); ++index) {
        if (itemData(index).toLongLong() == id) {
            setCurrentIndex(index);
            break;
        }
    }
}

EnumerationComboBox::IgnoredIds EnumerationComboBox::getIgnoredIds() const noexcept
{
    return d->ignoredIds;
}

void EnumerationComboBox::setIgnoredIds(IgnoredIds ignoredIds) noexcept
{
    d->ignoredIds = ignoredIds;
}

// PRIVATE

void EnumerationComboBox::initUi() noexcept
{
    setAutoFillBackground(true);
    bool ok {true};
    d->enumeration = d->enumerationService.getEnumerationByName(d->enumeration.getName(), &ok);
    if (ok)  {
        for (const auto &item : d->enumeration) {
            if (d->ignoredIds.find(item.id) == d->ignoredIds.cend()) {
                addItem(item.name, QVariant::fromValue(item.id));
            }
        }
    }
}
