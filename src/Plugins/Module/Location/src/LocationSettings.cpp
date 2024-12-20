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
#include <memory>
#include <utility>

#include <QList>
#include <QVariant>

#include <Kernel/Const.h>
#include <Kernel/Enum.h>
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

    constexpr const char *DefaultAltitudeKey {"DefaultAltitude"};
    constexpr const char *DefaultIndicatedAirspeedKey {"DefaultIndicatedAirspeed"};
    constexpr const char *DefaultEngineEventKey {"DefaultEngineEvent"};
    constexpr const char *DefaultOnGroundKey {"DefaultOnGround"};

    constexpr const char *DateSelectionKey {"DateSelection"};
    constexpr const char *DateKey {"Date"};
    constexpr const char *TimeSelectionKey {"TimeSelection"};

    constexpr const char *LocationTableStateKey {"LocationTableState"};

    // Defaults
    constexpr std::int64_t DefaultCategoryId {Const::InvalidId};
    constexpr std::int64_t DefaultCountryId {Const::InvalidId};

    constexpr int DefaultAltitude {Const::DefaultAltitude};
    constexpr int DefaultIndicatedAirspeed {Const::DefaultIndicatedAirspeed};
    constexpr bool DefaultOnGround {false};

    constexpr LocationSettings::DateSelection DefaultDateSelection {LocationSettings::DateSelection::Today};
    constexpr LocationSettings::TimeSelection DefaultTimeSelection {LocationSettings::TimeSelection::Now};
}

struct LocationSettingsPrivate
{
    // Note: search keywords are deliberately not persisted in the settings
    LocationSelector locationSelector;
    int altitude {::DefaultAltitude};
    int indicatedAirspeed {::DefaultIndicatedAirspeed};
    std::int64_t engineEventId {Const::InvalidId};
    bool onGround {::DefaultOnGround};

    LocationSettings::DateSelection dateSelection {::DefaultDateSelection};
    QDate date {DefaultDate};
    LocationSettings::TimeSelection timeSelection {::DefaultTimeSelection};

    QByteArray locationTableState;

    const std::int64_t DefaultEngineEventId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventKeepSymId).id()};
    static inline const QDate DefaultDate {QDate::currentDate()};
};

// PUBLIC

LocationSettings::LocationSettings() noexcept
    : ModuleBaseSettings(),
      d {std::make_unique<LocationSettingsPrivate>()}
{}

LocationSettings::~LocationSettings() = default;

LocationSelector::TypeSelection LocationSettings::getTypeSelection() const noexcept
{
    return d->locationSelector.typeSelection;
}

void LocationSettings::setTypeSelection(LocationSelector::TypeSelection typeSelection) noexcept
{
    if (d->locationSelector.typeSelection != typeSelection) {
        d->locationSelector.typeSelection = std::move(typeSelection);
        emit changed();
    }
}

std::int64_t LocationSettings::getCategoryId() const noexcept
{
    return d->locationSelector.categoryId;
}

void LocationSettings::setCategoryId(std::int64_t categoryId) noexcept
{
    if (d->locationSelector.categoryId != categoryId) {
        d->locationSelector.categoryId = categoryId;
        emit changed();
    }
}

std::int64_t LocationSettings::getCountryId() const noexcept
{
    return d->locationSelector.countryId;
}

void LocationSettings::setCountryId(std::int64_t countryId) noexcept
{
    if (d->locationSelector.countryId != countryId) {
        d->locationSelector.countryId = countryId;
        emit changed();
    }
}

const QString &LocationSettings::getSearchKeyword() const noexcept
{
    return d->locationSelector.searchKeyword;
}

void LocationSettings::setSearchKeyword(QString keyword) noexcept
{
    if (d->locationSelector.searchKeyword != keyword) {
        d->locationSelector.searchKeyword = std::move(keyword);
        emit changed();
    }
}

bool LocationSettings::showUserLocations() const noexcept
{
    return d->locationSelector.showUserLocations();
}

bool LocationSettings::hasSelectors() const noexcept
{
    return d->locationSelector.hasSelectors();
}

const LocationSelector &LocationSettings::getLocationSelector() const noexcept
{
    return d->locationSelector;
}

int LocationSettings::getDefaultAltitude() const noexcept
{
    return d->altitude;
}

void LocationSettings::setDefaultAltitude(int altitude) noexcept
{
    if (d->altitude != altitude) {
        d->altitude = altitude;
        emit changed();
    }
}

