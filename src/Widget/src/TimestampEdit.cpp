/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
    QDateTime startDateTime;
    // The current timestamp [msec]
    std::int64_t timestamp {0};
    // The maximum recorded timestamp [msec]
    std::int64_t endTimestamp {0};
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

QDateTime TimestampEdit::getStartDateTime() const noexcept
{
    return d->startDateTime;
}

void TimestampEdit::setStartDateTime(QDateTime dateTime) noexcept
{
    if (d->startDateTime != dateTime) {
        d->startDateTime = std::move(dateTime);
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
        updateTimestamp();
    }
}

std::int64_t TimestampEdit::getEndTimestamp() const noexcept
{
    return d->endTimestamp;
}

void TimestampEdit::setEndTimestamp(std::int64_t endTimestamp) noexcept
{
    if (d->endTimestamp != endTimestamp) {
        d->endTimestamp = endTimestamp;
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

void TimestampEdit::updateTimestamp() noexcept
{
    if (d->endTimestamp < Unit::MillisecondsPerDay) {
        const auto time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(d->timestamp));
        ui->timeEdit->setTime(time);
    } else {
        const auto dateTime = d->startDateTime.addMSecs(d->timestamp);
        ui->dateTimeEdit->setDateTime(dateTime);
    }
}

// PRIVATE SLOTS

void TimestampEdit::updateUi() noexcept
{
    if (d->endTimestamp < Unit::MillisecondsPerDay) {
        ui->timeLabel->setVisible(true);
        ui->timeEdit->setVisible(true);
        ui->dateTimeLabel->setVisible(false);
        ui->dateTimeEdit->setVisible(false);
        const QTime time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(d->endTimestamp));
        ui->timeEdit->setMaximumTime(time);
    } else {        
        ui->timeLabel->setVisible(false);
        ui->timeEdit->setVisible(false);
        ui->dateTimeEdit->setVisible(true);
        if (d->minimalUiEnabled) {
            ui->dateTimeLabel->setVisible(false);
        } else {
            ui->dateTimeLabel->setVisible(true);
        }
        const auto startDateTime = getStartDateTime();
        const auto endDateTime = startDateTime.addMSecs(d->endTimestamp);
        ui->dateTimeEdit->setMinimumDateTime(startDateTime);
        ui->dateTimeEdit->setMaximumDateTime(endDateTime);
    }
    updateTimestamp();
}

void TimestampEdit::onTimeEditChanged(QTime time) noexcept
{
    const auto timestamp = time.hour() * Unit::MillisecondsPerHour + time.minute() * Unit::MillisecondsPerMinute + time.second() * Unit::MillisecondsPerSecond;
    emit timestampChanged(timestamp);
}

void TimestampEdit::onDateTimeEditChanged(const QDateTime &dateTime) noexcept
{
    const auto seconds = getStartDateTime().secsTo(dateTime);
    const auto timestamp = seconds * Unit::MillisecondsPerSecond;
    emit timestampChanged(timestamp);
}
