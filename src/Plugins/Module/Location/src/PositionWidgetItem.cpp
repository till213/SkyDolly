/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <QTableWidgetItem>
#include <QStringBuilder>

#include <Kernel/Unit.h>
#include "PositionWidgetItem.h"

// PUBLIC

PositionWidgetItem::PositionWidgetItem()
    : QTableWidgetItem(QTableWidgetItem::UserType)
{

}

QVariant PositionWidgetItem::data(int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        return m_position;
        break;
    case Qt::EditRole:
    {
        const QString editString = m_unit.formatCoordinates(m_latitude, m_longitude);
        return QVariant::fromValue(editString);
        break;
    }
    default:
        return QTableWidgetItem::data(role);
        break;
    }
    return QVariant();
}

void PositionWidgetItem::setData(int role, const QVariant &value)
{
    switch (role)
    {
    case Qt::DisplayRole:
        m_position = value.toString();
        break;
    case Qt::EditRole:
    {
        QStringList values = value.toString().split(',');
        m_latitude = values.first().toFloat();
        m_longitude = values.last().toFloat();
        m_position = m_unit.formatLatLongPositionDMS(m_latitude, m_longitude);
        break;
    }
    default:
        QTableWidgetItem::setData(role, value);
        break;
    }
}
