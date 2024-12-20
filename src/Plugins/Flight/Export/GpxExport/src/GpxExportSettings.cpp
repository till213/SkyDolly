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

#include <QString>

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <PluginManager/Flight/FlightExportPluginBaseSettings.h>
#include "GpxExportSettings.h"

namespace
{
    // Keys
    constexpr const char *TimestampModeKey {"TimestampMode"};
    constexpr const char *GeoidHeightExportEnabledKey {"GeoidHeightExportEnabled"};

    // Defaults
    constexpr GpxExportSettings::TimestampMode DefaultTimestampMode {GpxExportSettings::TimestampMode::Simulation};
    constexpr bool DefaultGeoidHeightExportEnable {false};
}

struct GpxExportSettingsPrivate
{
    GpxExportSettings::TimestampMode timestampMode {::DefaultTimestampMode};
    bool geoidHeightExportEnabled {::DefaultGeoidHeightExportEnable};
};

// PUBLIC

GpxExportSettings::GpxExportSettings() noexcept
    : FlightExportPluginBaseSettings(),
      d {std::make_unique<GpxExportSettingsPrivate>()}
{}

GpxExportSettings::~GpxExportSettings() = default;

GpxExportSettings::TimestampMode GpxExportSettings::getTimestampMode() const noexcept
{
    return d->timestampMode;
}

void GpxExportSettings::setTimestampMode(TimestampMode timestampMode) noexcept
{
    if (d->timestampMode != timestampMode) {
        d->timestampMode = timestampMode;
        emit changed();
    }
}

bool GpxExportSettings::isGeoidHeightExportEnabled() const noexcept
{
    return d->geoidHeightExportEnabled;
}

void GpxExportSettings::setGeoidHeightExportEnabled(bool enable) noexcept
{
    if (d->geoidHeightExportEnabled != enable) {
        d->geoidHeightExportEnabled = enable;
        emit changed();
    }
}

bool GpxExportSettings::isResamplingSupported() const noexcept
{
    return true;
}

bool GpxExportSettings::isFormationExportSupported(FormationExport formationExport) const noexcept
{
    bool supported {false};
    switch (formationExport) {
    case FormationExport::AllAircraftOneFile:
        supported = true;
        break;
    case FormationExport::AllAircraftSeparateFiles:
        supported = true;
        break;
    case FormationExport::UserAircraftOnly:
        supported = true;
        break;
    }
    return supported;
};

// PROTECTED

void GpxExportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::TimestampModeKey;
    keyValue.second = Enum::underly(d->timestampMode);
    keyValues.push_back(keyValue);

    keyValue.first = ::GeoidHeightExportEnabledKey;
    keyValue.second = d->geoidHeightExportEnabled;
    keyValues.push_back(keyValue);
}

void GpxExportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::TimestampModeKey;
    keyValue.second = Enum::underly(::DefaultTimestampMode);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::GeoidHeightExportEnabledKey;
    keyValue.second = ::DefaultGeoidHeightExportEnable;
    keysWithDefaults.push_back(keyValue);
}

void GpxExportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    auto enumValue = valuesByKey.at(::TimestampModeKey).toInt(&ok);
    d->timestampMode = ok && Enum::contains<GpxExportSettings::TimestampMode>(enumValue) ? static_cast<GpxExportSettings::TimestampMode>(enumValue) : ::DefaultTimestampMode;

    d->geoidHeightExportEnabled = valuesByKey.at(::GeoidHeightExportEnabledKey).toBool();
}

void GpxExportSettings::restoreDefaultsExtn() noexcept
{
    d->timestampMode = ::DefaultTimestampMode;
    d->geoidHeightExportEnabled = ::DefaultGeoidHeightExportEnable;
}
