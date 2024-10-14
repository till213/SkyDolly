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
#include <QWidget>
#include <QTextEdit>
#include <QApplication>
#include <QMouseEvent>
#include <QDesktopServices>

#include "HyperlinkTextEdit.h"

// PUBLIC

HyperlinkTextEdit::HyperlinkTextEdit(QWidget *parent)
    : QTextEdit {parent}
{
    setMouseTracking(true);
}

// PROTECTED

void HyperlinkTextEdit::mouseMoveEvent(QMouseEvent *event) noexcept
{
    QTextEdit::mouseMoveEvent(event);
    if (!anchorAt(event->pos()).isNull()) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
    } else {
        QApplication::restoreOverrideCursor();
    }
}

void HyperlinkTextEdit::mousePressEvent(QMouseEvent *event) noexcept
{
    QTextEdit::mousePressEvent(event);
    m_anchor = anchorAt(event->pos());
    if (!m_anchor.isNull()) {
        QApplication::setOverrideCursor(Qt::PointingHandCursor);
    }
}

void HyperlinkTextEdit::mouseReleaseEvent(QMouseEvent *event) noexcept
{
    QTextEdit::mouseReleaseEvent(event);
    if (!m_anchor.isNull()) {
        QDesktopServices::openUrl(QUrl(m_anchor));
        QApplication::restoreOverrideCursor();
        m_anchor = QString();
    }
}
