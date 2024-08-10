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
#ifndef BASICLOCATIONEXPORTDIALOG_H
#define BASICLOCATIONEXPORTDIALOG_H

#include <memory>

#include <QDialog>

#include <Kernel/SampleRate.h>
#include "../PluginManagerLib.h"

struct Location;
class LocationExportPluginBaseSettings;
struct BasicLocationExportDialogPrivate;

namespace Ui {
    class BasicLocationExportDialog;
}

class PLUGINMANAGER_API BasicLocationExportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BasicLocationExportDialog(QString fileExtension, QString fileFilter, LocationExportPluginBaseSettings &pluginSettings, QWidget *parent = nullptr) noexcept;
    BasicLocationExportDialog(const BasicLocationExportDialog &rhs) = delete;
    BasicLocationExportDialog(BasicLocationExportDialog &&rhs) = delete;
    BasicLocationExportDialog &operator=(const BasicLocationExportDialog &rhs) = delete;
    BasicLocationExportDialog &operator=(BasicLocationExportDialog &&rhs) = delete;
    ~BasicLocationExportDialog() override;

    QString getSelectedFilePath() const noexcept;
    void setSelectedFilePath(const QString &filePath) noexcept;

    void setOptionWidget(QWidget *widget) noexcept;

    /*!
     * Returns whether the user has selected the file via the file selection dialog,
     * which typically already asks the user whether to overwrite existing files.
     *
     * Note: the assumption here is that a file selection dialog will check the
     * existence of a selected file. This is the case on Windows, macOS and Ubuntu
     * (with MATE).
     *
     * \return \c true if the user has selected the file path via the file selection dialog;
     *         \c false else
     */
    bool isFileDialogSelectedFile() const noexcept;

private:
    const std::unique_ptr<Ui::BasicLocationExportDialog> ui;
    const std::unique_ptr<BasicLocationExportDialogPrivate> d;

    void initUi() noexcept;
    void initBasicUi() noexcept;
    void initOptionUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void updateUi() noexcept;

    void onFileSelectionButtonClicked() noexcept;
    void onFilePathChanged();
    void onDoExportPresetLocationsChanged(bool enable) noexcept;
    void onDoOpenExportedFilesChanged(bool enable) noexcept;
    void onRestoreDefaults() noexcept;
};

#endif // BASICLOCATIONEXPORTDIALOG_H
