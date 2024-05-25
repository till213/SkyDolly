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
#include <QFlags>
#include <QStringList>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Enum.h>
#include <PluginManager/Connect/FlightSimulatorShortcuts.h>
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

    enum struct KeySequenceState: std::uint32_t
    {
        None = 0x0,
        Record = 0x1,
        Replay = 0x2,
        Pause = 0x4,
        Stop = 0x8,
        Forward = 0x8,
        Backward = 0x10,
        Begin = 0x20,
        End = 0x40,
        All = 0xffffffff
    };
    Q_DECLARE_FLAGS(KeySequenceStates, KeySequenceState)
}

// PRIVATE

struct InputEventPrivate
{
    bool isInitialised {false};
    KeySequenceStates keySequenceStates {KeySequenceState::None};

    // https://docs.flightsimulator.com/html/Programming_Tools/SimConnect/API_Reference/InputEvents/SimConnect_MapInputEventToClientEvent_EX1.htm
    const std::unordered_map<Qt::Key, QByteArray> qtToMSFS {
        { Qt::Key_Backspace, "Backspace" },
        { Qt::Key_Tab, "Tab" },
        // Note: the SimConnect documentation says "VK_Enter", but it is really "Enter"
        { Qt::Key_Enter, "Enter" },
        { Qt::Key_Return, "Enter" },
        { Qt::Key_Pause, "VK_PAUSE" },
        { Qt::Key_CapsLock, "Caps_Lock" },
        { Qt::Key_Escape, "Esc" },
        { Qt::Key_Space, "Space" },
        { Qt::Key_PageUp, "VK_PRIOR" },
        { Qt::Key_PageDown, "VK_NEXT" },
        { Qt::Key_End, "VK_END" },
        { Qt::Key_Home, "VK_HOME" },
        { Qt::Key_Left, "VK_LEFT" },
        { Qt::Key_Up, "VK_UP" },
        { Qt::Key_Right, "VK_RIGHT" },
        { Qt::Key_Down, "VK_DOWN" },
        { Qt::Key_Select, "VK_SELECT" },
        { Qt::Key_Print, "VK_PRINT" },
        { Qt::Key_Execute, "VK_EXECUTE" },
        { Qt::Key_SysReq, "Sys_Req" },
        { Qt::Key_Insert, "VK_INSERT" },
        { Qt::Key_Delete, "VK_DELETE" },
        { Qt::Key_Help, "VK_HELP" },
        { Qt::Key_Meta, "VK_LWIN" },
        { Qt::Key_Sleep, "VK_SLEEP" },
        { Qt::Key_multiply, "VK_MULTIPLY" },
        { Qt::Key_Asterisk, "VK_MULTIPLY" },
        { Qt::Key_division, "VK_DIVIDE" },
        { Qt::Key_Pause, "Pause" },
        { Qt::Key_ScrollLock, "VK_SCROLL" },
        { Qt::Key_Shift, "VK_LSHIFT" },
        { Qt::Key_Control, "VK_LCONTROL" },
        { Qt::Key_Alt, "VK_LMENU" },
        { Qt::Key_AltGr, "VK_RMENU" },
        { Qt::Key_VolumeMute, "VK_VOLUME_MUTE" },
        { Qt::Key_VolumeDown, "VK_VOLUME_DOWN" },
        { Qt::Key_VolumeUp, "VK_VOLUME_UP" },
        { Qt::Key_MediaNext, "VK_MEDIA_NEXT_TRACK" },
        { Qt::Key_MediaPrevious, "VK_MEDIA_PREV_TRACK" },
        { Qt::Key_MediaStop, "VK_MEDIA_STOP" },
        { Qt::Key_MediaTogglePlayPause, "VK_MEDIA_PLAY_PAUSE" },
        { Qt::Key_Semicolon, "VK_SEMICOLON" },
        { Qt::Key_Plus, "VK_PLUS" },
        { Qt::Key_Comma, "VK_COMMA" },
        { Qt::Key_Minus, "VK_MINUS" },
        { Qt::Key_Period, "VK_PERIOD" },
        { Qt::Key_Slash, "VK_SLASH" },
        { Qt::Key_AsciiTilde, "VK_TILDE" },
        { Qt::Key_BracketLeft, "VK_LBRACKET" },
        { Qt::Key_Backslash, "VK_BACKSLASH" },
        { Qt::Key_BracketRight, "VK_RBRACKET" },
        { Qt::Key_QuoteDbl, "VK_QUOTE" },
        { Qt::Key_Play, "VK_PLAY" },
        { Qt::Key_Zoom, "VK_ZOOM" }
    };

