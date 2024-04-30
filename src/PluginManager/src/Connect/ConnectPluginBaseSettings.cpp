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
#include <Kernel/FlightSimulatorShortcuts.h>
#include <Connect/ConnectPluginBaseSettings.h>

namespace
{
    // Keys
    constexpr const char *RecordShortcutKey {"RecordShortcutKey"};
    constexpr const char *ReplayShortcutKey {"ReplayShortcutKey"};

    // Defaults
    constexpr bool DefaultOpenExportedFilesEnabled {false};
}

struct ConnectPluginBaseSettingsPrivate
{
    FlightSimulatorShortcuts flightSimulatorShortcuts {DefaultFlightSimulatorShortcuts};

    static inline const QKeySequence DefaultRecordShortcut{QStringLiteral("Ctrl+R")};
    static inline const QKeySequence DefaultReplayShortcut{QStringLiteral("Ctrl+P")};
    static inline const QKeySequence DefaultPauseShortcut{QStringLiteral("Alt+P")};
    static inline const QKeySequence DefaultStopShortcut{QStringLiteral("Ctrl+S")};
    static inline const QKeySequence DefaultBackwardShortcut{QStringLiteral("Ctrl+,")};
    static inline const QKeySequence DefaultForwardShortcut{QStringLiteral("Ctrl+.")};
    // TODO Find good default keys that are not already taken my the MSFS default keyboard layout
    static inline const QKeySequence DefaultBeginShortcut{QStringLiteral("Ctrl+PgUp")};
    static inline const QKeySequence DefaultEndShortcut{QStringLiteral("Ctrl+PgDown")};
    static inline const FlightSimulatorShortcuts DefaultFlightSimulatorShortcuts {
        DefaultRecordShortcut,
        DefaultReplayShortcut,
        DefaultPauseShortcut,
        DefaultStopShortcut,
        DefaultBackwardShortcut,
        DefaultForwardShortcut,
        DefaultBeginShortcut,
        DefaultEndShortcut
    };
};

// PUBLIC

ConnectPluginBaseSettings::ConnectPluginBaseSettings() noexcept
    : d(std::make_unique<ConnectPluginBaseSettingsPrivate>())
{}

ConnectPluginBaseSettings::~ConnectPluginBaseSettings() = default;

FlightSimulatorShortcuts ConnectPluginBaseSettings::getFlightSimulatorShortcuts() const noexcept
{
    return d->flightSimulatorShortcuts;
}

void ConnectPluginBaseSettings::setFlightSimulatorShortcuts(FlightSimulatorShortcuts shortcuts) noexcept
{
    if (d->flightSimulatorShortcuts != shortcuts) {
        d->flightSimulatorShortcuts = std::move(shortcuts);
        emit Settings::getInstance().flightSimulatorShortcutsChanged(d->flightSimulatorShortcuts);
    }
}

void ConnectPluginBaseSettings::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = QString::fromLatin1(::RecordShortcutKey);
    keyValue.second = d->flightSimulatorShortcuts.record.toString();
    keyValues.push_back(keyValue);

    addSettingsExtn(keyValues);
}

void ConnectPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = QString::fromLatin1(::RecordShortcutKey);
    keyValue.second = d->DefaultRecordShortcut.toString();
    keysWithDefaults.push_back(keyValue);

    addKeysWithDefaultsExtn(keysWithDefaults);
}

void ConnectPluginBaseSettings::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->flightSimulatorShortcuts.record = valuesByKey.at(QString::fromLatin1(::RecordShortcutKey)).toString();
    restoreSettingsExtn(valuesByKey);

    emit changed();
}

void ConnectPluginBaseSettings::restoreDefaults() noexcept
{
    d->flightSimulatorShortcuts.record = d->DefaultRecordShortcut;
    restoreDefaultsExtn();

    emit changed();
}
