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
#ifndef DYNAMICDATETIMEEDIT_H
#define DYNAMICDATETIMEEDIT_H

#include <memory>
#include <cstdint>

#include <QWidget>
#include <QTime>

class QDateTime;

#include "WidgetLib.h"

struct DynamicDateTimeEditPrivate;

namespace Ui {
    class DynamicDateTimeEdit;
}

/*!
 * Dynamically shows a simple QTimeEdit, an additional QDateEdit or a combined QDateTimeEdit,
 * depending on the maximum time and minimal user interface mode.
 */
class WIDGET_API DynamicDateTimeEdit : public QWidget
{
    Q_OBJECT
public:
    explicit DynamicDateTimeEdit(QWidget *parent = nullptr) noexcept;
    DynamicDateTimeEdit(const DynamicDateTimeEdit &rhs) = delete;
    DynamicDateTimeEdit(DynamicDateTimeEdit &&rhs) = delete;
    DynamicDateTimeEdit &operator=(const DynamicDateTimeEdit &rhs) = delete;
    DynamicDateTimeEdit &operator=(DynamicDateTimeEdit &&rhs) = delete;
    ~DynamicDateTimeEdit() override;

    /*!
     * Returns the current time.
     *
     * \return the current time [msec|
     */
    std::int64_t getTimeMSec() const noexcept;

    /*!
     * Sets the current time.
     *
     * \param time
     *        the current time [msec]
     */
    void setTimeMSec(std::int64_t time) noexcept;

    /*!
     * Returns the maximum recorded time.
     *
     * \return the maximum recorded time [msec|
     */
    std::int64_t getMaximumTimeMSec() const noexcept;

    /*!
     * Sets the maximum recorded time.
     *
     * \param maximumTime
     *        the maximum recorded time [msec]
     */
    void setMaximumTimeMSec(std::int64_t maximumTime) noexcept;

    /*!
     * Returns whether the minimal user interface is enabled.
     *
     * \return \c true if the minimal user interface is enabled; \c false else
     */
    bool isMinimalUiEnabled() const noexcept;

    /*!
     * Sets the minimal user interface mode to \c enable.
     *
     * \param enable
     *        set to \c true in order to enable the minimal user interface;
     *        \c false in order to enable normal user interface mode
     */
    void setMinimalUiEnabled(bool enable) noexcept;

signals:
    /*!
     * Emitted whenever the current time has changed
     *
     * \param time
     *        the current time [msec]
     */
    void timeChanged(std::int64_t time);

private:
    const std::unique_ptr<Ui::DynamicDateTimeEdit> ui;
    const std::unique_ptr<DynamicDateTimeEditPrivate> d;

    void initUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void updateUi() noexcept;

    void onTimeEditChanged(QTime time) noexcept;
};

#endif // DYNAMICDATETIMEEDIT_H
