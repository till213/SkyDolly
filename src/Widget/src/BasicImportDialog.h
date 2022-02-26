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
#ifndef BASICIMPORTDIALOG_H
#define BASICIMPORTDIALOG_H

#include <memory>

#include <QDialog>
#include <QString>

class QWidget;
class QAbstractButton;

#include "WidgetLib.h"

struct AircraftType;
class BasicImportDialogPrivate;

namespace Ui {
    class BasicImportDialog;
}

class WIDGET_API BasicImportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BasicImportDialog(const QString &fileExtension, QWidget *parent = nullptr);
    virtual ~BasicImportDialog();

    QString getSelectedFilePath() const noexcept;
    bool getSelectedAircraftType(AircraftType &aircraftType) const noexcept;
    bool isAddToFlightEnabled() const noexcept;

    QString getFileFilter() const noexcept;
    void setFileFilter(const QString &extension) noexcept;

    void setOptionWidget(QWidget *widget) noexcept;

signals:
    void restoreDefaultOptions();

private:
    Ui::BasicImportDialog *ui;
    std::unique_ptr<BasicImportDialogPrivate> d;

    void frenchConnection() noexcept;
    void initUi() noexcept;
    void initBasicUi() noexcept;
    void initOptionUi() noexcept;

private slots:
    void onFileSelectionPushButtonClicked() noexcept;
    void onDefaultButtonClicked(QAbstractButton *button) noexcept;
    void onAccepted() noexcept;
    void updateUi() noexcept;
};

#endif // BASICIMPORTDIALOG_H
