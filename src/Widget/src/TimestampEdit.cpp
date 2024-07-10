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
#include <QSize>
#include <QTimeEdit>
#include <QDateEdit>
#include <QDate>
#include <QDateTimeEdit>

#include <Kernel/Unit.h>
#include "TimestampEdit.h"
#include "ui_TimestampEdit.h"

namespace
{
    constexpr const char *TimeFormat {"hh:mm:ss"};
    constexpr const char *DateTimeFormat {"dd:MM:yyyy hh:mm:ss"};
}

struct TimestampEditPrivate
{
    QDateTime startZuluDateTime;
    QDateTime startRealWorldLocalDateTime;
    // The current timestamp [msec]
    std::int64_t timestamp {0};
    // The maximum recorded timestamp [msec]
    std::int64_t maximumTimestamp {0};
    bool minimalUiEnabled {false};
};

// PUBLIC

TimestampEdit::TimestampEdit(QWidget *parent) noexcept
    : QWidget(parent),
      ui {std::make_unique<Ui::TimestampEdit>()},
      d {std::make_unique<TimestampEditPrivate>()}

{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

TimestampEdit::~TimestampEdit() = default;

QDateTime TimestampEdit::getStartZuluDateTime() const noexcept
{
    return d->startZuluDateTime;
}

void TimestampEdit::setStartZuluDateTime(QDateTime dateTime) noexcept
{
    if (d->startZuluDateTime != dateTime) {
        d->startZuluDateTime = std::move(dateTime);
        d->startRealWorldLocalDateTime = d->startZuluDateTime.toLocalTime();
        updateUi();
    }
}

std::int64_t TimestampEdit::getTimestamp() const noexcept
{
    return d->timestamp;
}

void TimestampEdit::setTimestamp(std::int64_t timestamp) noexcept
{
    if (d->timestamp != timestamp) {
        d->timestamp = timestamp;
        updateUi();
    }
}

std::int64_t TimestampEdit::getMaximumTimestamp() const noexcept
{
    return d->maximumTimestamp;
}

void TimestampEdit::setMaximumTimestamp(std::int64_t maximumTime) noexcept
{
    if (d->maximumTimestamp != maximumTime) {
        d->maximumTimestamp = maximumTime;
        updateUi();
    }
}

bool TimestampEdit::isMinimalUiEnabled() const noexcept
{
    return d->minimalUiEnabled;
}

void TimestampEdit::setMinimalUiEnabled(bool enable) noexcept
{
    if (d->minimalUiEnabled != enable) {
        d->minimalUiEnabled = enable;
        updateUi();
    }
}

// PRIVATE

void TimestampEdit::initUi() noexcept
{
    ui->timeEdit->setDisplayFormat(::TimeFormat);
    ui->dateTimeEdit->setDisplayFormat(::DateTimeFormat);
}

void TimestampEdit::frenchConnection() noexcept
{
    connect(ui->timeEdit, &QTimeEdit::timeChanged,
            this, &TimestampEdit::onTimeEditChanged);
    connect(ui->dateTimeEdit, &QDateTimeEdit::dateTimeChanged,
            this, &TimestampEdit::onDateTimeEditChanged);
}

// PRIVATE SLOTS

void TimestampEdit::updateUi() noexcept
{
    if (d->maximumTimestamp < Unit::MillisecondsPerDay) {
        const QTime time = QTime::fromMSecsSinceStartOfDay(d->timestamp);
        ui->timeEdit->setTime(time);
        ui->timeLabel->setVisible(true);
        ui->timeEdit->setVisible(true);
        ui->dateTimeLabel->setVisible(false);
        ui->dateTimeEdit->setVisible(false);
    } else {
        const QDateTime dateTime = d->startRealWorldLocalDateTime.addMSecs(d->timestamp);
        ui->dateTimeEdit->setDateTime(dateTime);
        ui->timeLabel->setVisible(false);
        ui->timeEdit->setVisible(false);
        ui->dateTimeEdit->setVisible(true);
        if (d->minimalUiEnabled) {
            ui->dateTimeLabel->setVisible(false);
        } else {
            ui->dateTimeLabel->setVisible(true);
        }
    }
}

void TimestampEdit::onTimeEditChanged(QTime time) noexcept
{
    std::int64_t timestamp = time.hour() * Unit::MillisecondsPerHour + time.minute() * Unit::MillisecondsPerMinute + time.second() * Unit::MillisecondsPerSecond;
    emit timestampChanged(timestamp);
}

void TimestampEdit::onDateTimeEditChanged(const QDateTime &dateTime) noexcept
{

}
