/**
 * Sky Dolly - The black sheep for your flight recordings
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
#ifndef LOGBOOKSETTINGSDIALOG_H
#define LOGBOOKSETTINGSDIALOG_H

#include <memory>

#include <QDialog>

class QShowEvent;

class DatabaseService;

namespace Ui {
    class LogbookSettingsDialog;
}

class LogbookSettingsDialogPrivate;

class LogbookSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LogbookSettingsDialog(DatabaseService &databaseService, QWidget *parent = nullptr) noexcept;
    virtual ~LogbookSettingsDialog() noexcept;

protected:
    virtual void showEvent(QShowEvent *event) noexcept override;

private:
    Q_DISABLE_COPY(LogbookSettingsDialog)
    std::unique_ptr<LogbookSettingsDialogPrivate> d;
    Ui::LogbookSettingsDialog *ui;

    void initUi() noexcept;
    void updateUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void on_showLogbookPathPushButton_clicked() noexcept;
    void handleAccepted() noexcept;
};

#endif // LOGBOOKSETTINGSDIALOG_H
