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

#include <cstdint>

#include <QTableWidgetItem>
#include <QStringBuilder>
#include <QVariant>

#include <Kernel/Unit.h>
#include "UnitWidgetItem.h"

// PUBLIC

UnitWidgetItem::UnitWidgetItem(const Unit &unit, Unit::Name name)
    : QTableWidgetItem(QTableWidgetItem::UserType),
      m_unit(unit),
      m_unitName(name)
{}

QVariant UnitWidgetItem::data(int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return m_displayValue;
        break;
    case Qt::EditRole:
    {
        return m_value;
        break;
    }
    default:
        return QTableWidgetItem::data(role);
        break;
    }
    return {};
}

void UnitWidgetItem::setData(int role, const QVariant &value)
{
    switch (role) {
    case Qt::DisplayRole:
        m_displayValue = value.toString();
        break;
    case Qt::EditRole:
    {
        m_value = value;
        switch (m_unitName) {
        case Unit::Name::Second:
        {
            const double seconds = m_value.toDouble();
            m_displayValue = m_unit.formatSeconds(seconds);
            break;
        }
        case Unit::Name::Feet:
        {
            const double feet = m_value.toDouble();
            m_displayValue = m_unit.formatFeet(feet);
            break;
        }
        case Unit::Name::Knot:
            const double knots = m_value.toDouble();
            m_displayValue = m_unit.formatKnots(knots);
            break;
        }
        break;
    }
    default:
        break;
    }
    QTableWidgetItem::setData(role, value);
}
