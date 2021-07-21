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
#ifndef ACTIONCHECKBOX_H
#define ACTIONCHECKBOX_H

#include <memory>

#include <QObject>
#include <QCheckBox>

#include "WidgetLib.h"

class ActionCheckBoxPrivate;

/*!
 * An extension of a QRadioButton that supports QAction.
 * This class represents a QRadioButton extension that can be
 * connected to an action and that configures itself depending
 * on the status of the action.
 * When the action changes its state, the radio button reflects
 * such changes, and when the button is clicked the action
 * is triggered.
 *
 * Based on: https://wiki.qt.io/PushButton_Based_On_Action
 */
class WIDGET_API ActionCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit ActionCheckBox(bool withText, QWidget *parent = nullptr) noexcept;
    virtual ~ActionCheckBox() noexcept;

    /*!
     * Sets the action to be associated with this radio button. This radio button is
     * configured immediately according to the action status and the button and
     * the action are connected together so that when the action is changed the button
     * is updated and when the button is clicked the action is triggered.
     *
     * \p action
     *    the action to be associated with this button
     */
    void setAction(const QAction *action) noexcept;

private:
    Q_DISABLE_COPY(ActionCheckBox)
    std::unique_ptr<ActionCheckBoxPrivate> d;

private slots:

   /*!
    * Update the radio button status depending on a change
    * on the action status. This slot is invoked each time the action
    * "changed" signal is emitted.
    */
   void updateButtonStatusFromAction() noexcept;

   void connectToAction() noexcept;

   /*!
    * Disassociates this radio button from the action, typically when the
    * action is deleted.
    */
   void disconnectFromAction() noexcept;
};

#endif // ACTIONCHECKBOX_H
