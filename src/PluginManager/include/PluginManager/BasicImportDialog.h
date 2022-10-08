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
#ifndef BASICIMPORTDIALOG_H
#define BASICIMPORTDIALOG_H

#include <memory>

#include <QDialog>
#include <QString>

class QWidget;

#include <Model/AircraftType.h>
#include "PluginManagerLib.h"

class Flight;
class ImportPluginBaseSettings;
struct BasicImportDialogPrivate;

namespace Ui {
    class BasicImportDialog;
}

class PLUGINMANAGER_API BasicImportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BasicImportDialog(const Flight &flight, const QString &fileExtension, ImportPluginBaseSettings &pluginSettings, QWidget *parent = nullptr) noexcept;
    BasicImportDialog(const BasicImportDialog &rhs) = delete;
    BasicImportDialog(BasicImportDialog &&rhs) = delete;
    BasicImportDialog &operator=(const BasicImportDialog &rhs) = delete;
    BasicImportDialog &operator=(BasicImportDialog &&rhs) = delete;
    ~BasicImportDialog() override;

    AircraftType getSelectedAircraftType(bool *ok = nullptr) const noexcept;
    QString getSelectedPath() const noexcept;

    QString getFileFilter() const noexcept;
    void setFileFilter(const QString &fileFilter) noexcept;

    void setOptionWidget(QWidget *widget) noexcept;

private:
    const std::unique_ptr<Ui::BasicImportDialog> ui;
    const std::unique_ptr<BasicImportDialogPrivate> d;

    void initUi() noexcept;
    void initBasicUi() noexcept;
    void initOptionUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void updateUi() noexcept;

    void onFileSelectionChanged() noexcept;
    void onImportDirectoryChanged(bool enable) noexcept;
    void onAddToExistingFlightChanged(bool enable) noexcept;
    void onRestoreDefaults() noexcept;
    void onAccepted() noexcept;    
};

#endif // BASICIMPORTDIALOG_H
