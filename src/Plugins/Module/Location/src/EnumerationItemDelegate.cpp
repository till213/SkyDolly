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
#include <memory>
#include <utility>

#include <QStyledItemDelegate>
#include <QModelIndex>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Widget/EnumerationComboBox.h>

#include <Model/Enumeration.h>
#include "EnumerationItemDelegate.h"

struct EnumerationItemDelegatePrivate
{
    EnumerationItemDelegatePrivate(QString theEnumerationName)
        : enumerationName(std::move(theEnumerationName))
    {}

    QString enumerationName;
};

// PUBLIC

EnumerationItemDelegate::EnumerationItemDelegate(QString enumerationName) noexcept
    : QStyledItemDelegate(),
      d(std::make_unique<EnumerationItemDelegatePrivate>(std::move(enumerationName)))
{
#ifdef DEBUG
    qDebug() << "EnumerationItemDelegate::EnumerationItemDelegate: CREATED";
#endif
}

EnumerationItemDelegate::~EnumerationItemDelegate() noexcept
{
#ifdef DEBUG
    qDebug() << "EnumerationItemDelegate::~EnumerationItemDelegate: DELETED";
#endif
}

QWidget *EnumerationItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().canConvert<QString>()) {
        EnumerationComboBox *enumerationComboBox = new EnumerationComboBox(d->enumerationName, parent);
        connect(enumerationComboBox, &EnumerationComboBox::currentIndexChanged,
                         this, &EnumerationItemDelegate::commitAndCloseEditor);
        return enumerationComboBox;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void EnumerationItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const noexcept
{
    const QVariant data = index.data(Qt::EditRole);
    if (data.canConvert<std::int64_t>()) {
         std::int64_t id = qvariant_cast<std::int64_t>(data);
         EnumerationComboBox *enumerationEditor = qobject_cast<EnumerationComboBox *>(editor);
         enumerationEditor->setCurrentId(id);
     } else {
         QStyledItemDelegate::setEditorData(editor, index);
     }
}

void EnumerationItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const noexcept
{
    if (index.data().canConvert<std::int64_t>()) {
         EnumerationComboBox *enumerationEditor = qobject_cast<EnumerationComboBox *>(editor);
         model->setData(index, QVariant::fromValue(enumerationEditor->getCurrentId()));
     } else {
         QStyledItemDelegate::setModelData(editor, model, index);
    }
}

// PRIVATE SLOTS

void EnumerationItemDelegate::commitAndCloseEditor() noexcept
{
    EnumerationComboBox *editor = qobject_cast<EnumerationComboBox *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

