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

class QEvent;
class QResizeEvent;

#include "../../SkyConnect/src/SkyConnectIntf.h"
#include "../../Module/src/Module.h"
#include "../../SkyConnect/src/Connect.h"
#include "UserInterfaceLib.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QButtonGroup;
class QTime;
class QResizeEvent;
class QCloseEvent;

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

protected:
    virtual void resizeEvent(QResizeEvent *event) noexcept override;
    virtual void closeEvent(QCloseEvent *event) noexcept override;

private:
    Q_DISABLE_COPY(MainWindow)
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<MainWindowPrivate> d;

    void frenchConnection() noexcept;
    void initUi() noexcept;
    void initPlugins() noexcept;
    void initModuleSelectorUi() noexcept;
    void initControlUi() noexcept;
    void initReplaySpeedUi() noexcept;
    void initSkyConnectPlugin() noexcept;

    void updateMinimalUi(bool enable);    
    double getCustomSpeedFactor() const;
    
private slots:

    // Ui elements
    void on_positionSlider_sliderPressed() noexcept;
    void on_positionSlider_valueChanged(int value) noexcept;
    void on_positionSlider_sliderReleased() noexcept;
    void on_timestampTimeEdit_timeChanged(const QTime &time) noexcept;

    void updateWindowSize() noexcept;
    void handleTimestampChanged(qint64 timestamp) noexcept;
    void handleReplaySpeedSelected(QAction *action) noexcept;
    void handleCustomSpeedChanged() noexcept;
    void handleReplaySpeedUnitSelected(int index) noexcept;

    void updateUi() noexcept;
    void updateControlUi() noexcept;
    void updateControlIcons() noexcept;
    void updateReplaySpeedUi() noexcept;
    void updateTimestamp() noexcept;
    void updateFileMenu() noexcept;
    void updateWindowMenu() noexcept;
    void updateMainWindow() noexcept;

    // Modules
    void handleModuleActivated(const QString title, Module::Module moduleId) noexcept;
    void on_moduleSelectorVisibleCheckBox_clicked(bool enabled) noexcept;

    // File menu
    void on_newLogbookAction_triggered() noexcept;
    void on_openLogbookAction_triggered() noexcept;
    void on_backupLogbookAction_triggered() noexcept;
    void on_optimiseLogbookAction_triggered() noexcept;
    void on_showSettingsAction_triggered() noexcept;
    void on_quitAction_triggered() noexcept;
    // Window menu
    void on_showFlightAction_triggered(bool enabled) noexcept;
    void on_showSimulationVariablesAction_triggered(bool enabled) noexcept;
    void on_showStatisticsAction_triggered(bool enabled) noexcept;
    void on_stayOnTopAction_triggered(bool enabled) noexcept;
    void on_showMinimalAction_triggered(bool enabled) noexcept;
    // About menu
    void on_aboutLogbookAction_triggered() noexcept;
    void on_aboutAction_triggered() noexcept;
    void on_aboutQtAction_triggered() noexcept;

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

    // Service
    void handleFlightRestored() noexcept;
    void handleLogbookConnectionChanged(bool connected) noexcept;

    // Import / export
    void handleImport(QAction *action) noexcept;
    void handleExport(QAction *action) noexcept;
};
#endif // MAINWINDOW_H
