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
#include <Kernel/Enum.h>
#include <Kernel/System.h>
#include <Kernel/Settings.h>
#include "IgcImportSettings.h"

namespace
{
    // Keys
    constexpr const char *AltitudeKey {"Altitude"};
    constexpr const char *EnlThresholdKey {"EnlThreshold"};
    constexpr const char *ConvertAltitudeKey {"ConvertAltitude"};

    // Defaults
    constexpr IgcImportSettings::AltitudeMode DefaultAltitudeMode {IgcImportSettings::AltitudeMode::Gnss};
    constexpr int DefaultEnlThresholdPercent {40};
    constexpr bool DefaultConvertAltitude {true};
}

struct IgcImportSettingsPrivate
{
    IgcImportSettings::AltitudeMode altitudeMode {::DefaultAltitudeMode};
    int enlThresholdPercent {::DefaultEnlThresholdPercent};
    bool convertAltitude {::DefaultConvertAltitude};
};

// PUBLIC

IgcImportSettings::IgcImportSettings() noexcept
    : d {std::make_unique<IgcImportSettingsPrivate>()}
{}

IgcImportSettings::~IgcImportSettings() = default;

bool IgcImportSettings::isAircraftSelectionRequired() const noexcept
{
    return true;
}

bool IgcImportSettings::isTimeOffsetSyncSupported() const noexcept
{
    return true;
}

IgcImportSettings::AltitudeMode IgcImportSettings::getAltitudeMode() const noexcept
{
    return d->altitudeMode;
}

void IgcImportSettings::setAltitudeMode(AltitudeMode altitudeMode) noexcept
{
    if (d->altitudeMode != altitudeMode) {
        d->altitudeMode = altitudeMode;
        emit changed();
    }
}

int IgcImportSettings::getEnlThresholdPercent() const noexcept
{
    return d->enlThresholdPercent;
}

void IgcImportSettings::setEnlThresholdPercent(int enlThresholdPercent) noexcept
{
    if (d->enlThresholdPercent != enlThresholdPercent) {
        d->enlThresholdPercent = enlThresholdPercent;
        emit changed();
    }
}

bool IgcImportSettings::isConvertAltitudeEnabled() const noexcept
{
    return d->convertAltitude;
}

void IgcImportSettings::setConvertAltitudeEnabled(bool enable) noexcept
{
    if (d->convertAltitude != enable) {
        d->convertAltitude = enable;
        emit changed();
    }
}

// PROTECTED

void IgcImportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::AltitudeKey;
    keyValue.second = Enum::underly(d->altitudeMode);
    keyValues.push_back(keyValue);

    keyValue.first = ::EnlThresholdKey;
    keyValue.second = d->enlThresholdPercent;
    keyValues.push_back(keyValue);

    keyValue.first = ::ConvertAltitudeKey;
    keyValue.second = d->convertAltitude;
    keyValues.push_back(keyValue);
}

void IgcImportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::AltitudeKey;
    keyValue.second = Enum::underly(::DefaultAltitudeMode);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::EnlThresholdKey;
    keyValue.second = ::DefaultEnlThresholdPercent;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::ConvertAltitudeKey;
    keyValue.second = ::DefaultConvertAltitude;
    keysWithDefaults.push_back(keyValue);
}

void IgcImportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    auto enumValue = valuesByKey.at(::AltitudeKey).toInt(&ok);
    d->altitudeMode = ok && Enum::contains<AltitudeMode>(enumValue) ? static_cast<AltitudeMode>(enumValue) : ::DefaultAltitudeMode;

    const int enlThresholdPercent = valuesByKey.at(::EnlThresholdKey).toInt(&ok);
    d->enlThresholdPercent = ok ? enlThresholdPercent :  ::DefaultEnlThresholdPercent;

    d->convertAltitude = valuesByKey.at(::ConvertAltitudeKey).toBool();
}

void IgcImportSettings::restoreDefaultsExtn() noexcept
{
    d->altitudeMode = ::DefaultAltitudeMode;
    d->enlThresholdPercent = ::DefaultEnlThresholdPercent;
    d->convertAltitude = ::DefaultConvertAltitude;
}
