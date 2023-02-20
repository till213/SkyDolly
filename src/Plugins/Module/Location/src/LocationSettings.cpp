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

#include <Kernel/Settings.h>
#include <PluginManager/Module/ModuleBaseSettings.h>
#include "LocationSettings.h"

namespace
{
    // Keys
    constexpr const char *locationTableStateKey {"LocationTableState"};
}

struct LocationSettingsPrivate
{
    QByteArray locationTableState;
};

// PUBLIC

LocationSettings::LocationSettings() noexcept
    : ModuleBaseSettings(),
      d(std::make_unique<LocationSettingsPrivate>())
{}

LocationSettings::~LocationSettings() = default;

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

    keyValue.first = ::locationTableStateKey;
    keyValue.second = d->locationTableState;
    keyValues.push_back(keyValue);
}

void LocationSettings::addKeysWithDefaultsExtn([[maybe_unused]] Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::locationTableStateKey;
    keyValue.second = QByteArray();
    keysWithDefaults.push_back(keyValue);
}

void LocationSettings::restoreSettingsExtn([[maybe_unused]] const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->locationTableState = valuesByKey.at(::locationTableStateKey).toByteArray();
}

void LocationSettings::restoreDefaultsExtn() noexcept
{
    d->locationTableState = {};
}
