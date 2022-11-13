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
#ifndef BASICFLIGHTEXPORTDIALOG_H
#define BASICFLIGHTEXPORTDIALOG_H

#include <memory>
#include <cstdint>

#include <QDialog>

#include <Kernel/SampleRate.h>
#include "PluginManagerLib.h"

class Flight;
class FlightExportPluginBaseSettings;
struct BasicFlightExportDialogPrivate;

namespace Ui {
    class BasicFlightExportDialog;
}

class PLUGINMANAGER_API BasicFlightExportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BasicFlightExportDialog(const Flight &flight, const QString &fileSuffix, const QString &fileFilter, FlightExportPluginBaseSettings &pluginSettings, QWidget *parent = nullptr) noexcept;
    BasicFlightExportDialog(const BasicFlightExportDialog &rhs) = delete;
    BasicFlightExportDialog(BasicFlightExportDialog &&rhs) = delete;
    BasicFlightExportDialog &operator=(const BasicFlightExportDialog &rhs) = delete;
    BasicFlightExportDialog &operator=(BasicFlightExportDialog &&rhs) = delete;
    ~BasicFlightExportDialog() override;

    QString getSelectedFilePath() const noexcept;
    void setSelectedFilePath(const QString &filePath) noexcept;

    void setOptionWidget(QWidget *widget) noexcept;

private:
    const std::unique_ptr<Ui::BasicFlightExportDialog> ui;
    const std::unique_ptr<BasicFlightExportDialogPrivate> d;

    void initUi() noexcept;
    void initBasicUi() noexcept;
    void initOptionUi() noexcept;
    void updateDataGroupBox() noexcept;
    void frenchConnection() noexcept;
    inline bool isExportUserAircraftOnly() const noexcept;
    std::int64_t estimateNofSamplePoints() const noexcept;

private slots:
    void updateUi() noexcept;

    void onFileSelectionButtonClicked() noexcept;
    void onFilePathChanged();
    void onFormationExportChanged() noexcept;
    void onResamplingOptionChanged() noexcept;
    void onDoOpenExportedFilesChanged(bool enable) noexcept;
    void onRestoreDefaults() noexcept;
};

#endif // BASICFLIGHTEXPORTDIALOG_H