int LocationSettings::getDefaultIndicatedAirspeed() const noexcept
{
    return d->indicatedAirspeed;
}

void LocationSettings::setDefaultIndicatedAirspeed(int airspeed)
{
    if (d->indicatedAirspeed != airspeed) {
        d->indicatedAirspeed = airspeed;
        emit changed();
    }
}

const std::int64_t LocationSettings::getDefaultEngineEventId() const noexcept
{
    return d->engineEventId;
}

void LocationSettings::setDefaultEngineEventId(std::int64_t eventId) noexcept
{
    if (d->engineEventId != eventId) {
        d->engineEventId = eventId;
        emit changed();
    }
}

bool LocationSettings::isDefaultOnGround() const noexcept
{
    return d->onGround;
}

void LocationSettings::setDefaultOnGround(bool enable) noexcept
{
    if (d->onGround != enable) {
        d->onGround = enable;
        emit changed();
    }
}

const LocationSettings::DateSelection LocationSettings::getDateSelection() const noexcept
{
    return d->dateSelection;
}

void LocationSettings::setDateSelection(DateSelection dateSelection) noexcept
{
    if (d->dateSelection != dateSelection) {
        d->dateSelection = dateSelection;
        emit changed();
    }
}

const QDate LocationSettings::getDate() const noexcept
{
    return d->date;
}

void LocationSettings::setDate(QDate date) noexcept
{
    if (d->date != date) {
        d->date = date;
        emit changed();
    }
}

const LocationSettings::TimeSelection LocationSettings::getTimeSelection() const noexcept
{
    return d->timeSelection;
}

void LocationSettings::setTimeSelection(TimeSelection timeSelection) noexcept
{
    if (d->timeSelection != timeSelection) {
        d->timeSelection = timeSelection;
        emit changed();
    }
}

QByteArray LocationSettings::getLocationTableState() const
{
    return d->locationTableState;
}

void LocationSettings::setLocationTableState(QByteArray state) noexcept
{
    d->locationTableState = std::move(state);
}

void LocationSettings::resetFilter() noexcept
{
    restoreFilter();
    emit changed();
}

void LocationSettings::resetDefaultValues() noexcept
{
    restoreDefaultValues();
    emit changed();
}

// PROTECTED

void LocationSettings::addSettingsExtn([[maybe_unused]] Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    // Filters
    keyValue.first = ::LocationTypeSelectionKey;
    QList<QVariant> typeList;
    typeList.reserve(static_cast<qsizetype>(d->locationSelector.typeSelection.size()));
    for (const auto it : d->locationSelector.typeSelection) {
        typeList.append(QVariant::fromValue(it));
    }
    keyValue.second = QVariant::fromValue(typeList);
    keyValues.push_back(keyValue);

    keyValue.first = ::LocationCategorySelectionKey;
    keyValue.second = QVariant::fromValue(d->locationSelector.categoryId);
    keyValues.push_back(keyValue);

    keyValue.first = ::CountrySelectionKey;
    keyValue.second = QVariant::fromValue(d->locationSelector.countryId);
    keyValues.push_back(keyValue);

    // Date and time
    keyValue.first = ::DateSelectionKey;
    keyValue.second = Enum::underly(d->dateSelection);
    keyValues.push_back(keyValue);

    keyValue.first = ::DateKey;
    keyValue.second = QVariant::fromValue(d->date);
    keyValues.push_back(keyValue);

    keyValue.first = ::TimeSelectionKey;
    keyValue.second = Enum::underly(d->timeSelection);
    keyValues.push_back(keyValue);

    // Default values
    keyValue.first = ::DefaultAltitudeKey;
    keyValue.second = d->altitude;
    keyValues.push_back(keyValue);

    keyValue.first = ::DefaultIndicatedAirspeedKey;
    keyValue.second = d->indicatedAirspeed;
    keyValues.push_back(keyValue);

    keyValue.first = ::DefaultEngineEventKey;
    keyValue.second = QVariant::fromValue(d->engineEventId);
    keyValues.push_back(keyValue);

    keyValue.first = ::DefaultOnGroundKey;
    keyValue.second = d->onGround;
    keyValues.push_back(keyValue);

    // Table state
    keyValue.first = ::LocationTableStateKey;
    keyValue.second = d->locationTableState;
    keyValues.push_back(keyValue);
}

