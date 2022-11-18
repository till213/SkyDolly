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
#ifndef BASICLOCATIONIMPORTDIALOG_H
#define BASICLOCATIONIMPORTDIALOG_H

#include <memory>

#include <QDialog>
#include <QString>

class QWidget;

#include "PluginManagerLib.h"

class LocationImportPluginBaseSettings;
struct BasicLocationImportDialogPrivate;

namespace Ui {
    class BasicLocationImportDialog;
}

class PLUGINMANAGER_API BasicLocationImportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BasicLocationImportDialog(const QString &fileExtension, LocationImportPluginBaseSettings &pluginSettings, QWidget *parent = nullptr) noexcept;
    BasicLocationImportDialog(const BasicLocationImportDialog &rhs) = delete;
    BasicLocationImportDialog(BasicLocationImportDialog &&rhs) = delete;
    BasicLocationImportDialog &operator=(const BasicLocationImportDialog &rhs) = delete;
    BasicLocationImportDialog &operator=(BasicLocationImportDialog &&rhs) = delete;
    ~BasicLocationImportDialog() override;

    QString getSelectedPath() const noexcept;

    QString getFileFilter() const noexcept;
    void setFileFilter(const QString &fileFilter) noexcept;

    void setOptionWidget(QWidget *widget) noexcept;

private:
    const std::unique_ptr<Ui::BasicLocationImportDialog> ui;
    const std::unique_ptr<BasicLocationImportDialogPrivate> d;

    void initUi() noexcept;
    void initBasicUi() noexcept;
    void initOptionUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void updateUi() noexcept;

    void onFileSelectionChanged() noexcept;
    void onImportDirectoryChanged(bool enable) noexcept;
    void onRestoreDefaults() noexcept;
};

#endif // BASICLOCATIONIMPORTDIALOG_H
