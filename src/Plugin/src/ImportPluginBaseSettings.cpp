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
#include <memory>

#include "../../Kernel/src/Settings.h"
#include "ImportPluginBaseSettings.h"

namespace
{
    // Keys
    constexpr char AddToFlightEnabledKey[] = "AddToFlightEnabled";

    // Defaults
    constexpr bool DefaultAddToFlightEnabled = false;
}

class ImportPluginBaseSettingsPrivate
{
public:
    ImportPluginBaseSettingsPrivate()
        : addToFlightEnabled(::DefaultAddToFlightEnabled)
    {}

    bool addToFlightEnabled;
};

// PUBLIC

ImportPluginBaseSettings::ImportPluginBaseSettings() noexcept
    : d(std::make_unique<ImportPluginBaseSettingsPrivate>())
{
#ifdef DEBUG
    qDebug("ImportPluginBaseSettings::ImportPluginBaseSettings: CREATED");
#endif
}

ImportPluginBaseSettings::~ImportPluginBaseSettings() noexcept
{
#ifdef DEBUG
    qDebug("ImportPluginBaseSettings::~ImportPluginBaseSettings: DELETED");
#endif
}

bool ImportPluginBaseSettings::isAddToFlightEnabled() const noexcept
{
    return d->addToFlightEnabled;
}

void ImportPluginBaseSettings::setAddToFlightEnabled(bool enabled) noexcept
{
    if (d->addToFlightEnabled != enabled) {
        d->addToFlightEnabled = enabled;
        emit baseSettingsChanged();
    }
}

void ImportPluginBaseSettings::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::AddToFlightEnabledKey;
    keyValue.second = d->addToFlightEnabled;
    keyValues.push_back(keyValue);

    addSettingsExtn(keyValues);
}

void ImportPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::AddToFlightEnabledKey;
    keyValue.second = ::DefaultAddToFlightEnabled;
    keysWithDefaults.push_back(keyValue);

    addKeysWithDefaultsExtn(keysWithDefaults);
}

void ImportPluginBaseSettings::restoreSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    d->addToFlightEnabled = valuesByKey[::AddToFlightEnabledKey].toBool();
    emit baseSettingsChanged();

    restoreSettingsExtn(valuesByKey);
}

void ImportPluginBaseSettings::restoreDefaults() noexcept
{
    d->addToFlightEnabled = ::DefaultAddToFlightEnabled;
    emit baseSettingsChanged();

    restoreDefaultsExtn();
}
