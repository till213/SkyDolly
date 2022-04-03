/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#ifndef BASICEXPORTDIALOG_H
#define BASICEXPORTDIALOG_H

#include <memory>
#include <cstdint>

#include <QDialog>

#include "../../Kernel/src/SampleRate.h"
#include "PluginLib.h"

class ExportPluginBaseSettings;
class BasicExportDialogPrivate;

namespace Ui {
    class BasicExportDialog;
}

class PLUGIN_API BasicExportDialog : public QDialog
{
    Q_OBJECT
public:

    explicit BasicExportDialog(const QString &fileExtension, const QString &fileFilter, ExportPluginBaseSettings &settings, QWidget *parent = nullptr) noexcept;
    virtual ~BasicExportDialog() noexcept;

    QString getSelectedFilePath() const noexcept;
    void setSelectedFilePath(const QString &filePath) noexcept;

    void setOptionWidget(QWidget *widget) noexcept;

signals:
    void restoreDefaultOptions();

private:
    std::unique_ptr<Ui::BasicExportDialog> ui;
    std::unique_ptr<BasicExportDialogPrivate> d;

    void initUi() noexcept;
    void initBasicUi() noexcept;
    void initOptionUi() noexcept;
    void updateInfoUi() noexcept;
    void frenchConnection() noexcept;

    std::int64_t estimateNofSamplePoints() noexcept;

private slots:
    void updateUi() noexcept;

    void onFileSelectionButtonClicked() noexcept;
    void onFilePathChanged();
    void onResamplingOptionChanged(int index) noexcept;
    void onDoOpenExportedFileChanged(bool enable) noexcept;
    void onRestoreSettings() noexcept;
};

#endif // BASICEXPORTDIALOG_H
