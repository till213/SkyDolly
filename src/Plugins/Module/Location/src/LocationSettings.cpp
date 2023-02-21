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
#include <unordered_set>
#include <utility>

#include <QList>
#include <QVariant>

#include <Kernel/Const.h>
#include <Kernel/Settings.h>
#include <Persistence/LocationSelector.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/Service/EnumerationService.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "LocationSettings.h"

namespace
{
    // Keys
    constexpr const char *LocationTypeSelectionKey {"LocationTypeSelection"};
    constexpr const char *LocationCategorySelectionKey {"LocationCategorySelection"};
    constexpr const char *CountrySelectionKey {"CountrySelection"};
    constexpr const char *LocationTableStateKey {"LocationTableState"};
}

struct LocationSettingsPrivate
{
    LocationSelector::TypeSelection typeSelection;
    std::int64_t categoryId {Const::InvalidId};
    std::int64_t countryId {Const::InvalidId};
    QByteArray locationTableState;

    // Defaults
    const std::int64_t DefaultLocationCategoryId {PersistedEnumerationItem(EnumerationService::LocationCategory, EnumerationService::LocationCategoryNoneSymId).id()};
    const std::int64_t DefaultCountryId {PersistedEnumerationItem(EnumerationService::Country, EnumerationService::CountryWorldSymId).id()};
};

// PUBLIC

LocationSettings::LocationSettings() noexcept
    : ModuleBaseSettings(),
      d(std::make_unique<LocationSettingsPrivate>())
{}

LocationSettings::~LocationSettings() = default;

LocationSelector::TypeSelection LocationSettings::getTypeSelection() const noexcept
{
    return d->typeSelection;
}

void LocationSettings::setTypeSelection(LocationSelector::TypeSelection typeSelection) noexcept
{
    d->typeSelection = std::move(typeSelection);
}

std::int64_t LocationSettings::getCategoryId() const noexcept
{
    return d->categoryId;
}

void LocationSettings::setCategoryId(std::int64_t categoryIdd) noexcept
{
    d->categoryId = categoryIdd;
}

std::int64_t LocationSettings::getCountryId() const noexcept
{
    return d->countryId;
}

void LocationSettings::setCountryId(std::int64_t countryId) noexcept
{
    d->countryId = countryId;
}

QByteArray LocationSettings::getLocationTableState() const
{
    return d->locationTableState;
}

void LocationSettings::setLocationTableState(QByteArray state) noexcept
{
    d->locationTableState = std::move(state);
}

// PROTECTED

void LocationSettings::addSettingsExtn([[maybe_unused]] Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::LocationTypeSelectionKey;
    QList<QVariant> typeList;
    for (const auto it : d->typeSelection) {
        typeList.append(it);
    }
    keyValue.second = QVariant::fromValue(typeList);
    keyValues.push_back(keyValue);

    keyValue.first = ::LocationCategorySelectionKey;
    keyValue.second = d->categoryId;
    keyValues.push_back(keyValue);

    keyValue.first = ::CountrySelectionKey;
    keyValue.second = d->countryId;
    keyValues.push_back(keyValue);

    keyValue.first = ::LocationTableStateKey;
    keyValue.second = d->locationTableState;
    keyValues.push_back(keyValue);
}

void LocationSettings::addKeysWithDefaultsExtn([[maybe_unused]] Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::LocationTypeSelectionKey;
    keyValue.second = QVariant::fromValue(QList<QVariant>());
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::LocationCategorySelectionKey;
    keyValue.second = d->DefaultLocationCategoryId;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::CountrySelectionKey;
    keyValue.second = d->DefaultCountryId;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::LocationTableStateKey;
    keyValue.second = QByteArray();
    keysWithDefaults.push_back(keyValue);
}

void LocationSettings::restoreSettingsExtn([[maybe_unused]] const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->typeSelection.clear();
    QList<QVariant> typeList = valuesByKey.at(::LocationTypeSelectionKey).toList();
    for (const QVariant &v : typeList) {
        d->typeSelection.insert(v.toLongLong());
    }
    bool ok {false};
    d->categoryId = valuesByKey.at(::LocationCategorySelectionKey).toLongLong(&ok);
    if (!ok) {
        d->categoryId = d->DefaultLocationCategoryId;
    }
    d->countryId = valuesByKey.at(::CountrySelectionKey).toLongLong(&ok);
    if (!ok) {
        d->countryId = d->DefaultCountryId;
    }
    d->locationTableState = valuesByKey.at(::LocationTableStateKey).toByteArray();
}

void LocationSettings::restoreDefaultsExtn() noexcept
{
    d->typeSelection.clear();
    d->categoryId = d->DefaultLocationCategoryId;
    d->countryId = d->DefaultCountryId;
    d->locationTableState = {};
}
