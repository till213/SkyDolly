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
#ifndef TIMESTAMPEDIT_H
#define TIMESTAMPEDIT_H

#include <memory>
#include <cstdint>

#include <QWidget>
#include <QTime>
#include <QDateTime>

#include "WidgetLib.h"

struct TimestampEditPrivate;

namespace Ui {
    class TimestampEdit;
}

/*!
 * Dynamically shows a simple QTimeEdit, an additional QDateEdit or a combined QDateTimeEdit,
 * depending on the maximum time and minimal user interface mode.
 */
class WIDGET_API TimestampEdit : public QWidget
{
    Q_OBJECT
public:
    explicit TimestampEdit(QWidget *parent = nullptr) noexcept;
    TimestampEdit(const TimestampEdit &rhs) = delete;
    TimestampEdit(TimestampEdit &&rhs) = delete;
    TimestampEdit &operator=(const TimestampEdit &rhs) = delete;
    TimestampEdit &operator=(TimestampEdit &&rhs) = delete;
    ~TimestampEdit() override;

    /*!
     * Returns the start date and time.
     *
     * \return the start date and time
     */
    QDateTime getStartDateTime() const noexcept;

    /*!
     * Sets the start \p dateTime. The start date and time is used
     * to show absoute time (including date, if the duration is longer
     * than a day).
     *
     * \param dateTime
     *        the start date and time
     */
    void setStartDateTime(QDateTime dateTime) noexcept;

    /*!
     * Returns the current timestamp.
     *
     * \return the current timestamp [msec|
     */
    std::int64_t getTimestamp() const noexcept;

    /*!
     * Sets the current timestamp to \p timestamp.
     *
     * \param timestamp
     *        the current timestamp [msec]
     */
    void setTimestamp(std::int64_t timestamp) noexcept;

    /*!
     * Returns the end (maximum recorded) timestamp.
     *
     * \return the end timestamp [msec|
     */
    std::int64_t getEndTimestamp() const noexcept;

    /*!
     * Sets the end (maximum recorded) timestamp to \p endTimestamp.
     *
     * \param end
     *        the end timestamp [msec]
     */
    void setEndTimestamp(std::int64_t endTimestamp) noexcept;

    /*!
     * Returns whether the minimal user interface is enabled.
     *
     * \return \c true if the minimal user interface is enabled; \c false else
     */
    bool isMinimalUiEnabled() const noexcept;

    /*!
     * Sets the minimal user interface mode to \p enable.
     *
     * \param enable
     *        set to \c true in order to enable the minimal user interface;
     *        \c false in order to enable normal user interface mode
     */
    void setMinimalUiEnabled(bool enable) noexcept;

signals:
    /*!
     * Emitted whenever the current timestamp has changed
     *
     * \param time
     *        the current timestamp [msec]
     */
    void timestampChanged(std::int64_t time);

private:
    const std::unique_ptr<Ui::TimestampEdit> ui;
    const std::unique_ptr<TimestampEditPrivate> d;

    void initUi() noexcept;
    void frenchConnection() noexcept;
    void updateTimestamp() noexcept;

private slots:
    void updateUi() noexcept;

    void onTimeEditChanged(QTime time) noexcept;
    void onDateTimeEditChanged(const QDateTime &dateTime) noexcept;
};

#endif // TIMESTAMPEDIT_H
