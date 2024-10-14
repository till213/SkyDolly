/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

#include <QString>
#include <QDate>

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Model/SimType.h>
#include <Persistence/FlightSelector.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "LogbookSettings.h"

namespace
{
    // Keys
    constexpr const char *HasFormationKey {"HasFormation"};
    constexpr const char *EngineTypeKey {"EngineTypeKey"};
    constexpr const char *MinimumDurationMinutesKey {"MinimumDurationMinutes"};

constexpr const char *LogbookTableStateKey {"LogbookTableState"};

    // Defaults
    constexpr bool DefaultHasFormation {false};
    constexpr SimType::EngineType DefaultEngineType {SimType::EngineType::All};
    constexpr std::int64_t DefaultMinimumDurationMinutes {0};
}

struct LogbookSettingsPrivate
{
    // Note: search keywords and from/to dates are deliberately not persisted in the settings
    FlightSelector flightSelector;

    QByteArray logbookTableState;
};

// PUBLIC

LogbookSettings::LogbookSettings() noexcept
    : ModuleBaseSettings(),
      d {std::make_unique<LogbookSettingsPrivate>()}
{}

LogbookSettings::~LogbookSettings() = default;

QDate LogbookSettings::getFromDate() const noexcept
{
    return d->flightSelector.fromDate;
}

void LogbookSettings::setFromDate(QDate from) noexcept
{
    if (d->flightSelector.fromDate != from) {
        d->flightSelector.fromDate = from;
        emit changed();
    }
}

QDate LogbookSettings::getToDate() const noexcept
{
    return d->flightSelector.toDate;
}

void LogbookSettings::setToDate(QDate to) noexcept
{
    if (d->flightSelector.toDate != to) {
        d->flightSelector.toDate = to;
        emit changed();
    }
}

const QString &LogbookSettings::getSearchKeyword() const noexcept
{
    return d->flightSelector.searchKeyword;
}

void LogbookSettings::setSearchKeyword(QString keyword) noexcept
{
    if (d->flightSelector.searchKeyword != keyword) {
        d->flightSelector.searchKeyword = std::move(keyword);
        emit changed();
    }
}

bool LogbookSettings::hasFormation() const noexcept
{
    return d->flightSelector.hasFormation;
}

void LogbookSettings::setFormation(bool enable) noexcept
{
    if (d->flightSelector.hasFormation != enable) {
        d->flightSelector.hasFormation = enable;
        emit changed();
    }
}

SimType::EngineType LogbookSettings::getEngineType() const noexcept
{
    return d->flightSelector.engineType;
}

void LogbookSettings::setEngineType(SimType::EngineType engineType) noexcept
{
    if (d->flightSelector.engineType != engineType) {
        d->flightSelector.engineType = engineType;
        emit changed();
    }
}

int LogbookSettings::getMinimumDurationMinutes() const noexcept
{
    return d->flightSelector.mininumDurationMinutes;
}

void LogbookSettings::setMinimumDurationMinutes(int minutes) noexcept
{
    if (d->flightSelector.mininumDurationMinutes != minutes) {
        d->flightSelector.mininumDurationMinutes = minutes;
        emit changed();
    }
}

const FlightSelector &LogbookSettings::getFlightSelector() const noexcept
{
    return d->flightSelector;
}

QByteArray LogbookSettings::getLogbookTableState() const
{
    return d->logbookTableState;
}

void LogbookSettings::setLogbookTableState(QByteArray state) noexcept
{
    d->logbookTableState = std::move(state);
}

void LogbookSettings::resetFilter() noexcept
{
    restoreFilter();
    emit changed();
}

// PROTECTED

void LogbookSettings::addSettingsExtn([[maybe_unused]] Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::HasFormationKey;
    keyValue.second = d->flightSelector.hasFormation;
    keyValues.push_back(keyValue);

    keyValue.first = ::EngineTypeKey;
    keyValue.second = Enum::underly(d->flightSelector.engineType);
    keyValues.push_back(keyValue);

    keyValue.first = ::MinimumDurationMinutesKey;
    keyValue.second = d->flightSelector.mininumDurationMinutes;
    keyValues.push_back(keyValue);

    keyValue.first = ::LogbookTableStateKey;
    keyValue.second = d->logbookTableState;
    keyValues.push_back(keyValue);
}

void LogbookSettings::addKeysWithDefaultsExtn([[maybe_unused]] Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::HasFormationKey;
    keyValue.second = ::DefaultHasFormation;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::EngineTypeKey;
    keyValue.second = Enum::underly(DefaultEngineType);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::MinimumDurationMinutesKey;
    keyValue.second = QVariant::fromValue(::DefaultMinimumDurationMinutes);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::LogbookTableStateKey;
    keyValue.second = QByteArray();
    keysWithDefaults.push_back(keyValue);
}

void LogbookSettings::restoreSettingsExtn([[maybe_unused]] const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {false};
    d->flightSelector.hasFormation = valuesByKey.at(::HasFormationKey).toBool();

    auto enumValue = valuesByKey.at(::EngineTypeKey).toInt(&ok);
    d->flightSelector.engineType = ok && Enum::contains<SimType::EngineType>(enumValue) ? static_cast<SimType::EngineType>(enumValue) : ::DefaultEngineType;

    d->flightSelector.mininumDurationMinutes = valuesByKey.at(::MinimumDurationMinutesKey).toInt(&ok);
    if (!ok) {
        d->flightSelector.mininumDurationMinutes = ::DefaultMinimumDurationMinutes;
    }

    d->logbookTableState = valuesByKey.at(::LogbookTableStateKey).toByteArray();
}

void LogbookSettings::restoreDefaultsExtn() noexcept
{
    restoreFilter();
    d->logbookTableState = {};
}

// PRIVATE

void LogbookSettings::restoreFilter() noexcept
{
    d->flightSelector.hasFormation = ::DefaultHasFormation;
    d->flightSelector.engineType = ::DefaultEngineType;
    d->flightSelector.mininumDurationMinutes = ::DefaultMinimumDurationMinutes;
}
