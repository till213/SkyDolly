/**
 * Sky Dolly - The black sheep for your flight recordings
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
#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <memory>

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
    Q_DISABLE_COPY(ActionButton)
    std::unique_ptr<ActionButtonPrivate> d;

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
