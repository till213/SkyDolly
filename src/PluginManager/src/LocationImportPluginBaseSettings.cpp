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
#include <string_view>

#include <Kernel/Settings.h>
#include "LocationImportPluginBaseSettings.h"

namespace
{
    // Keys
    constexpr const char *ImportDirectoryEnabledKey {"ImportDirectoryEnabled"};
    constexpr const char *AddToFlightEnabledKey {"AddToFlightEnabled"};

    // Defaults
    constexpr bool DefaultImportDirectoryEnabled = false;
    constexpr bool DefaultAddToFlightEnabled = false;
}

struct LocationImportPluginBaseSettingsPrivate
{
    bool importDirectoryEnabled {::DefaultImportDirectoryEnabled};
};

// PUBLIC

LocationImportPluginBaseSettings::LocationImportPluginBaseSettings() noexcept
    : d(std::make_unique<LocationImportPluginBaseSettingsPrivate>())
{}

LocationImportPluginBaseSettings::~LocationImportPluginBaseSettings() = default;

bool LocationImportPluginBaseSettings::isImportDirectoryEnabled() const noexcept
{
    return d->importDirectoryEnabled;
}

void LocationImportPluginBaseSettings::setImportDirectoryEnabled(bool enabled) noexcept
{
    if (d->importDirectoryEnabled != enabled) {
        d->importDirectoryEnabled = enabled;
        emit baseSettingsChanged();
    }
}

void LocationImportPluginBaseSettings::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ImportDirectoryEnabledKey;
    keyValue.second = d->importDirectoryEnabled;
    keyValues.push_back(keyValue);

    addSettingsExtn(keyValues);
}

void LocationImportPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ImportDirectoryEnabledKey;
    keyValue.second = ::DefaultImportDirectoryEnabled;
    keysWithDefaults.push_back(keyValue);

    addKeysWithDefaultsExtn(keysWithDefaults);
}

void LocationImportPluginBaseSettings::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->importDirectoryEnabled = valuesByKey.at(::ImportDirectoryEnabledKey).toBool();
    emit baseSettingsChanged();

    restoreSettingsExtn(valuesByKey);
}

void LocationImportPluginBaseSettings::restoreDefaults() noexcept
{
    d->importDirectoryEnabled = ::DefaultImportDirectoryEnabled;
    emit baseSettingsChanged();

    restoreDefaultsExtn();
}
