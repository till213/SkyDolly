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
#include <QWidget>
#include <QComboBox>

#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include "BackupPeriodComboBox.h"

class BackupPeriodComboBoxPrivate
{
public:
    BackupPeriodComboBoxPrivate(BackupPeriodComboBox::Selection theSelection)
        : selection(theSelection)
    {}

    BackupPeriodComboBox::Selection selection;
};


// PUBLIC

BackupPeriodComboBox::BackupPeriodComboBox(QWidget *parent, Selection selection) noexcept
    : QComboBox(parent),
      d(std::make_unique<BackupPeriodComboBoxPrivate>(selection))
{
    initUi();
}

BackupPeriodComboBox::~BackupPeriodComboBox() noexcept
{}

BackupPeriodComboBox::Selection BackupPeriodComboBox::getSelection() const noexcept
{
    return d->selection;
}

void BackupPeriodComboBox::setSelection(Selection selection) noexcept
{
    d->selection = selection;
    updateUi();
}

// PRIVATE

void BackupPeriodComboBox::initUi() noexcept
{
    insertItem(Enum::toUnderlyingType(Index::Never), tr("Never"), Const::BackupNeverSymId);
    insertItem(Enum::toUnderlyingType(Index::Monthly), tr("Once a month, when exiting Sky Dolly"), Const::BackupMonthlySymId);
    insertItem(Enum::toUnderlyingType(Index::Weekly), tr("Once a week, when exiting Sky Dolly"), Const::BackupWeeklySymId);
    insertItem(Enum::toUnderlyingType(Index::Daily), tr("Daily, when exiting Sky Dolly"), Const::BackupDailySymId);
    insertItem(Enum::toUnderlyingType(Index::Always), tr("Always, when exiting Sky Dolly"), Const::BackupAlwaysSymId);
    if (d->selection == Selection::IncludingNextTime) {
        insertItem(Enum::toUnderlyingType(Index::NextTime), tr("Next time, when exiting Sky Dolly"));
    }
}

void BackupPeriodComboBox::updateUi() noexcept
{
    if (d->selection == Selection::BackupPeriod && count() == Enum::toUnderlyingType(Index::NofIndices)) {
        removeItem(Enum::toUnderlyingType(Index::NextTime));
    } else if (d->selection == Selection::IncludingNextTime && count() != Enum::toUnderlyingType(Index::NofIndices)) {
        insertItem(Enum::toUnderlyingType(Index::NextTime), tr("Next time, when exiting Sky Dolly"));
    }
}
