/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef LOGBOOKSETTINGSDIALOG_H
#define LOGBOOKSETTINGSDIALOG_H

#include <memory>

#include <QDialog>

class QWidget;
class QShowEvent;

namespace Ui {
    class LogbookSettingsDialog;
}

struct LogbookSettingsDialogPrivate;

class LogbookSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LogbookSettingsDialog(QWidget *parent = nullptr) noexcept;
    LogbookSettingsDialog(const LogbookSettingsDialog &rhs) = delete;
    LogbookSettingsDialog(LogbookSettingsDialog &&rhs) = delete;
    LogbookSettingsDialog &operator=(const LogbookSettingsDialog &rhs) = delete;
    LogbookSettingsDialog &operator=(LogbookSettingsDialog &&rhs) = delete;
    ~LogbookSettingsDialog() override;

public slots:
    void accept() noexcept override;

protected:
    void showEvent(QShowEvent *event) noexcept override;

private:
    const std::unique_ptr<Ui::LogbookSettingsDialog> ui;
    const std::unique_ptr<LogbookSettingsDialogPrivate> d;

    void initUi() noexcept;
    void updateUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void openLogbookDirectory() noexcept;
};

#endif // LOGBOOKSETTINGSDIALOG_H
