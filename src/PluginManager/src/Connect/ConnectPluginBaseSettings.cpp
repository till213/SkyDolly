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

#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Settings.h>
#include <Connect/Connect.h>
#include <Connect/ConnectPluginBaseSettings.h>
#include <Connect/FlightSimulatorShortcuts.h>

namespace
{
    // Keys
    constexpr const char *RecordShortcutKey {"RecordShortcutKey"};
    constexpr const char *ReplayShortcutKey {"ReplayShortcutKey"};
    constexpr const char *PauseShortcutKey {"PauseShortcutKey"};
    constexpr const char *StopShortcutKey {"StopShortcutKey"};
    constexpr const char *BackwardShortcutKey {"BackwardShortcutKey"};
    constexpr const char *ForwardShortcutKey {"ForwardShortcutKey"};
    constexpr const char *BeginShortcutKey {"BeginShortcutKey"};
    constexpr const char *EndShortcutKey {"EndShortcutKey"};
}

struct ConnectPluginBaseSettingsPrivate
{
    FlightSimulatorShortcuts flightSimulatorShortcuts {DefaultFlightSimulatorShortcuts};

    // Defaults
    static inline const QKeySequence DefaultRecordShortcut {"Ctrl+R"};
    static inline const QKeySequence DefaultReplayShortcut {"Ctrl+P"};
    static inline const QKeySequence DefaultPauseShortcut {"Alt+P"};
    static inline const QKeySequence DefaultStopShortcut {"Ctrl+S"};
    static inline const QKeySequence DefaultBackwardShortcut {"Ctrl+,"};
    static inline const QKeySequence DefaultForwardShortcut {"Ctrl+."};
    static inline const QKeySequence DefaultBeginShortcut {"Ctrl+PgUp"};
    static inline const QKeySequence DefaultEndShortcut {"Ctrl+PgDown"};
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
    : d {std::make_unique<ConnectPluginBaseSettingsPrivate>()}
{}

ConnectPluginBaseSettings::~ConnectPluginBaseSettings()
{
#ifdef DEBUG
    qDebug() << "ConnectPluginBaseSettings::~ConnectPluginBaseSettings: DELETED";
#endif
}

FlightSimulatorShortcuts ConnectPluginBaseSettings::getFlightSimulatorShortcuts() const noexcept
{
    return d->flightSimulatorShortcuts;
}

void ConnectPluginBaseSettings::setFlightSimulatorShortcuts(FlightSimulatorShortcuts shortcuts) noexcept
{
    if (d->flightSimulatorShortcuts != shortcuts) {
        d->flightSimulatorShortcuts = std::move(shortcuts);
        emit changed(Connect::Mode::SetupOnly);
    }
}

void ConnectPluginBaseSettings::addSettings(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::RecordShortcutKey;
    keyValue.second = d->flightSimulatorShortcuts.record.toString();
    keyValues.push_back(keyValue);

    keyValue.first = ::ReplayShortcutKey;
    keyValue.second = d->flightSimulatorShortcuts.replay.toString();
    keyValues.push_back(keyValue);

    keyValue.first = ::PauseShortcutKey;
    keyValue.second = d->flightSimulatorShortcuts.pause.toString();
    keyValues.push_back(keyValue);

    keyValue.first = ::StopShortcutKey;
    keyValue.second = d->flightSimulatorShortcuts.stop.toString();
    keyValues.push_back(keyValue);

    keyValue.first = ::BackwardShortcutKey;
    keyValue.second = d->flightSimulatorShortcuts.backward.toString();
    keyValues.push_back(keyValue);

    keyValue.first = ::ForwardShortcutKey;
    keyValue.second = d->flightSimulatorShortcuts.forward.toString();
    keyValues.push_back(keyValue);

    keyValue.first = ::BeginShortcutKey;
    keyValue.second = d->flightSimulatorShortcuts.begin.toString();
    keyValues.push_back(keyValue);

    keyValue.first = ::EndShortcutKey;
    keyValue.second = d->flightSimulatorShortcuts.end.toString();
    keyValues.push_back(keyValue);

    addSettingsExtn(keyValues);
}

void ConnectPluginBaseSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::RecordShortcutKey;
    keyValue.second = d->DefaultRecordShortcut.toString();
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::ReplayShortcutKey;
    keyValue.second = d->DefaultReplayShortcut.toString();
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::PauseShortcutKey;
    keyValue.second = d->DefaultPauseShortcut.toString();
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::StopShortcutKey;
    keyValue.second = d->DefaultStopShortcut.toString();
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::BackwardShortcutKey;
    keyValue.second = d->DefaultBackwardShortcut.toString();
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::ForwardShortcutKey;
    keyValue.second = d->DefaultForwardShortcut.toString();
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::BeginShortcutKey;
    keyValue.second = d->DefaultBeginShortcut.toString();
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::EndShortcutKey;
    keyValue.second = d->DefaultEndShortcut.toString();
    keysWithDefaults.push_back(keyValue);

    addKeysWithDefaultsExtn(keysWithDefaults);
}

void ConnectPluginBaseSettings::restoreSettings(const Settings::ValuesByKey &valuesByKey) noexcept
{
    d->flightSimulatorShortcuts.record = valuesByKey.at(::RecordShortcutKey).toString();
    d->flightSimulatorShortcuts.replay = valuesByKey.at(::ReplayShortcutKey).toString();
    d->flightSimulatorShortcuts.pause = valuesByKey.at(::PauseShortcutKey).toString();
    d->flightSimulatorShortcuts.stop = valuesByKey.at(::StopShortcutKey).toString();
    d->flightSimulatorShortcuts.backward = valuesByKey.at(::BackwardShortcutKey).toString();
    d->flightSimulatorShortcuts.forward = valuesByKey.at(::ForwardShortcutKey).toString();
    d->flightSimulatorShortcuts.begin = valuesByKey.at(::BeginShortcutKey).toString();
    d->flightSimulatorShortcuts.end = valuesByKey.at(::EndShortcutKey).toString();

    restoreSettingsExtn(valuesByKey);

    emit changed(Connect::Mode::SetupOnly);
}

void ConnectPluginBaseSettings::restoreDefaults() noexcept
{
    d->flightSimulatorShortcuts.record = d->DefaultRecordShortcut;
    d->flightSimulatorShortcuts.replay = d->DefaultReplayShortcut;
    d->flightSimulatorShortcuts.pause = d->DefaultPauseShortcut;
    d->flightSimulatorShortcuts.stop = d->DefaultStopShortcut;
    d->flightSimulatorShortcuts.backward = d->DefaultBackwardShortcut;
    d->flightSimulatorShortcuts.forward = d->DefaultForwardShortcut;
    d->flightSimulatorShortcuts.begin = d->DefaultBeginShortcut;
    d->flightSimulatorShortcuts.end = d->DefaultEndShortcut;

    restoreDefaultsExtn();

    emit changed(Connect::Mode::SetupOnly);
}
