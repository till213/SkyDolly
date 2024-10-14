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
#ifndef UNITWIDGETITEM_H
#define UNITWIDGETITEM_H

#include <cstdint>

#include <QTableWidgetItem>
#include <QVariant>

#include <Kernel/Unit.h>
#include "WidgetLib.h"

class Unit;

/*!
 * The UnitWidgetItem stores values  of the given \c enumeration and displays
 * their names in table cells.
 */
class WIDGET_API UnitWidgetItem : public QTableWidgetItem
{
public:
    UnitWidgetItem(const Unit &unit, Unit::Name name);

    QVariant data(int role) const override;
    void setData(int role, const QVariant &value) override;

private:
    QVariant m_value;
    QString m_displayValue;
    Unit::Name m_unitName;
    const Unit &m_unit;
};

#endif // UNITWIDGETITEM_H
