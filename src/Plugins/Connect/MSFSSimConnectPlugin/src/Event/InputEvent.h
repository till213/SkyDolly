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
#ifndef INPUTEVENT_H
#define INPUTEVENT_H

#include <windows.h>
#include <SimConnect.h>

#include <Kernel/Enum.h>
#include <Kernel/FlightSimulatorShortcuts.h>
#include "SimConnectEvent.h"

class InputEvent
{
public:
    static bool setup(HANDLE simConnectHandle, const FlightSimulatorShortcuts &shortcuts) noexcept
    {
        // Recording
        QByteArray shortcut = toMSFSShortcut(shortcuts.record);
        HRESULT result = ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomRecording), "Custom.Recording");
        result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomRecording));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::SkyDollyShortcuts), Enum::underly(SimConnectEvent::Event::CustomRecording));

        // Replay
        shortcut = toMSFSShortcut(shortcuts.replay);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomReplay), "Custom.Replay");
        result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomReplay));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::SkyDollyShortcuts), Enum::underly(SimConnectEvent::Event::CustomReplay));

        // Pause
        shortcut = toMSFSShortcut(shortcuts.pause);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomPause), "Custom.Pause");
        result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomPause));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::SkyDollyShortcuts), Enum::underly(SimConnectEvent::Event::CustomPause));

        // Stop
        shortcut = toMSFSShortcut(shortcuts.stop);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomStop), "Custom.Stop");
        result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomStop));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::SkyDollyShortcuts), Enum::underly(SimConnectEvent::Event::CustomStop));

        // Backward
        shortcut = toMSFSShortcut(shortcuts.backward);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomBackward), "Custom.Backward");
        result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomBackward));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::SkyDollyShortcuts), Enum::underly(SimConnectEvent::Event::CustomBackward));

        // Forward
        shortcut = toMSFSShortcut(shortcuts.forward);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomForward), "Custom.Forward");
        result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomForward));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::SkyDollyShortcuts), Enum::underly(SimConnectEvent::Event::CustomForward));

        // Begin
        shortcut = toMSFSShortcut(shortcuts.begin);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomBegin), "Custom.Begin");
        result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomBegin));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::SkyDollyShortcuts), Enum::underly(SimConnectEvent::Event::CustomBegin));

        // End
        shortcut = toMSFSShortcut(shortcuts.end);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomEnd), "Custom.End");
        result |= ::SimConnect_MapInputEventToClientEvent(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomEnd));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::SkyDollyShortcuts), Enum::underly(SimConnectEvent::Event::CustomEnd));

        result |= ::SimConnect_SetNotificationGroupPriority(simConnectHandle, Enum::underly(NotificationGroup::SkyDollyShortcuts), SIMCONNECT_GROUP_PRIORITY_HIGHEST);
        result |= ::SimConnect_SetInputGroupState(simConnectHandle, Enum::underly(Input::SkyDollyControl), SIMCONNECT_STATE_ON);

        return result == S_OK;
    }

private:
    enum struct Input: ::SIMCONNECT_INPUT_GROUP_ID {
        SkyDollyControl,
    };

    enum struct NotificationGroup: ::SIMCONNECT_NOTIFICATION_GROUP_ID {
        SkyDollyShortcuts,
    };

    static QByteArray toMSFSShortcut(const QKeySequence &sequence) noexcept {
        QString portableFormat = sequence.toString();
        // TODO IMPLEMENT ME (ALT, SHIFT, ...)
        QByteArray shortcut = portableFormat.replace("Ctrl", "VK_LCONTROL")
                                  .replace("Alt", "VK_LMENU")
                                  .replace("Shift", "VK_LSHIFT")
                                  .toLatin1();
        return shortcut;
    }

};

#endif // INPUTEVENT_H
