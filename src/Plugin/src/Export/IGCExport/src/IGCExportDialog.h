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
#ifndef IGCEXPORTDIALOG_H
#define IGCEXPORTDIALOG_H

#include <memory>
#include <utility>
#include <cstdint>

#include <QDialog>

namespace Ui {
    class IGCExportDialog;
}

struct IGCExportSettings;
class IGCExportDialogPrivate;

class IGCExportDialog : public QDialog
{
    Q_OBJECT
public:
    static inline const QString FileSuffix {QStringLiteral("igc")};

    explicit IGCExportDialog(IGCExportSettings &exportSettings, QWidget *parent = nullptr) noexcept;
    virtual ~IGCExportDialog() noexcept;

    QString getSelectedFilePath() const noexcept;
    bool doOpenExportedFile() const noexcept;

private:
    Ui::IGCExportDialog *ui;
    std::unique_ptr<IGCExportDialogPrivate> d;

    void initUi() noexcept;
    void updateInfoUi() noexcept;
    void updateFlightUi() noexcept;
    void frenchConnection() noexcept;

    std::int64_t estimateNofSamplePoints() noexcept;

private slots:
    void updateUi() noexcept;

    void restoreDefaults() noexcept;

    void on_fileSelectionPushButton_clicked() noexcept;
    void on_resamplingComboBox_activated(int index) noexcept;
    void on_pilotNameLineEdit_editingFinished() noexcept;
    void on_coPilotNameLineEdit_editingFinished() noexcept;
};

#endif // IGCEXPORTDIALOG_H
