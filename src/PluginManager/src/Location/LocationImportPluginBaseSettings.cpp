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

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Persistence/Service/LocationService.h>
#include <Location/LocationImportPluginBaseSettings.h>

namespace
{
    // Keys
    constexpr const char *ImportDirectoryEnabledKey {"ImportDirectoryEnabled"};
    constexpr const char *ImportModeKey {"ImportMode"};
    constexpr const char *NearestLocationDistanceKey {"NearestLocationDistance"};

    // Defaults
    constexpr bool DefaultImportDirectoryEnabled {false};
    constexpr LocationService::Mode DefaultImportMode {LocationService::Mode::Skip};
    // 250 metres
    constexpr double DefaultNearestLocationDistanceKm {250.0 / 1000.0};
}

struct LocationImportPluginBaseSettingsPrivate
{
    bool importDirectoryEnabled {::DefaultImportDirectoryEnabled};
    LocationService::Mode importMode {::DefaultImportMode};
    double nearestLocationDistanceKm {::DefaultNearestLocationDistanceKm};
};

// PUBLIC

LocationImportPluginBaseSettings::LocationImportPluginBaseSettings() noexcept
    : d {std::make_unique<LocationImportPluginBaseSettingsPrivate>()}
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
        emit changed();
    }
}

LocationService::Mode LocationImportPluginBaseSettings::getImportMode() const noexcept
{
    return d->importMode;
}

void LocationImportPluginBaseSettings::setImportMode(LocationService::Mode mode) noexcept
{
    if (d->importMode != mode) {
        d->importMode = mode;
        emit changed();
    }
}

double LocationImportPluginBaseSettings::getNearestLocationDistanceKm() const noexcept
{
    return d->nearestLocationDistanceKm;
}

void LocationImportPluginBaseSettings::setNearestLocationDistanceKm(double distanceKm) noexcept
{
    if (d->nearestLocationDistanceKm != distanceKm) {
        d->nearestLocationDistanceKm = distanceKm;
        emit changed();
    }
}

void LocationImportPluginBaseSettings::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ImportDirectoryEnabledKey;
    keyValue.second = d->importDirectoryEnabled;
    keyValues.push_back(keyValue);

    keyValue.first = ::ImportModeKey;
    keyValue.second = Enum::underly(d->importMode);
    keyValues.push_back(keyValue);

    keyValue.first = ::NearestLocationDistanceKey;
    keyValue.second = d->nearestLocationDistanceKm;
    keyValues.push_back(keyValue);

    addSettingsExtn(keyValues);
}

void LocationImportPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ImportDirectoryEnabledKey;
    keyValue.second = ::DefaultImportDirectoryEnabled;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::ImportModeKey;
    keyValue.second = Enum::underly(::DefaultImportMode);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::NearestLocationDistanceKey;
    keyValue.second = ::DefaultNearestLocationDistanceKm;
    keysWithDefaults.push_back(keyValue);

    addKeysWithDefaultsExtn(keysWithDefaults);
}

void LocationImportPluginBaseSettings::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->importDirectoryEnabled = valuesByKey.at(::ImportDirectoryEnabledKey).toBool();

    bool ok {true};
    auto enumValue = valuesByKey.at(::ImportModeKey).toInt(&ok);
    d->importMode = ok && Enum::contains<LocationService::Mode>(enumValue) ? static_cast<LocationService::Mode>(enumValue) : ::DefaultImportMode;

    d->nearestLocationDistanceKm = valuesByKey.at(::NearestLocationDistanceKey).toDouble(&ok);
    if (!ok) {
        d->nearestLocationDistanceKm = ::DefaultNearestLocationDistanceKm;
    }

    restoreSettingsExtn(valuesByKey);

    emit changed();
}

void LocationImportPluginBaseSettings::restoreDefaults() noexcept
{
    d->importDirectoryEnabled = ::DefaultImportDirectoryEnabled;
    d->importMode = ::DefaultImportMode;
    d->nearestLocationDistanceKm = ::DefaultNearestLocationDistanceKm;

    restoreDefaultsExtn();

    emit changed();
}
