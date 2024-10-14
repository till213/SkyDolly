/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef BASICFLIGHTIMPORTDIALOG_H
#define BASICFLIGHTIMPORTDIALOG_H

#include <memory>

#include <QDialog>
#include <QString>

class QWidget;

#include <Model/AircraftType.h>
#include "../PluginManagerLib.h"

class Flight;
class FlightImportPluginBaseSettings;
struct BasicFlightImportDialogPrivate;

namespace Ui {
    class BasicFlightImportDialog;
}

class PLUGINMANAGER_API BasicFlightImportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BasicFlightImportDialog(const Flight &flight, const QString &fileFilter, FlightImportPluginBaseSettings &pluginSettings, QWidget *parent = nullptr) noexcept;
    BasicFlightImportDialog(const BasicFlightImportDialog &rhs) = delete;
    BasicFlightImportDialog(BasicFlightImportDialog &&rhs) = delete;
    BasicFlightImportDialog &operator=(const BasicFlightImportDialog &rhs) = delete;
    BasicFlightImportDialog &operator=(BasicFlightImportDialog &&rhs) = delete;
    ~BasicFlightImportDialog() override;

    AircraftType getSelectedAircraftType(bool *ok = nullptr) const noexcept;
    QString getSelectedPath() const noexcept;

    QString getFileFilter() const noexcept;
    void setFileFilter(const QString &fileFilter) noexcept;

    void setOptionWidget(QWidget *widget) noexcept;

private:
    const std::unique_ptr<Ui::BasicFlightImportDialog> ui;
    const std::unique_ptr<BasicFlightImportDialogPrivate> d;

    void initUi() noexcept;
    void initBasicUi() noexcept;
    void initOptionUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void updateUi() noexcept;

    void onFileSelectionChanged() noexcept;
    void onImportDirectoryChanged(bool enable) noexcept;
    void onAircraftImportModeChanged() noexcept;
    void onTimeOffsetSyncChanged() noexcept;
    void onRestoreDefaults() noexcept;
    void onAccepted() noexcept;    
};

#endif // BASICFLIGHTIMPORTDIALOG_H
