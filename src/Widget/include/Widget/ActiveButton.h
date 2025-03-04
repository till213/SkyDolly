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
#ifndef ACTIVEBUTTON_H
#define ACTIVEBUTTON_H

#include <memory>

#include <QPushButton>

class QMouseEvent;
class QIcon;

#include "WidgetLib.h"

struct ActiveButtonPrivate;

/*!
 * Displays the \e active icon when the button is pressed.
 *
 * Implementation note: the default QPushButton does not seem
 * to honour the "active" pixmap in the given QIcon. Also refer
 * to:
 * https://stackoverflow.com/questions/40318759/change-qpushbutton-icon-on-hover-and-pressed
 */
class WIDGET_API ActiveButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ActiveButton(QWidget *parent) noexcept;
    ActiveButton(const ActiveButton &rhs) = delete;
    ActiveButton(ActiveButton &&rhs) = delete;
    ActiveButton &operator=(const ActiveButton &rhs) = delete;
    ActiveButton &operator=(ActiveButton &&rhs) = delete;
    ~ActiveButton() override;

protected:
    void mousePressEvent(QMouseEvent *e) noexcept override;
    void mouseReleaseEvent(QMouseEvent *e) noexcept override;

private:
    const std::unique_ptr<ActiveButtonPrivate> d;
};

#endif // ACTIVEBUTTON_H
