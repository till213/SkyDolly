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
#include <QPushButton>
#include <QAction>

#include "ActionButton.h"

struct ActionButtonPrivate
{
    ActionButtonPrivate(ActionButton::Capitalisation theCapitalisation)  noexcept
        : capitalisation(theCapitalisation)
    {}

    const QAction *action {nullptr};
    bool showText {true};
    ActionButton::Capitalisation capitalisation;
};

// PUBLIC

ActionButton::ActionButton(QWidget *parent, Capitalisation capitalisation) noexcept
    : ActiveButton(parent),
      d(std::make_unique<ActionButtonPrivate>(capitalisation))
{
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

ActionButton::~ActionButton() noexcept
{}

void ActionButton::setAction(const QAction *action) noexcept
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

bool ActionButton::isShowText() const noexcept
{
    return d->showText;
}

void ActionButton::setShowText(bool enable) noexcept
{
    if (d->showText != enable) {
        d->showText = enable;
        updateText();
    }
}

// PRIVATE SLOTS

void ActionButton::updateButtonStatusFromAction() noexcept
{
    if (d->action != nullptr) {
        updateText();
        setStatusTip(d->action->statusTip());
        setToolTip(d->action->toolTip());
        setIcon(d->action->icon());
        setEnabled(d->action->isEnabled());
        setCheckable(d->action->isCheckable());
        setChecked(d->action->isChecked());
    }
}

void ActionButton::updateText() noexcept
{
    if (d->showText) {
        switch (d->capitalisation) {
        case Capitalisation::Normal:
            setText(d->action->text());
            break;
        case Capitalisation::AllCaps:
            setText(d->action->text().toUpper());
            break;
        }
    } else {
        setText(QString());
    }
}

void ActionButton::connectToAction() noexcept
{
   if (d->action != nullptr) {
       // React to the action state changes
       connect(d->action, &QAction::changed,
               this, &ActionButton::updateButtonStatusFromAction);
       connect(d->action, &QAction::destroyed,
               this, &ActionButton::disconnectFromAction);
       connect(this, &ActionButton::clicked,
               d->action, &QAction::trigger);
   }
}

void ActionButton::disconnectFromAction() noexcept
{
   if (d->action != nullptr) {
       disconnect(d->action, &QAction::changed,
                  this, &ActionButton::updateButtonStatusFromAction);
       disconnect(d->action, &QAction::destroyed,
                  this, &ActionButton::disconnectFromAction);
       disconnect(this, &ActionButton::clicked,
                  d->action, &QAction::trigger);
       d->action = nullptr;
   }
}
