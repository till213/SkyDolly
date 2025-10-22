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
#ifndef TABLEDURATIONITEM_H
#define TABLEDURATIONITEM_H

#include <cstdint>

#include <QTableWidgetItem>

#include <Kernel/Unit.h>
#include "WidgetLib.h"

/*!
 * Sortable QTableWidgetItem displaying duration.
 *
 * https://linux.m2osw.com/sorting-any-numeric-column-qtablewidget
 */
class WIDGET_API TableDurationItem final : public QTableWidgetItem
{
public:
    explicit TableDurationItem(std::int64_t duration = 0) noexcept;
    TableDurationItem(const TableDurationItem &rhs) = delete;
    TableDurationItem(TableDurationItem &&rhs) = delete;
    TableDurationItem &operator=(const TableDurationItem &rhs) = delete;
    TableDurationItem &operator=(TableDurationItem &&rhs) = delete;
    ~TableDurationItem() override = default;

    std::int64_t getDuration() const noexcept;
    void setDuration(std::int64_t duration) noexcept;

    bool operator<(const QTableWidgetItem &rhs) const noexcept override;

private:
    Unit m_unit;
};

#endif // TABLEDURATIONITEM_H
