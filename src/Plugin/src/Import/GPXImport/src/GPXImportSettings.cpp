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
#include "../../../../../Kernel/src/Settings.h"
#include "GPXImportSettings.h"

namespace
{
    constexpr char WaypointSelectionKey[] = "WaypointSelection";
    constexpr char PositionSelectionKey[] = "PositionSelection";
    constexpr char AltitudeKey[] = "DefaultAltitude";
    constexpr char DefaultVelocityKey[] = "DefaultVelocity";
}

// PUBLIC

GPXImportSettings::GPXImportSettings() noexcept
{
    initSettings();
}

Settings::PluginSettings GPXImportSettings::getSettings() const noexcept
{
    Settings::PluginSettings settings;
    Settings::KeyValue keyValue;

    keyValue.first = ::WaypointSelectionKey;
    keyValue.second = Enum::toUnderlyingType(m_waypointSelection);
    settings.push_back(keyValue);

    keyValue.first = ::PositionSelectionKey;
    keyValue.second = Enum::toUnderlyingType(m_positionSelection);
    settings.push_back(keyValue);

    keyValue.first = ::AltitudeKey;
    keyValue.second = m_defaultAltitude;
    settings.push_back(keyValue);

    keyValue.first = ::DefaultVelocityKey;
    keyValue.second = m_defaultVelocity;
    settings.push_back(keyValue);

    return settings;
}

Settings::KeysWithDefaults GPXImportSettings::getKeysWithDefault() const noexcept
{
    Settings::KeysWithDefaults keys;
    Settings::KeyValue keyValue;

    keyValue.first = ::WaypointSelectionKey;
    keyValue.second = Enum::toUnderlyingType(DefaultWaypointSelection);
    keys.push_back(keyValue);

    keyValue.first = ::PositionSelectionKey;
    keyValue.second = Enum::toUnderlyingType(DefaultPositionSelection);
    keys.push_back(keyValue);

    keyValue.first = ::AltitudeKey;
    keyValue.second = DefaultAltitude;
    keys.push_back(keyValue);

    keyValue.first = ::DefaultVelocityKey;
    keyValue.second = DefaultVelocity;
    keys.push_back(keyValue);

    return keys;
}

void GPXImportSettings::setSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    bool ok;
    int enumeration = valuesByKey[::WaypointSelectionKey].toInt(&ok);
    if (ok) {
        m_waypointSelection = static_cast<GPXElement >(enumeration);
    } else {
        m_waypointSelection = DefaultWaypointSelection;
    }

    enumeration = valuesByKey[::PositionSelectionKey].toInt(&ok);
    if (ok) {
        m_positionSelection = static_cast<GPXElement >(enumeration);
    } else {
        m_positionSelection = DefaultPositionSelection;
    }

    const int altitude = valuesByKey[::AltitudeKey].toInt(&ok);
    if (ok) {
        m_defaultAltitude = altitude;
    } else {
        m_defaultAltitude = DefaultAltitude;
    }

    const int velocity = valuesByKey[::DefaultVelocityKey].toInt(&ok);
    if (ok) {
        m_defaultVelocity = velocity;
    } else {
        m_defaultVelocity = DefaultVelocity;
    }
}

void GPXImportSettings::restoreDefaults() noexcept
{
    initSettings();
    emit defaultsRestored();
}

// PRIVATE

void GPXImportSettings::initSettings() noexcept
{
    m_waypointSelection = DefaultWaypointSelection;
    m_positionSelection = DefaultPositionSelection;
    m_defaultAltitude = DefaultAltitude;
    m_defaultVelocity = DefaultVelocity;
}