void LocationSettings::addKeysWithDefaultsExtn([[maybe_unused]] Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    // Filters
    keyValue.first = ::LocationTypeSelectionKey;
    keyValue.second = QVariant::fromValue(QList<QVariant>());
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::LocationCategorySelectionKey;
    keyValue.second = QVariant::fromValue(::DefaultCategoryId);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::CountrySelectionKey;
    keyValue.second = QVariant::fromValue(::DefaultCountryId);
    keysWithDefaults.push_back(keyValue);

    // Date and time
    keyValue.first = ::DateSelectionKey;
    keyValue.second = Enum::underly(::DefaultDateSelection);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::DateKey;
    keyValue.second = QVariant::fromValue(LocationSettingsPrivate::DefaultDate);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::TimeSelectionKey;
    keyValue.second = Enum::underly(::DefaultTimeSelection);
    keysWithDefaults.push_back(keyValue);

    // Default values
    keyValue.first = ::DefaultAltitudeKey;
    keyValue.second = ::DefaultAltitude;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::DefaultIndicatedAirspeedKey;
    keyValue.second = ::DefaultIndicatedAirspeed;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::DefaultEngineEventKey;
    keyValue.second = QVariant::fromValue(d->DefaultEngineEventId);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::DefaultOnGroundKey;
    keyValue.second = ::DefaultOnGround;
    keysWithDefaults.push_back(keyValue);

    // Table state
    keyValue.first = ::LocationTableStateKey;
    keyValue.second = QByteArray();
    keysWithDefaults.push_back(keyValue);
}

void LocationSettings::restoreSettingsExtn([[maybe_unused]] const Settings::ValuesByKey &valuesByKey) noexcept
{
    // Filters
    d->locationSelector.typeSelection.clear();
    QList<QVariant> typeList = valuesByKey.at(::LocationTypeSelectionKey).toList();
    d->locationSelector.typeSelection.reserve(typeList.count());
    for (const auto &variant : typeList) {
        d->locationSelector.typeSelection.insert(variant.toLongLong());
    }
    bool ok {false};
    d->locationSelector.categoryId = valuesByKey.at(::LocationCategorySelectionKey).toLongLong(&ok);
    if (!ok) {
        d->locationSelector.categoryId = ::DefaultCategoryId;
    }
    d->locationSelector.countryId = valuesByKey.at(::CountrySelectionKey).toLongLong(&ok);
    if (!ok) {
        d->locationSelector.countryId = ::DefaultCountryId;
    }

    // Date and time
    ok = true;
    auto enumValue = valuesByKey.at(::DateSelectionKey).toInt(&ok);
    d->dateSelection = ok && Enum::contains<DateSelection>(enumValue) ? static_cast<DateSelection>(enumValue) : ::DefaultDateSelection;

    d->date = valuesByKey.at(::DateKey).toDate();
    if (!d->date.isValid()) {
        d->date = LocationSettingsPrivate::DefaultDate;
    }

    enumValue = valuesByKey.at(::TimeSelectionKey).toInt(&ok);
    d->timeSelection = ok && Enum::contains<TimeSelection>(enumValue) ? static_cast<TimeSelection>(enumValue) : ::DefaultTimeSelection;

    // Default values
    d->altitude = valuesByKey.at(::DefaultAltitudeKey).toInt(&ok);
    if (!ok) {
        d->altitude = ::DefaultAltitude;
    }
    d->indicatedAirspeed = valuesByKey.at(::DefaultIndicatedAirspeedKey).toInt(&ok);
    if (!ok) {
        d->indicatedAirspeed = ::DefaultIndicatedAirspeed;
    }
    d->engineEventId = valuesByKey.at(::DefaultEngineEventKey).toLongLong(&ok);
    if (!ok) {
        d->engineEventId = d->DefaultEngineEventId;
    }
    d->onGround = valuesByKey.at(::DefaultOnGroundKey).toBool();

    // Table state
    d->locationTableState = valuesByKey.at(::LocationTableStateKey).toByteArray();
}

void LocationSettings::restoreDefaultsExtn() noexcept
{
    restoreFilter();
    resetDefaultValues();
    d->locationTableState = {};
}

// PRIVATE

void LocationSettings::restoreFilter() noexcept
{
    d->locationSelector.clear();
}

void LocationSettings::restoreDefaultValues() noexcept
{
    d->altitude = ::DefaultAltitude;
    d->indicatedAirspeed = ::DefaultIndicatedAirspeed;
    d->engineEventId = d->DefaultEngineEventId;
    d->onGround = ::DefaultOnGround;
    d->dateSelection = ::DefaultDateSelection;
    d->timeSelection = ::DefaultTimeSelection;
}
