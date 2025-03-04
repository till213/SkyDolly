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
#include <memory>
#include <utility>

#include <QStyledItemDelegate>
#include <QModelIndex>

#include <Widget/EnumerationComboBox.h>

#include <Model/Enumeration.h>
#include "EnumerationItemDelegate.h"

struct EnumerationItemDelegatePrivate
{
    EnumerationItemDelegatePrivate(QString enumerationName)
        : enumerationName(std::move(enumerationName))
    {}

    QString enumerationName;
};

// PUBLIC

EnumerationItemDelegate::EnumerationItemDelegate(QString enumerationName) noexcept
    : QStyledItemDelegate(),
      d {std::make_unique<EnumerationItemDelegatePrivate>(std::move(enumerationName))}
{}

EnumerationItemDelegate::~EnumerationItemDelegate() = default;

QWidget *EnumerationItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *editor {nullptr};
    if (index.data().canConvert<QString>()) {
        auto *enumerationComboBox = new EnumerationComboBox(d->enumerationName, EnumerationComboBox::Mode::Editable, parent);
        connect(enumerationComboBox, &EnumerationComboBox::currentIndexChanged,
                this, &EnumerationItemDelegate::commitAndCloseEditor);
        editor = enumerationComboBox;
    } else {
        editor = QStyledItemDelegate::createEditor(parent, option, index);
    }
    return editor;
}

void EnumerationItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const noexcept
{
    const QVariant data = index.data(Qt::EditRole);
    if (data.canConvert<std::int64_t>()) {
         const auto id = qvariant_cast<std::int64_t>(data);
         auto *enumerationEditor = qobject_cast<EnumerationComboBox *>(editor);
         enumerationEditor->blockSignals(true);
         enumerationEditor->setCurrentId(id);
         enumerationEditor->blockSignals(false);
     } else {
         QStyledItemDelegate::setEditorData(editor, index);
     }
}

void EnumerationItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const noexcept
{
    if (index.data().canConvert<std::int64_t>()) {
         const auto enumerationEditor = qobject_cast<EnumerationComboBox *>(editor);
         model->setData(index, QVariant::fromValue(enumerationEditor->getCurrentId()));
     } else {
         QStyledItemDelegate::setModelData(editor, model, index);
    }
}

// PRIVATE SLOTS

void EnumerationItemDelegate::commitAndCloseEditor() noexcept
{
    auto editor = qobject_cast<EnumerationComboBox *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
