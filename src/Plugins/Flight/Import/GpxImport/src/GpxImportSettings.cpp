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
#include <Kernel/Enum.h>
#include <Kernel/System.h>
#include <Kernel/Settings.h>
#include "GpxImportSettings.h"

namespace
{
    // Keys
    constexpr const char *WaypointSelectionKey {"WaypointSelection"};
    constexpr const char *PositionSelectionKey {"PositionSelection"};
    constexpr const char *DefaultAltitudeKey {"DefaultAltitude"};
    constexpr const char *DefaultSpeedKey {"DefaultSpeed"};
    constexpr const char *ConvertAltitudeKey {"ConvertAltitude"};

    // Defaults
    constexpr GpxImportSettings::GPXElement DefaultWaypointSelection {GpxImportSettings::GPXElement::Route};
    constexpr GpxImportSettings::GPXElement DefaultPositionSelection {GpxImportSettings::GPXElement::Track};
    // In feet
    constexpr int DefaultAltitude {1000};
    // In knots
    constexpr int DefaultSpeed {120};
    constexpr bool DefaultConvertAltitude {true};
}

struct GpxImportSettingsPrivate
{
    GpxImportSettings::GPXElement waypointSelection {::DefaultWaypointSelection};
    GpxImportSettings::GPXElement positionSelection {::DefaultPositionSelection};
    int defaultAltitude {::DefaultAltitude};
    int defaultSpeed {::DefaultSpeed};
    bool convertAltitude {::DefaultConvertAltitude};
};

// PUBLIC

GpxImportSettings::GpxImportSettings() noexcept
    : d(std::make_unique<GpxImportSettingsPrivate>())
{}

GpxImportSettings::~GpxImportSettings() = default;

bool GpxImportSettings::hasLogbookSupport() const noexcept
{
    return false;
}

GpxImportSettings::GPXElement GpxImportSettings::getWaypointSelection() const noexcept
{
    return d->waypointSelection;
}

void GpxImportSettings::setWaypointSelection(GPXElement selection) noexcept
{
    if (d->waypointSelection != selection) {
        d->waypointSelection = selection;
        emit extendedSettingsChanged();
    }
}

GpxImportSettings::GPXElement GpxImportSettings::getPositionSelection() const noexcept
{
    return d->positionSelection;
}

void GpxImportSettings::setPositionSelection(GPXElement selection) noexcept
{
    if (d->positionSelection != selection) {
        d->positionSelection = selection;
        emit extendedSettingsChanged();
    }
}

int GpxImportSettings::getDefaultAltitude() const noexcept
{
    return d->defaultAltitude;
}

void GpxImportSettings::setDefaultAltitude(int altitude) noexcept
{
    if (d->defaultAltitude != altitude) {
        d->defaultAltitude = altitude;
        emit extendedSettingsChanged();
    }
}

int GpxImportSettings::getDefaultSpeed() const noexcept
{
    return d->defaultSpeed;
}

void GpxImportSettings::setDefaultSpeed(int speed) noexcept
{
    if (d->defaultSpeed != speed) {
        d->defaultSpeed = speed;
        emit extendedSettingsChanged();
    }
}

bool GpxImportSettings::isConvertAltitudeEnabled() const noexcept
{
    return d->convertAltitude;
}

void GpxImportSettings::setConvertAltitudeEnabled(bool enable) noexcept
{
    if (d->convertAltitude != enable) {
        d->convertAltitude = enable;
        emit extendedSettingsChanged();
    }
}

// PROTECTED

void GpxImportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::WaypointSelectionKey;
    keyValue.second = Enum::underly(d->waypointSelection);
    keyValues.push_back(keyValue);

    keyValue.first = ::PositionSelectionKey;
    keyValue.second = Enum::underly(d->positionSelection);
    keyValues.push_back(keyValue);

    keyValue.first = ::DefaultAltitudeKey;
    keyValue.second = d->defaultAltitude;
    keyValues.push_back(keyValue);

    keyValue.first = ::DefaultSpeedKey;
    keyValue.second = d->defaultSpeed;
    keyValues.push_back(keyValue);

    keyValue.first = ::ConvertAltitudeKey;
    keyValue.second = d->convertAltitude;
    keyValues.push_back(keyValue);
}

void GpxImportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::WaypointSelectionKey;
    keyValue.second = Enum::underly(::DefaultWaypointSelection);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::PositionSelectionKey;
    keyValue.second = Enum::underly(::DefaultPositionSelection);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::DefaultAltitudeKey;
    keyValue.second = ::DefaultAltitude;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::DefaultSpeedKey;
    keyValue.second = ::DefaultSpeed;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::ConvertAltitudeKey;
    keyValue.second = ::DefaultConvertAltitude;
    keysWithDefaults.push_back(keyValue);
}

void GpxImportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    int enumeration = valuesByKey.at(::WaypointSelectionKey).toInt(&ok);
    if (ok) {
        d->waypointSelection = static_cast<GPXElement>(enumeration);
    } else {
        d->waypointSelection = DefaultWaypointSelection;
    }

    enumeration = valuesByKey.at(::PositionSelectionKey).toInt(&ok);
    if (ok) {
        d->positionSelection = static_cast<GPXElement>(enumeration);
    } else {
        d->positionSelection = DefaultPositionSelection;
    }

    const int altitude = valuesByKey.at(::DefaultAltitudeKey).toInt(&ok);
    if (ok) {
        d->defaultAltitude = altitude;
    } else {
        d->defaultAltitude = DefaultAltitude;
    }

    const int speed = valuesByKey.at(::DefaultSpeedKey).toInt(&ok);
    if (ok) {
        d->defaultSpeed = speed;
    } else {
        d->defaultSpeed = DefaultSpeed;
    }

    d->convertAltitude = valuesByKey.at(::ConvertAltitudeKey).toBool();

    emit extendedSettingsChanged();
}

void GpxImportSettings::restoreDefaultsExtn() noexcept
{
    d->waypointSelection = ::DefaultWaypointSelection;
    d->positionSelection = ::DefaultPositionSelection;
    d->defaultAltitude = ::DefaultAltitude;
    d->defaultSpeed = ::DefaultSpeed;
    d->convertAltitude = ::DefaultConvertAltitude;

    emit extendedSettingsChanged();
}
