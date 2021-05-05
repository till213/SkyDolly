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
#ifndef ABOUTLIBRARYDIALOG_H
#define ABOUTLIBRARYDIALOG_H

#include <memory>

#include <QDialog>

class QShowEvent;

class DatabaseService;

namespace Ui {
    class AboutLibraryDialog;
}

class AboutLibraryDialogPrivate;

class AboutLibraryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutLibraryDialog(DatabaseService &databaseService, QWidget *parent = nullptr) noexcept;
    virtual ~AboutLibraryDialog() noexcept;

protected:
    void showEvent(QShowEvent *event) noexcept override;

private:
    Q_DISABLE_COPY(AboutLibraryDialog)
    std::unique_ptr<AboutLibraryDialogPrivate> d;
    Ui::AboutLibraryDialog *ui;

    void updateUi() noexcept;
};

#endif // ABOUTLIBRARYDIALOG_H
