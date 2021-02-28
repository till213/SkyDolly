#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QObject>
#include <QPushButton>

class ActionButtonPrivate;

/*!
 * An extension of a QPushButton that supports QAction.
 * This class represents a QPushButton extension that can be
 * connected to an action and that configures itself depending
 * on the status of the action.
 * When the action changes its state, the button reflects
 * such changes, and when the button is clicked the action
 * is triggered.
 *
 * Source: https://wiki.qt.io/PushButton_Based_On_Action
 */
class ActionButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ActionButton(QWidget *parent = nullptr);
    virtual ~ActionButton();

    /*!
     * Sets the action to be associated with this button. This button is
     * configured immediately according to the action status and the button and
     * the action are connected together so that when the action is changed the button
     * is updated and when the button is clicked the action is triggered.
     *
     * \p action
     *    the action to be associated with this button
     */
    void setAction(const QAction *action);

private:
    ActionButtonPrivate *d;

private slots:

   /*!
    * Update the button status depending on a change
    * on the action status. This slot is invoked each time the action
    * "changed" signal is emitted.
    */
   void updateButtonStatusFromAction();

   void connectToAction();

   /*!
    * Disassociates this button from the action, typically when the
    * action is deleted.
    */
   void disconnectFromAction();

};

#endif // ACTIONBUTTON_H
