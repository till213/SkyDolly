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
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <memory>

#include <QDialog>

class QMousePressEvent;

struct AboutDialogPrivate;

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr) noexcept;
    AboutDialog(const AboutDialog &rhs) = delete;
    AboutDialog(AboutDialog &&rhs) = delete;
    AboutDialog &operator=(const AboutDialog &rhs) = delete;
    AboutDialog &operator=(AboutDialog &&rhs) = delete;
    ~AboutDialog() override;

protected:
    void mousePressEvent(QMouseEvent *event) noexcept override;
    void mouseReleaseEvent(QMouseEvent *event) noexcept override;

private:
    std::unique_ptr<AboutDialogPrivate> d;
    const std::unique_ptr<Ui::AboutDialog> ui;

    void initUi() noexcept;
    void updateUi() noexcept;
    void frenchConnection() noexcept;
    QString getVersionInfo() const noexcept;

private slots:
    void showAboutQtDialog() noexcept;
    void restoreVersionInfo() noexcept;
};

#endif // ABOUTDIALOG_H
