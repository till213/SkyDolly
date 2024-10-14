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
#ifndef TABLEBOOLEANITEM_H
#define TABLEBOOLEANITEM_H

#include <QTableWidgetItem>

#include <Kernel/Unit.h>
#include "WidgetLib.h"

/*!
 * Sortable QTableWidgetItem displaying a QDate.
 *
 * https://linux.m2osw.com/sorting-any-numeric-column-qtablewidget
 */
class WIDGET_API TableCheckableItem final : public QTableWidgetItem
{
public:
    explicit TableCheckableItem(bool enable = false) noexcept;
    TableCheckableItem(const TableCheckableItem &rhs) = delete;
    TableCheckableItem(TableCheckableItem &&rhs) = delete;
    TableCheckableItem &operator=(const TableCheckableItem &rhs) = delete;
    TableCheckableItem &operator=(TableCheckableItem &&rhs) = delete;
    ~TableCheckableItem() override = default;

    bool isChecked() const noexcept;
    void setChecked(bool enable) noexcept;

    bool operator<(const QTableWidgetItem &rhs) const noexcept override;
};

#endif // TABLEBOOLEANITEM_H
