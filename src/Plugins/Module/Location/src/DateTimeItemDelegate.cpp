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
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QDateTimeEdit>

#include "DateTimeItemDelegate.h"

// PUBLIC

DateTimeItemDelegate::DateTimeItemDelegate() noexcept
    : QStyledItemDelegate()
{}

DateTimeItemDelegate::~DateTimeItemDelegate() = default;

QWidget *DateTimeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto *dateTimeEdit = new QDateTimeEdit(parent);
    connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged,
            this, &DateTimeItemDelegate::commitAndCloseEditor);
    return dateTimeEdit;
}

void DateTimeItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const noexcept
{
    const QVariant data = index.data(Qt::EditRole);
    if (data.canConvert<QDateTime>()) {
         const auto dateTime = qvariant_cast<QDateTime>(data);
         auto *dateTimeEditor = qobject_cast<QDateTimeEdit *>(editor);
         dateTimeEditor->blockSignals(true);
         dateTimeEditor->setDateTime(dateTime);
         dateTimeEditor->blockSignals(false);
     } else {
         QStyledItemDelegate::setEditorData(editor, index);
     }
}

void DateTimeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const noexcept
{
    if (index.data().canConvert<QDateTime>()) {
        const auto dateTimeEditor = qobject_cast<QDateTimeEdit *>(editor);
        model->setData(index, QVariant::fromValue(dateTimeEditor->dateTime()));
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

// PRIVATE SLOTS

void DateTimeItemDelegate::commitAndCloseEditor() noexcept
{
    auto editor = qobject_cast<QDateTimeEdit *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
