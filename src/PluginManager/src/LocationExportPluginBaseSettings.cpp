/**
 * Sky Dolly - The Black Sheep for Your Location Recordings
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

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Kernel/SampleRate.h>
#include "LocationExportPluginBaseSettings.h"

namespace
{
    // Keys
    constexpr const char *ResamplingPeriodKey {"ResamplingPeriod"};
    constexpr const char *FormationExportKey {"FormationExport"};
    constexpr const char *OpenExportedFilesEnabledKey {"OpenExportedFilesEnabled"};

    // Defaults
    constexpr bool DefaultOpenExportedFilesEnabled {false};
}

struct LocationExportPluginBaseSettingsPrivate
{
    bool openExportedFilesEnabled {::DefaultOpenExportedFilesEnabled};
};

// PUBLIC

LocationExportPluginBaseSettings::LocationExportPluginBaseSettings() noexcept
    : d(std::make_unique<LocationExportPluginBaseSettingsPrivate>())
{}

LocationExportPluginBaseSettings::~LocationExportPluginBaseSettings() = default;

bool LocationExportPluginBaseSettings::isOpenExportedFilesEnabled() const noexcept
{
    return d->openExportedFilesEnabled;
}

void LocationExportPluginBaseSettings::setOpenExportedFilesEnabled(bool enabled) noexcept
{
    if (d->openExportedFilesEnabled != enabled) {
        d->openExportedFilesEnabled = enabled;
        emit baseSettingsChanged();
    }
}

void LocationExportPluginBaseSettings::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::OpenExportedFilesEnabledKey;
    keyValue.second = d->openExportedFilesEnabled;
    keyValues.push_back(keyValue);

    addSettingsExtn(keyValues);
}

void LocationExportPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::OpenExportedFilesEnabledKey;
    keyValue.second = ::DefaultOpenExportedFilesEnabled;
    keysWithDefaults.push_back(keyValue);

    addKeysWithDefaultsExtn(keysWithDefaults);
}

void LocationExportPluginBaseSettings::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->openExportedFilesEnabled = valuesByKey.at(::OpenExportedFilesEnabledKey).toBool();
    emit baseSettingsChanged();

    restoreSettingsExtn(valuesByKey);
}

void LocationExportPluginBaseSettings::restoreDefaults() noexcept
{
    d->openExportedFilesEnabled = ::DefaultOpenExportedFilesEnabled;
    emit baseSettingsChanged();

    restoreDefaultsExtn();
}
