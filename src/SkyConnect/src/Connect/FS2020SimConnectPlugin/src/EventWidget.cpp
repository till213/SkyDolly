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
#include <windows.h>

#include "EventWidget.h"

// PUBLIC

EventWidget::EventWidget(QWidget *parent) noexcept
    : QWidget(parent)
{
#ifdef DEBUG
    qDebug("EventWidget: CREATED");
#endif
}

EventWidget::~EventWidget() noexcept
{
#ifdef DEBUG
    qDebug("EventWidget: DELETED");
#endif
}

// PROTECTED

bool EventWidget::nativeEvent(const QByteArray &eventType, void *message, long *result) noexcept
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)

    bool handled;
    const MSG *msg = static_cast<MSG *>(message);
    switch(msg->message) {
    case SimConnnectUserMessage:
        emit simConnectEvent();
        handled = true;
        break;
    default:
        handled = false;
        break;
    }
    return handled;
}
