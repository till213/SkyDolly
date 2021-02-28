#include <QPushButton>
#include <QAction>

#include "ActionButton.h"

class ActionButtonPrivate
{
public:
    ActionButtonPrivate()
        : action(nullptr)
    {}

    const QAction *action;
};

// PUBLIC

ActionButton::ActionButton(QWidget *parent)
    : QPushButton(parent),
      d(new ActionButtonPrivate())
{

}

ActionButton::~ActionButton()
{
    delete d;
}

void ActionButton::setAction(const QAction *action)
{
   // If an action is already associated with the button then
   // remove all previous connections
   if (d->action != nullptr && d->action != action) {
       disconnectFromAction();
   }

   if (d->action != action) {
       d->action = action;
       updateButtonStatusFromAction();
       connectToAction();
   }
}

void ActionButton::updateButtonStatusFromAction()
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

void ActionButton::connectToAction()
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

void ActionButton::disconnectFromAction()
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
