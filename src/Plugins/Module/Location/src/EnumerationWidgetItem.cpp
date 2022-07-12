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
#include <cstdint>

#include <QTableWidgetItem>
#include <QStringBuilder>

#include "EnumerationWidgetItem.h"

// PUBLIC

EnumerationWidgetItem::EnumerationWidgetItem()
    : QTableWidgetItem(QTableWidgetItem::UserType)
{}

QVariant EnumerationWidgetItem::data(int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        return m_symbolicId;
        break;
    case Qt::EditRole:
    {
        return QVariant::fromValue(m_id);
        break;
    }
    default:
        return QTableWidgetItem::data(role);
        break;
    }
    return QVariant();
}

void EnumerationWidgetItem::setData(int role, const QVariant &value)
{
    switch (role)
    {
    case Qt::DisplayRole:
        m_symbolicId = value.toString();
        break;
    case Qt::EditRole:
    {
        m_id = value.toLongLong();
        m_symbolicId = "TODO Symbolic ID";
        break;
    }
    default:
        break;
    }
    QTableWidgetItem::setData(role, value);
}
