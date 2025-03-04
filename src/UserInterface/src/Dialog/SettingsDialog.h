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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <memory>

#include <QDialog>
#include <QKeySequence>

namespace Ui {
    class SettingsDialog;
}

struct SettingsDialogPrivate;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr) noexcept;
    SettingsDialog(const SettingsDialog &rhs) = delete;
    SettingsDialog(SettingsDialog &&rhs) = delete;
    SettingsDialog &operator=(const SettingsDialog &rhs) = delete;
    SettingsDialog &operator=(SettingsDialog &&rhs) = delete;
    ~SettingsDialog() override;

protected:
    void showEvent(QShowEvent *event) noexcept override;
    void hideEvent(QHideEvent *event) noexcept override;

private:
    const std::unique_ptr<Ui::SettingsDialog> ui;
    const std::unique_ptr<SettingsDialogPrivate> d;

    void initUi() noexcept;
    void frenchConnection() noexcept;
    void updateReplayTab() noexcept;
    void updateFlightSimulatorTab() noexcept;
    void updateUserInterfaceTab() noexcept;

private slots:
    void updateUi() noexcept;
    void updateConnectionStatus() const noexcept;
    void onFlightSimulatorConnectionSelectionChanged() const noexcept;
    void onStyleChanged() noexcept;
    void onSkyConnectPluginChanged() noexcept;
    void onAccepted() noexcept;
    void onTabChanged(int index) noexcept;

    void initFlightSimulatorOptionWidget() noexcept;
};

#endif // SETTINGSDIALOG_H
