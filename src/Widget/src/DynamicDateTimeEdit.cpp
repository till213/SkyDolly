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
#include <memory>

#include <QWidget>

#include "DynamicDateTimeEdit.h"
#include "ui_DynamicDateTimeEdit.h"

namespace
{
    constexpr const char *TimestampFormat {"hh:mm:ss"};
    constexpr std::int64_t MilliSecondsPerSecond {1000};
    constexpr std::int64_t MilliSecondsPerMinute {60 * ::MilliSecondsPerSecond};
    constexpr std::int64_t MilliSecondsPerHour {60 * ::MilliSecondsPerMinute};
    constexpr std::int64_t MilliSecondsPerDay {24 * ::MilliSecondsPerHour};
}

struct DynamicDateTimeEditPrivate
{
    // The currenttime [msec]
    std::int64_t time {0};
    // The maximum recorded time [msec]
    std::int64_t maximumTime {0};
    bool minimalUiEnabled {false};
};

// PUBLIC

DynamicDateTimeEdit::DynamicDateTimeEdit(QWidget *parent) noexcept
    : QWidget(parent),
      ui {std::make_unique<Ui::DynamicDateTimeEdit>()},
      d {std::make_unique<DynamicDateTimeEditPrivate>()}

{
    ui->setupUi(this);
    initUi();
}

DynamicDateTimeEdit::~DynamicDateTimeEdit() = default;

std::int64_t DynamicDateTimeEdit::getTimeMSec() const noexcept
{
    return d->time;
}

void DynamicDateTimeEdit::setTimeMSec(std::int64_t time) noexcept
{
    if (d->time != time) {
        d->time = time;
        updateUi();
    }
}

std::int64_t DynamicDateTimeEdit::getMaximumTimeMSec() const noexcept
{
    return d->maximumTime;
}

void DynamicDateTimeEdit::setMaximumTimeMSec(std::int64_t maximumTime) noexcept
{
    if (d->maximumTime != maximumTime) {
        d->maximumTime = maximumTime;
        updateUi();
    }
}

bool DynamicDateTimeEdit::isMinimalUiEnabled() const noexcept
{
    return d->minimalUiEnabled;
}

void DynamicDateTimeEdit::setMinimalUiEnabled(bool enable) noexcept
{
    if (d->minimalUiEnabled != enable) {
        d->minimalUiEnabled = enable;
        updateUi();
    }
}

// PRIVATE

void DynamicDateTimeEdit::initUi() noexcept
{
    // ui->timeEdit->setDisplayFormat(::TimestampFormat);
}

// PRIVATE SLOTS

void DynamicDateTimeEdit::updateUi() noexcept
{
    //QTime time = QTime::fromMSecsSinceStartOfDay(timestamp);
    if (d->maximumTime < ::MilliSecondsPerDay) {
        // ui->timeLabel->setVisible(true);
        // ui->timeEdit->setVisible(true);
        // ui->dateLabel->setVisible(false);
        // ui->dateEdit->setVisible(false);
        // ui->dateTimeEdit->setVisible(false);
    } else {
        if (d->minimalUiEnabled) {
            // ui->timeLabel->setVisible(false);
            // ui->timeEdit->setVisible(false);
            // ui->dateLabel->setVisible(false);
            // ui->dateEdit->setVisible(false);
            // ui->dateTimeEdit->setVisible(true);
        } else {
            // ui->timeLabel->setVisible(true);
            // ui->timeEdit->setVisible(true);
            // ui->dateLabel->setVisible(true);
            // ui->dateEdit->setVisible(true);
            // ui->dateTimeEdit->setVisible(false);
        }
    }
}

void DynamicDateTimeEdit::onTimeEditChanged(QTime time) noexcept
{
    // TODO Take date edit into account
    std::int64_t timestamp = time.hour() * ::MilliSecondsPerHour + time.minute() * ::MilliSecondsPerMinute + time.second() * ::MilliSecondsPerSecond;
    emit timeChanged(timestamp);
}