    const std::unordered_map<Qt::KeyboardModifier, QByteArray> qtModifierToMSFS {
        { Qt::ShiftModifier, "VK_LSHIFT" },
        { Qt::ControlModifier, "VK_LCONTROL" },
        { Qt::AltModifier, "VK_LMENU" },
        { Qt::MetaModifier, "VK_LWIN" }
    };

    const std::unordered_map<QByteArray, QByteArray> numpadify {
        { "VK_PLUS", "VK_ADD" },
        { "VK_MINUS", "VK_SUBTRACT" },
        { "VK_PERIOD", "VK_DECIMAL" },
        { "VK_SLASH", "VK_DIVIDE" }
    };
};

// PUBLIC

InputEvent::InputEvent() noexcept
    : d {std::make_unique<InputEventPrivate>()}
{}

InputEvent::~InputEvent() = default;

bool InputEvent::setup(HANDLE simConnectHandle, const FlightSimulatorShortcuts &shortcuts) noexcept
{
    HRESULT result {S_OK};
    if (d->isInitialised) {
        result = clear(simConnectHandle) ? S_OK : S_FALSE;
    }

    QByteArray shortcut;
    // Recording
    if (shortcuts.record.count() > 0) {
        shortcut = toMSFSShortcut(shortcuts.record);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomRecord), nullptr);
        result |= ::SimConnect_MapInputEventToClientEvent_EX1(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomRecord));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomRecord));
        d->keySequenceStates.setFlag(::KeySequenceState::Record);
    }

    // Replay
    if (shortcuts.replay.count() > 0) {
        shortcut = toMSFSShortcut(shortcuts.replay);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomReplay), nullptr);
        result |= ::SimConnect_MapInputEventToClientEvent_EX1(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomReplay));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomReplay));
        d->keySequenceStates.setFlag(::KeySequenceState::Replay);
    }

    // Pause
    if (shortcuts.pause.count() > 0) {
        shortcut = toMSFSShortcut(shortcuts.pause);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomPause), nullptr);
        result |= ::SimConnect_MapInputEventToClientEvent_EX1(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomPause));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomPause));
        d->keySequenceStates.setFlag(::KeySequenceState::Pause);
    }

    // Stop
    if (shortcuts.stop.count() > 0) {
        shortcut = toMSFSShortcut(shortcuts.stop);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomStop), nullptr);
        result |= ::SimConnect_MapInputEventToClientEvent_EX1(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomStop));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomStop));
        d->keySequenceStates.setFlag(::KeySequenceState::Stop);
    }

    // Backward
    if (shortcuts.backward.count() > 0) {
        shortcut = toMSFSShortcut(shortcuts.backward);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomBackwardDown), nullptr);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomBackwardUp), nullptr);
        result |= ::SimConnect_MapInputEventToClientEvent_EX1(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut,
                                                              Enum::underly(SimConnectEvent::Event::CustomBackwardDown), 0,
                                                              Enum::underly(SimConnectEvent::Event::CustomBackwardUp));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBackwardDown));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBackwardUp));
        d->keySequenceStates.setFlag(::KeySequenceState::Backward);
    }

    // Forward
    if (shortcuts.forward.count() > 0) {
        shortcut = toMSFSShortcut(shortcuts.forward);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomForwardDown), nullptr);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomForwardUp), nullptr);
        result |= ::SimConnect_MapInputEventToClientEvent_EX1(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut,
                                                              Enum::underly(SimConnectEvent::Event::CustomForwardDown), 0,
                                                              Enum::underly(SimConnectEvent::Event::CustomForwardUp));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomForwardDown));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomForwardUp));
        d->keySequenceStates.setFlag(::KeySequenceState::Forward);
    }

    // Begin
    if (shortcuts.begin.count() > 0) {
        shortcut = toMSFSShortcut(shortcuts.begin);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomBegin), nullptr);
        result |= ::SimConnect_MapInputEventToClientEvent_EX1(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomBegin));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBegin));
        d->keySequenceStates.setFlag(::KeySequenceState::Begin);
    }

    // End
    if (shortcuts.end.count() > 0) {
        shortcut = toMSFSShortcut(shortcuts.end);
        result |= ::SimConnect_MapClientEventToSimEvent(simConnectHandle, Enum::underly(SimConnectEvent::Event::CustomEnd), nullptr);
        result |= ::SimConnect_MapInputEventToClientEvent_EX1(simConnectHandle, Enum::underly(Input::SkyDollyControl), shortcut, Enum::underly(SimConnectEvent::Event::CustomEnd));
        result |= ::SimConnect_AddClientEventToNotificationGroup(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomEnd));
        d->keySequenceStates.setFlag(::KeySequenceState::End);
    }

    result |= ::SimConnect_SetNotificationGroupPriority(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), ::SIMCONNECT_GROUP_PRIORITY_HIGHEST);
    result |= ::SimConnect_SetInputGroupPriority(simConnectHandle, Enum::underly(Input::SkyDollyControl), ::SIMCONNECT_GROUP_PRIORITY_HIGHEST);
    result |= ::SimConnect_SetInputGroupState(simConnectHandle, Enum::underly(Input::SkyDollyControl), ::SIMCONNECT_STATE_ON);

    d->isInitialised = result == S_OK;

    return d->isInitialised;
}

