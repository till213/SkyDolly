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
#ifndef KMLEXPORTDIALOG_H
#define KMLEXPORTDIALOG_H

#include <QDialog>

namespace Ui {
    class KMLExportDialog;
}

class KMLExportDialogPrivate;

class KMLExportDialog : public QDialog
{
    Q_OBJECT
public:
    /*!
     * Resampling period [millisecons]
     */
    enum class ResamplingPeriod {
        Original = 0,
        TenHz = 100,
        FiveHz = 200,
        TwoHz = 500,
        OneHz = 1000,
        AFifthHz = 5000,
        ATenthHz = 10000
    };
    static constexpr char FileSuffix[] = "kml";

    explicit KMLExportDialog(QWidget *parent = nullptr) noexcept;
    virtual ~KMLExportDialog() noexcept;

    QString getSelectedFilePath() const noexcept;
    ResamplingPeriod getSelectedResamplingPeriod() const noexcept;
    bool doOpenExportedFile() const noexcept;

private:
    Ui::KMLExportDialog *ui;
    std::unique_ptr<KMLExportDialogPrivate> d;

    void initUi() noexcept;
    void updateInfoUi() noexcept;
    void frenchConnection() noexcept;

    qint64 estimateNofSamplePoints() noexcept;

private slots:
    void updateUi() noexcept;

    void on_fileSelectionPushButton_clicked() noexcept;
    void on_resamplingComboBox_activated(int index) noexcept;
};

#endif // KMLEXPORTDIALOG_H