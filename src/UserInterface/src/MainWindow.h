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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QMainWindow>

#include "../../SkyConnect/src/SkyConnectIntf.h"
#include "../../SkyConnect/src/Connect.h"
#include "UserInterfaceLib.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QButtonGroup;
class QTime;

class AboutDialog;
class SettingsDialog;
class SimulationVariablesDialog;
class MainWindowPrivate;

class USERINTERFACE_API MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr) noexcept;
    virtual ~MainWindow() noexcept;

private:
    Q_DISABLE_COPY(MainWindow)
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<MainWindowPrivate> d;

    void frenchConnection() noexcept;
    void initUi() noexcept;
    void initControlUi() noexcept;

    bool connectWithDb() noexcept;

private slots:
    void on_positionSlider_sliderPressed() noexcept;
    void on_positionSlider_valueChanged(int value) noexcept;
    void on_positionSlider_sliderReleased() noexcept;
    void on_timestampTimeEdit_timeChanged(const QTime &time) noexcept;
    void on_customReplaySpeedLineEdit_editingFinished() noexcept;

    void updateUi() noexcept;
    void updateControlUi() noexcept;
    void updateTimestamp() noexcept;
    void updateFileMenu() noexcept;
    void updateWindowMenu() noexcept;
    void updateMainWindow() noexcept;

    // File menu
    void on_newLibraryAction_triggered() noexcept;
    void on_openLibraryAction_triggered() noexcept;
    void on_openScenarioAction_triggered() noexcept;
    void on_importCSVAction_triggered() noexcept;
    void on_exportCSVAction_triggered() noexcept;
    void on_showSettingsAction_triggered() noexcept;
    void on_quitAction_triggered() noexcept;
    // Window menu
    void on_showScenarioAction_triggered(bool enabled) noexcept;
    void on_showSimulationVariablesAction_triggered(bool enabled) noexcept;
    void on_showStatisticsAction_triggered(bool enabled) noexcept;
    void on_stayOnTopAction_triggered(bool enabled) noexcept;
    // About menu
    void on_aboutLibraryAction_triggered() noexcept;
    void on_aboutAction_triggered() noexcept;
    void on_aboutQtAction_triggered() noexcept;

    void handleTimestampChanged(qint64 timestamp) noexcept;
    void handleReplaySpeedSelected(int selection) noexcept;

    // Replay
    void toggleRecord(bool checked) noexcept;
    void togglePause(bool checked) noexcept;
    void togglePlay(bool checked) noexcept;
    void stop() noexcept;

    // Transport
    void skipToBegin() noexcept;
    void skipBackward() noexcept;
    void skipForward() noexcept;
    void skipToEnd() noexcept;
};
#endif // MAINWINDOW_H
