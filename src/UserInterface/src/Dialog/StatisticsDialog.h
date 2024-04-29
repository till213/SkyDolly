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
#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <memory>
#include <cstdint>

#include <QDialog>

class QShowEvent;
class QHideEvent;

struct StatisticsDialogPrivate;

namespace Ui {
    class StatisticsDialog;
}

class StatisticsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StatisticsDialog(QWidget *parent = nullptr) noexcept;
    StatisticsDialog(const StatisticsDialog &rhs) = delete;
    StatisticsDialog(StatisticsDialog &&rhs) = delete;
    StatisticsDialog &operator=(const StatisticsDialog &rhs) = delete;
    StatisticsDialog &operator=(StatisticsDialog &&rhs) = delete;
    ~StatisticsDialog() override;

signals:
    void visibilityChanged(bool visible);

protected:
    void showEvent(QShowEvent *event) noexcept override;
    void hideEvent(QHideEvent *event) noexcept override;

private:
    const std::unique_ptr<StatisticsDialogPrivate> d;
    const std::unique_ptr<Ui::StatisticsDialog> ui;

    void initUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void updateUi() noexcept;
    void updateRecordingSampleRate() noexcept;
    void updateRecordUi(std::int64_t timestamp) noexcept;
};

#endif // STATISTICSDIALOG_H