bool InputEvent::clear(HANDLE simConnectHandle) noexcept
{
    HRESULT result {S_OK};
    if (d->keySequenceStates.testFlag(::KeySequenceState::Record))
    {
        result = ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomRecord));
    }
    if (d->keySequenceStates.testFlag(::KeySequenceState::Replay))
    {
        result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomReplay));
    }
    if (d->keySequenceStates.testFlag(::KeySequenceState::Pause))
    {
        result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomPause));
    }
    if (d->keySequenceStates.testFlag(::KeySequenceState::Stop))
    {
        result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomStop));
    }
    if (d->keySequenceStates.testFlag(::KeySequenceState::Backward))
    {
        result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBackwardDown));
        result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBackwardUp));
    }
    if (d->keySequenceStates.testFlag(::KeySequenceState::Forward))
    {
        result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomForwardDown));
        result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomForwardUp));
    }
    if (d->keySequenceStates.testFlag(::KeySequenceState::Begin))
    {
        result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomBegin));
    }
    if (d->keySequenceStates.testFlag(::KeySequenceState::End))
    {
        result |= ::SimConnect_RemoveClientEvent(simConnectHandle, Enum::underly(NotificationGroup::ShortcutsGroup), Enum::underly(SimConnectEvent::Event::CustomEnd));
    }
    d->keySequenceStates = ::KeySequenceState::None;
    result |= ::SimConnect_ClearInputGroup(simConnectHandle, Enum::underly(Input::SkyDollyControl));

    d->isInitialised = false;

    return result == S_OK;
}

// PRIVATE

QByteArray InputEvent::toMSFSShortcut(const QKeySequence &sequence) const noexcept
{
    QByteArray shortcut;

    const QKeyCombination combination = sequence[0];
    const auto modifier = combination.keyboardModifiers();
    bool numpadModifier {false};
    if (modifier != Qt::NoModifier) {
        if (modifier.testFlag(Qt::ShiftModifier)) {
            auto msfsModifier = d->qtModifierToMSFS.at(Qt::ShiftModifier);
            shortcut.append(msfsModifier).append('+');
        }
        if (modifier.testFlag(Qt::ControlModifier)) {
            auto msfsModifier = d->qtModifierToMSFS.at(Qt::ControlModifier);
            shortcut.append(msfsModifier).append('+');
        }
        if (modifier.testFlag(Qt::AltModifier)) {
            auto msfsModifier = d->qtModifierToMSFS.at(Qt::AltModifier);
            shortcut.append(msfsModifier).append('+');
        }
        if (modifier.testFlag(Qt::MetaModifier)) {
            auto msfsModifier = d->qtModifierToMSFS.at(Qt::MetaModifier);
            shortcut.append(msfsModifier).append('+');
        }
        if (modifier.testFlag(Qt::KeypadModifier)) {
            numpadModifier = true;
        }
    }

    const auto key = combination.key();
    if (d->qtToMSFS.contains(key)) {
        auto msfsKey = d->qtToMSFS.at(key);
        if (numpadModifier) {
            msfsKey =  d->numpadify.contains(msfsKey) ? d->numpadify.at(msfsKey) : msfsKey;
        }
        shortcut.append(msfsKey);
    } else {
        const auto keys = sequence.toString().split('+');
        shortcut.append(keys.last().toLatin1());
    }
#ifdef DEBUG
    qDebug() << "InputEvent::toMSFSShortcut: Qt:" << sequence.toString() << "key:" << key << "MSFS:" << shortcut;
#endif
    return shortcut;
}
