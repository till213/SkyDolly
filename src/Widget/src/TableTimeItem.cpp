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
#include <QTime>

#include <Kernel/Unit.h>
#include "TableTimeItem.h"

// PUBLIC

TableTimeItem::TableTimeItem(QTime time) noexcept
    : QTableWidgetItem()
{
    setTime(time);
}

QTime TableTimeItem::getTime() const noexcept
{
    return data(Qt::EditRole).toTime();
}

void TableTimeItem::setTime(QTime time) noexcept
{
    setData(Qt::DisplayRole, m_unit.formatTime(time));
    setData(Qt::EditRole, time);
}

bool TableTimeItem::operator<(const QTableWidgetItem &rhs) const noexcept
{
    const QTime time1 = data(Qt::EditRole).toTime();
    const QTime time2 = rhs.data(Qt::EditRole).toTime();
    return time1 < time2;
}
