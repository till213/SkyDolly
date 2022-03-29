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

#include <QString>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../Plugin/src/ExportPluginBaseSettings.h"
#include "GPXExportSettings.h"

namespace
{
    // Keys
    constexpr char TimestampModeKey[] = "TimestampMode";

    // Defaults
    constexpr GPXExportSettings::TimestampMode DefaultTimestampMode = GPXExportSettings::TimestampMode::Simulation;
}

class GPXExportSettingsPrivate
{
public:
    GPXExportSettingsPrivate()
    : timestampMode(::DefaultTimestampMode)
    {}

    GPXExportSettings::TimestampMode timestampMode;
};

// PUBLIC

GPXExportSettings::GPXExportSettings() noexcept
    : ExportPluginBaseSettings(),
      d(std::make_unique<GPXExportSettingsPrivate>())
{
#ifdef DEBUG
    qDebug("GPXExportSettings::GPXExportSettings: CREATED");
#endif
}

GPXExportSettings::~GPXExportSettings() noexcept
{
#ifdef DEBUG
    qDebug("GPXExportSettings::~GPXExportSettings: DELETED");
#endif
}

GPXExportSettings::TimestampMode GPXExportSettings::getTimestampMode() const noexcept
{
    return d->timestampMode;
}

void GPXExportSettings::setTimestampMode(TimestampMode timestampMode) noexcept
{
    if (d->timestampMode != timestampMode) {
        d->timestampMode = timestampMode;
        emit extendedSettingsChanged();
    }
}

// PROTECTED

void GPXExportSettings::addSettingsExtn(Settings::PluginSettings &settings) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::TimestampModeKey;
    keyValue.second = Enum::toUnderlyingType(d->timestampMode);
    settings.push_back(keyValue);
}

void GPXExportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::TimestampModeKey;
    keyValue.second = Enum::toUnderlyingType(::DefaultTimestampMode);
    keysWithDefaults.push_back(keyValue);
}

void GPXExportSettings::restoreSettingsExtn(Settings::ValuesByKey valuesByKey) noexcept
{
    bool ok;
    const int enumeration = valuesByKey[::TimestampModeKey].toInt(&ok);
    if (ok) {
        d->timestampMode = static_cast<GPXExportSettings::TimestampMode >(enumeration);
    } else {
        d->timestampMode = ::DefaultTimestampMode;
    }

    emit extendedSettingsChanged();
}

void GPXExportSettings::restoreDefaultsExtn() noexcept
{
    d->timestampMode = ::DefaultTimestampMode;

    emit extendedSettingsChanged();
}
