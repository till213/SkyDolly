/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QTime>
#include <QVariant>

#include <Widget/FocusTimeEdit.h>
#include "TimeItemDelegate.h"

// PUBLIC

TimeItemDelegate::TimeItemDelegate() noexcept
    : QStyledItemDelegate()
{}

TimeItemDelegate::~TimeItemDelegate() = default;

QWidget *TimeItemDelegate::createEditor(QWidget *parent, [[maybe_unused]] const QStyleOptionViewItem &option, [[maybe_unused]] const QModelIndex &index) const
{
    auto *timeEdit = new FocusTimeEdit(parent);
    connect(timeEdit, &FocusTimeEdit::focusLost,
            this, &TimeItemDelegate::commitAndCloseEditor);
    return timeEdit;
}

void TimeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const noexcept
{
    const auto data = index.data(Qt::EditRole);
    QTime time;
    if (data.canConvert<QTime>()) {
        time = qvariant_cast<QTime>(data);
    }
    if (!time.isValid()) {
        time = QTime::currentTime();
    }

    auto *timeEditor = qobject_cast<FocusTimeEdit *>(editor);
    timeEditor->blockSignals(true);
    timeEditor->setTime(time);
    timeEditor->blockSignals(false);
}

void TimeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const noexcept
{
    if (index.data().canConvert<QTime>()) {
        const auto timeEditor = qobject_cast<FocusTimeEdit *>(editor);
        model->setData(index, QVariant::fromValue(timeEditor->time()));
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

// PRIVATE SLOTS

void TimeItemDelegate::commitAndCloseEditor() noexcept
{
    auto editor = qobject_cast<FocusTimeEdit *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
