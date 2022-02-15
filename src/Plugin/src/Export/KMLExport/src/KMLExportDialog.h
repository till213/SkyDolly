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
#ifndef KMLEXPORTDIALOG_H
#define KMLEXPORTDIALOG_H

#include <memory>
#include <utility>
#include <cstdint>

#include <QDialog>

#include "KMLStyleExport.h"

namespace Ui {
    class KMLExportDialog;
}

struct KMLExportSettings;
class KMLExportDialogPrivate;

class KMLExportDialog : public QDialog
{
    Q_OBJECT
public:
    static inline const QString FileSuffix {QStringLiteral("kml")};

    explicit KMLExportDialog(KMLExportSettings &exportSettings, QWidget *parent = nullptr) noexcept;
    virtual ~KMLExportDialog() noexcept;

    QString getSelectedFilePath() const noexcept;
    bool doOpenExportedFile() const noexcept;

private:
    Ui::KMLExportDialog *ui;
    std::unique_ptr<KMLExportDialogPrivate> d;

    void initUi() noexcept;
    void initColorUi() noexcept;
    void updateInfoUi() noexcept;
    void updateColorUi() noexcept;
    void frenchConnection() noexcept;

    std::int64_t estimateNofSamplePoints() noexcept;

private slots:
    void updateUi() noexcept;

    void selectColor(int id) noexcept;
    void restoreDefaults() noexcept;

    void on_fileSelectionPushButton_clicked() noexcept;
    void on_resamplingComboBox_activated(int index) noexcept;
    void on_colorStyleComboBox_activated(int index) noexcept;
};

#endif // KMLEXPORTDIALOG_H
