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
#include <QRadioButton>
#include <QAction>

#include "ActionRadioButton.h"

struct ActionRadioButtonPrivate
{
    const QAction *action {nullptr};
};

// PUBLIC

ActionRadioButton::ActionRadioButton(QWidget *parent) noexcept
    : QRadioButton {parent},
      d {std::make_unique<ActionRadioButtonPrivate>()}
{}

ActionRadioButton::~ActionRadioButton() = default;

void ActionRadioButton::setAction(const QAction *action) noexcept
{
    // If an action is already associated with the button then
    // disconnect all previous connections
    if (d->action != nullptr && d->action != action) {
        disconnectFromAction();
    }

    if (d->action != action) {
        d->action = action;
        updateButtonStatusFromAction();
        connectToAction();
    }
}

// PRIVATE SLOTS

void ActionRadioButton::updateButtonStatusFromAction() noexcept
{
    if (d->action != nullptr) {
        setText(d->action->text());
        setStatusTip(d->action->statusTip());
        setToolTip(d->action->toolTip());
        setIcon(d->action->icon());
        setEnabled(d->action->isEnabled());
        setCheckable(d->action->isCheckable());
        setChecked(d->action->isChecked());
    }
}

void ActionRadioButton::connectToAction() noexcept
{
    if (d->action != nullptr) {
        // React to the action state changes
        connect(d->action, &QAction::changed,
                this, &ActionRadioButton::updateButtonStatusFromAction);
        connect(d->action, &QAction::destroyed,
                this, &ActionRadioButton::disconnectFromAction);
        connect(this, &ActionRadioButton::clicked,
                d->action, &QAction::trigger);
    }
}

void ActionRadioButton::disconnectFromAction() noexcept
{
    if (d->action != nullptr) {
        disconnect(d->action, &QAction::changed,
                   this, &ActionRadioButton::updateButtonStatusFromAction);
        disconnect(d->action, &QAction::destroyed,
                   this, &ActionRadioButton::disconnectFromAction);
        disconnect(this, &ActionRadioButton::clicked,
                   d->action, &QAction::trigger);
        d->action = nullptr;
    }
}
