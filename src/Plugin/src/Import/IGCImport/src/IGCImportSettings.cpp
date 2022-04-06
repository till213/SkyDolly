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

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/System.h"
#include "../../../../../Kernel/src/Settings.h"
#include "IGCImportSettings.h"

namespace
{
    // Keys
    constexpr char AltitudeKey[] = "Altitude";
    constexpr char EnlThresholdKey[] = "EnlThreshold";

    // Defaults
    constexpr IGCImportSettings::AltitudeMode DefaultAltitudeMode = IGCImportSettings::AltitudeMode::Gnss;
    constexpr int DefaultEnlThresholdPercent = 40;
}

class IGCImportSettingsPrivate
{
public:
    IGCImportSettingsPrivate()
        : altitudeMode(::DefaultAltitudeMode),
          enlThresholdPercent(::DefaultEnlThresholdPercent)
    {}

    IGCImportSettings::AltitudeMode altitudeMode;
    int enlThresholdPercent;
};

// PUBLIC

IGCImportSettings::IGCImportSettings() noexcept
    : d(std::make_unique<IGCImportSettingsPrivate>())
{
#ifdef DEBUG
    qDebug("IGCImportSettings::IGCImportSettings: CREATED");
#endif
}

IGCImportSettings::~IGCImportSettings() noexcept
{
#ifdef DEBUG
    qDebug("IGCImportSettings::~IGCImportSettings: DELETED");
#endif
}

IGCImportSettings::AltitudeMode IGCImportSettings::getAltitudeMode() const noexcept
{
    return d->altitudeMode;
}

void IGCImportSettings::setAltitudeMode(AltitudeMode altitudeMode) noexcept
{
    if (d->altitudeMode != altitudeMode) {
        d->altitudeMode = altitudeMode;
        emit extendedSettingsChanged();
    }
}

int IGCImportSettings::getEnlThresholdPercent() const noexcept
{
    return d->enlThresholdPercent;
}

void IGCImportSettings::setEnlThresholdPercent(int enlThresholdPercent) noexcept
{
    if (d->enlThresholdPercent != enlThresholdPercent) {
        d->enlThresholdPercent = enlThresholdPercent;
        emit extendedSettingsChanged();
    }
}

// PROTECTED

void IGCImportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::AltitudeKey;
    keyValue.second = Enum::toUnderlyingType(d->altitudeMode);
    keyValues.push_back(keyValue);

    keyValue.first = ::EnlThresholdKey;
    keyValue.second = d->enlThresholdPercent;
    keyValues.push_back(keyValue);
}

void IGCImportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::AltitudeKey;
    keyValue.second = Enum::toUnderlyingType(::DefaultAltitudeMode);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::EnlThresholdKey;
    keyValue.second = ::DefaultEnlThresholdPercent;
    keysWithDefaults.push_back(keyValue);
}

void IGCImportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok;
    const int enumeration = valuesByKey.at(::AltitudeKey).toInt(&ok);
    if (ok) {
        d->altitudeMode = static_cast<AltitudeMode >(enumeration);
    } else {
        d->altitudeMode = ::DefaultAltitudeMode;
    }

    const double enlThresholdPercent = valuesByKey.at(::EnlThresholdKey).toDouble(&ok);
    if (ok) {
        d->enlThresholdPercent = enlThresholdPercent;
    } else {
        d->enlThresholdPercent = ::DefaultEnlThresholdPercent;
    }
}

void IGCImportSettings::restoreDefaultsExtn() noexcept
{
    d->altitudeMode = ::DefaultAltitudeMode;
    d->enlThresholdPercent = ::DefaultEnlThresholdPercent;

    emit extendedSettingsChanged();
}
