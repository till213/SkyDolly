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
#include <QPushButton>
#include <QMouseEvent>
#include <QIcon>

#include "ActiveButton.h"

class ActiveButtonPrivate
{
public:
    ActiveButtonPrivate()
    {}

    QPixmap normalPixmap;
    QPixmap activePixmap;
};

// PUBLIC

ActiveButton::ActiveButton(QWidget *parent) noexcept
    : QPushButton(parent),
      d(std::make_unique<ActiveButtonPrivate>())
{
    setAutoDefault(false);
}

ActiveButton::~ActiveButton() noexcept
{}

// PROTECTED

void ActiveButton::mousePressEvent(QMouseEvent *e) noexcept
{
    QPushButton::mousePressEvent(e);

    const QPixmap currentPixmap = icon().pixmap(iconSize(), QIcon::Normal);
    if (d->normalPixmap.isNull() || d->normalPixmap != currentPixmap) {
        // The icon of the QAction has been changed -> update the pixmap cache
        d->normalPixmap = currentPixmap;
        d->activePixmap = icon().pixmap(iconSize(), QIcon::Active);
    }
    QPushButton::setIcon(d->activePixmap);
}

void ActiveButton::mouseReleaseEvent(QMouseEvent *e) noexcept
{
    setIcon(d->normalPixmap);
    QPushButton::mouseReleaseEvent(e);
}
