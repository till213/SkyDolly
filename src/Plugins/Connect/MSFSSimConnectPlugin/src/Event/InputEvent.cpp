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
#include <windows.h>
#include <unordered_map>

#include <SimConnect.h>

#include <QByteArray>
#include <QKeySequence>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <Kernel/FlightSimulatorShortcuts.h>
#include "SimConnectEvent.h"
#include "InputEvent.h"

namespace
{
    enum struct Input: ::SIMCONNECT_INPUT_GROUP_ID {
        SkyDollyControl
    };

    enum struct NotificationGroup: ::SIMCONNECT_NOTIFICATION_GROUP_ID {
        ShortcutsGroup
    };
}

// PRIVATE

struct InputEventPrivate
{
    bool isInitialised {false};

    // https://docs.flightsimulator.com/html/Programming_Tools/SimConnect/API_Reference/Events_And_Data/SimConnect_MapInputEventToClientEvent.htm
    static const inline std::unordered_map<QString, QByteArray> qtToMSFS {
        { "Alt", "VK_LMENU" },
        { "Ctrl", "VK_LCONTROL" },
        { "Shift", "VK_LSHIFT" },
        { "Up", "VK_UP" },
        { "Down", "VK_DOWN" },
        { "Left", "VK_LEFT" },
        { "Right", "VK_RIGHT" },
        { "PgUp", "VK_PRIOR" },
        { "PgDown", "VK_NEXT" }
    };
};

// PUBLIC

InputEvent::InputEvent() noexcept
    : d(std::make_unique<InputEventPrivate>())
{}

InputEvent::~InputEvent() = default;

bool InputEvent::setup(HANDLE simConnectHandle, const FlightSimulatorShortcuts &shortcuts) noexcept
{
    HRESULT result {S_OK};
    if (d->isInitialised) {
        result = clear(simConnectHandle);
    }

    // Recording
    QByteArray shortcut = toMSFSShortcut(shortcuts.record);
    result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomRecording), "Custom.Recording");
    result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomRecording));
    result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomRecording));

    // Replay
    shortcut = toMSFSShortcut(shortcuts.replay);
    result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomReplay), "Custom.Replay");
    result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomReplay));
    result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomReplay));

    // Pause
    shortcut = toMSFSShortcut(shortcuts.pause);
    result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomPause), "Custom.Pause");
    result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomPause));
    result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomPause));

    // Stop
    shortcut = toMSFSShortcut(shortcuts.stop);
    result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomStop), "Custom.Stop");
    result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomStop));
    result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomStop));

    // Backward
    shortcut = toMSFSShortcut(shortcuts.backward);
    result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomBackward), "Custom.Backward");
    result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomBackward));
    result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBackward));

    // Forward
    shortcut = toMSFSShortcut(shortcuts.forward);
    result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomForward), "Custom.Forward");
    result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomForward));
    result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomForward));

    // Begin
    shortcut = toMSFSShortcut(shortcuts.begin);
    result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomBegin), "Custom.Begin");
    result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomBegin));
    result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBegin));

    // End
    shortcut = toMSFSShortcut(shortcuts.end);
    result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomEnd), "Custom.End");
    result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomEnd));
    result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomEnd));

    result |= ::SimConnect_SetNotificationGroupPriority(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), SIMCONNECT_GROUP_PRIORITY_HIGHEST);
    result |= ::SimConnect_SetInputGroupState(simConnectHandle, Enum::underly(Input::SkyDollyControl), SIMCONNECT_STATE_ON);

    d->isInitialised = result == S_OK;

    return d->isInitialised;
}

// PRIVATE

bool InputEvent::clear(HANDLE simConnectHandle) noexcept
{
    HRESULT result = ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomRecording));
    result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomReplay));
    result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomPause));
    result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomStop));
    result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBackward));
    result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomForward));
    result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBegin));
    result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomEnd));
    result |= ::SimConnect_ClearInputGroup(simConnectHandle, Enum::underly(Input::SkyDollyControl));

    return result == S_OK;
}

QByteArray InputEvent::toMSFSShortcut(const QKeySequence &sequence) noexcept {
    QStringList keys = sequence.toString().split('+');
    QByteArray shortcut;
    shortcut.reserve(keys.size() * 6);
    bool hasKey {false};
    for (const QString &key: keys) {
        if (hasKey) {
            shortcut.append('+');
        }
        if (InputEventPrivate::qtToMSFS.contains(key)) {
            shortcut.append(InputEventPrivate::qtToMSFS.at(key));
        } else {
            shortcut.append(key.toLatin1());
        }
        hasKey = true;
    }
#ifdef DEBUG
    qDebug() << "InputEvent::toMSFSShortcut: Qt:" << sequence.toString() << "MSFS:" << shortcut;
#endif
    return shortcut;
}
