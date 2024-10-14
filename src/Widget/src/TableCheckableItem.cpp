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
#include <Kernel/Unit.h>
#include "TableCheckableItem.h"

// PUBLIC

TableCheckableItem::TableCheckableItem(bool enable) noexcept
    : QTableWidgetItem()
{
    setChecked(enable);
}

bool TableCheckableItem::isChecked() const noexcept
{
    return checkState() == Qt::CheckState::Checked ? true : false;
}

void TableCheckableItem::setChecked(bool enable) noexcept
{
    const auto checked = enable ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    setCheckState(checked);
}

bool TableCheckableItem::operator<(const QTableWidgetItem &rhs) const noexcept
{
    const bool checked1 = isChecked();
    const bool checked2 = static_cast<const TableCheckableItem &>(rhs).isChecked();
    return checked1 < checked2;
}
