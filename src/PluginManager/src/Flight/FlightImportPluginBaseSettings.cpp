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

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Flight/FlightImportPluginBaseSettings.h>

namespace
{
    // Keys
    constexpr const char *ImportDirectoryEnabledKey {"ImportDirectoryEnabled"};
    constexpr const char *AircraftImportModeKey {"AircraftImportMode"};
    constexpr const char *TimeOffsetSyncKey {"TimeOffsetSync"};

    // Defaults
    constexpr FlightImportPluginBaseSettings::AircraftImportMode DefaultAircraftImportMode {FlightImportPluginBaseSettings::AircraftImportMode::AddToNewFlight};
    constexpr SkyMath::TimeOffsetSync DefaultTimeOffsetSync {SkyMath::TimeOffsetSync::None};
    constexpr bool DefaultImportDirectoryEnabled {false};    
}

struct FlightImportPluginBaseSettingsPrivate
{
    FlightImportPluginBaseSettings::AircraftImportMode aircraftImportMode {::DefaultAircraftImportMode};
    SkyMath::TimeOffsetSync timeOffsetSync {::DefaultTimeOffsetSync};
    bool importDirectoryEnabled {::DefaultImportDirectoryEnabled};
};

// PUBLIC

FlightImportPluginBaseSettings::FlightImportPluginBaseSettings() noexcept
    : d {std::make_unique<FlightImportPluginBaseSettingsPrivate>()}
{}

FlightImportPluginBaseSettings::~FlightImportPluginBaseSettings() = default;

bool FlightImportPluginBaseSettings::isImportDirectoryEnabled() const noexcept
{
    return d->importDirectoryEnabled;
}

void FlightImportPluginBaseSettings::setImportDirectoryEnabled(bool enabled) noexcept
{
    if (d->importDirectoryEnabled != enabled) {
        d->importDirectoryEnabled = enabled;
        emit changed();
    }
}

FlightImportPluginBaseSettings::AircraftImportMode FlightImportPluginBaseSettings::getAircraftImportMode() const noexcept
{
    return d->aircraftImportMode;
}

void FlightImportPluginBaseSettings::setAircraftImportMode(AircraftImportMode mode) noexcept
{
    if (d->aircraftImportMode != mode) {
        d->aircraftImportMode = mode;
        emit changed();
    }
}

SkyMath::TimeOffsetSync FlightImportPluginBaseSettings::getTimeOffsetSync() const noexcept
{
    return d->timeOffsetSync;
}

void FlightImportPluginBaseSettings::setTimeOffsetSync(SkyMath::TimeOffsetSync sync) noexcept
{
    if (d->timeOffsetSync != sync) {
        d->timeOffsetSync = sync;
        emit changed();
    }
}

void FlightImportPluginBaseSettings::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ImportDirectoryEnabledKey;
    keyValue.second = d->importDirectoryEnabled;
    keyValues.push_back(keyValue);

    keyValue.first = ::AircraftImportModeKey;
    keyValue.second = Enum::underly(d->aircraftImportMode);
    keyValues.push_back(keyValue);

    keyValue.first = ::TimeOffsetSyncKey;
    keyValue.second = Enum::underly(d->timeOffsetSync);
    keyValues.push_back(keyValue);

    addSettingsExtn(keyValues);
}

void FlightImportPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ImportDirectoryEnabledKey;
    keyValue.second = ::DefaultImportDirectoryEnabled;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::AircraftImportModeKey;
    keyValue.second = Enum::underly(::DefaultAircraftImportMode);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::TimeOffsetSyncKey;
    keyValue.second = Enum::underly(::DefaultTimeOffsetSync);
    keysWithDefaults.push_back(keyValue);

    addKeysWithDefaultsExtn(keysWithDefaults);
}

void FlightImportPluginBaseSettings::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->importDirectoryEnabled = valuesByKey.at(::ImportDirectoryEnabledKey).toBool();

    bool ok {true};
    auto enumValue = valuesByKey.at(::AircraftImportModeKey).toInt(&ok);
    d->aircraftImportMode = ok && Enum::contains<FlightImportPluginBaseSettings::AircraftImportMode>(enumValue) ? static_cast<FlightImportPluginBaseSettings::AircraftImportMode>(enumValue) : ::DefaultAircraftImportMode;

    enumValue = valuesByKey.at(::TimeOffsetSyncKey).toInt(&ok);
    d->timeOffsetSync = ok && Enum::contains<SkyMath::TimeOffsetSync>(enumValue) ? static_cast<SkyMath::TimeOffsetSync>(enumValue) : ::DefaultTimeOffsetSync;

    restoreSettingsExtn(valuesByKey);

    emit changed();
}

void FlightImportPluginBaseSettings::restoreDefaults() noexcept
{
    d->importDirectoryEnabled = ::DefaultImportDirectoryEnabled;
    d->aircraftImportMode = ::DefaultAircraftImportMode;
    d->timeOffsetSync = ::DefaultTimeOffsetSync;

    restoreDefaultsExtn();

    emit changed();
}
