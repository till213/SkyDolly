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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <cstdint>

#include <QMainWindow>

class QButtonGroup;
class QTime;
class QEvent;
class QResizeEvent;
class QCloseEvent;

#include <Module/Module.h>
#include "UserInterfaceLib.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class AboutDialog;
class SettingsDialog;
class FlightDialog;
class SimulationVariablesDialog;
class StatisticsDialog;
class MainWindowPrivate;

class USERINTERFACE_API MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &filePath = QString(), QWidget *parent = nullptr) noexcept;
    ~MainWindow() noexcept override;

    bool connectWithLogbook(const QString &filePath) noexcept;

protected:
    void resizeEvent(QResizeEvent *event) noexcept override;
    void closeEvent(QCloseEvent *event) noexcept override;

private:
    Q_DISABLE_COPY(MainWindow)
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<MainWindowPrivate> d;

    void frenchConnection() noexcept;
    void initUi() noexcept;
    void initPlugins() noexcept;
    void initModuleSelectorUi() noexcept;
    void initViewUi() noexcept;
    void initControlUi() noexcept;
    void initReplaySpeedUi() noexcept;
    void initSkyConnectPlugin() noexcept;

    /*
     * Use this method in order to access the FlightDialog instance: creates on demand
     * FlightDialog instance, connects to its signals and returns the instance.
     */
    FlightDialog &getFlightDialog() noexcept;

    /* Returns true if a flight dialog has ever been created. */
    inline bool hasFlightDialog() const noexcept;

    /*
     * Use this method in order to access the SimulationVariablesDialog instance: creates on demand
     * SimulationVariablesDialog instance, connects to its signals and returns the instance.
     */
    SimulationVariablesDialog &getSimulationVariablesDialog() noexcept;

    /* Returns true if a simulation variables dialog has ever been created. */
    inline bool hasSimulationVariablesDialog() const noexcept;

    /*
     * Use this method in order to access the StatisticsDialog instance: creates on demand
     * StatisticsDialog instance, connects to its signals and returns the instance.
     */
    StatisticsDialog &getStatisticsDialog() noexcept;

    /* Returns true if a statistics dialog has ever been created. */
    inline bool hasStatisticsDialog() const noexcept;

    void updateMinimalUi(bool enable);

    /*
     * Updates the timestamp time edit widget by setting the maximum time according
     * to the currently recorded time given by 'timestamp'.
     *
     * Also refer to updateReplayDuration.
     */
    void updateRecordingDuration(std::int64_t timestamp) noexcept;

    /*
     * Updates the position slider widget by setting the position according to
     * 'timestamp'.
     */
    void updatePositionSlider(std::int64_t timestamp) noexcept;

    void updateMinimalUiButtonTextVisibility() noexcept;
    void updateMinimalUiEssentialButtonVisibility() noexcept;

    /* Updates the replay speed group visibility. Set 'enterMinimalUi' to true
     * when switching into the minimal UI mode
     */
    void updateReplaySpeedVisibility(bool enterMinimalUi) noexcept;
    void updatePositionSliderTickInterval() noexcept;

    double getCustomSpeedFactor() const;
    
private slots:
    // Ui elements
    void on_positionSlider_sliderPressed() noexcept;
    void on_positionSlider_valueChanged(int value) noexcept;
    void on_positionSlider_sliderReleased() noexcept;
    void on_timestampTimeEdit_timeChanged(const QTime &time) noexcept;

    void updateWindowSize() noexcept;
    void handleTimestampChanged(std::int64_t timestamp) noexcept;
    void handleReplaySpeedSelected(QAction *action) noexcept;
    void handleCustomSpeedChanged() noexcept;
    void handleReplaySpeedUnitSelected(int index) noexcept;

    void updateUi() noexcept;
    void updateControlUi() noexcept;
    void updateControlIcons() noexcept;
    void updateReplaySpeedUi() noexcept;
    void onDefaultMinimalUiButtonTextVisibilityChanged(bool visible) noexcept;
    void onDefaultMinimalUiEssentialButtonVisibilityChanged(bool visible) noexcept;

    /*
     * Updates the timestamp by setting the maximum replay time according
     * to the total duration of the current flight.
     *
     * Also refer to updateRecordingDuration.
     */
    void updateReplayDuration() noexcept;

    void updateFileMenu() noexcept;
    void updateModuleActions() noexcept;
    void updateWindowMenu() noexcept;
    void updateMainWindow() noexcept;

    // Modules
    void handleModuleActivated(const QString title, Module::Module moduleId) noexcept;

    // File menu
    void on_newLogbookAction_triggered() noexcept;
    void on_openLogbookAction_triggered() noexcept;
    void on_optimiseLogbookAction_triggered() noexcept;
    void on_showSettingsAction_triggered() noexcept;
    void on_showLogbookSettingsAction_triggered() noexcept;
    void on_quitAction_triggered() noexcept;

    // View menu
    void onShowModulesChanged(bool enable) noexcept;
    void onShowReplaySpeedChanged(bool enable) noexcept;

    // Window menu
    void toggleFlightDialog(bool enable) noexcept;
    void onFlightDialogDeleted() noexcept;
    void toggleSimulationVariablesDialog(bool enable) noexcept;
    void onSimulationVariablesDialogDeleted() noexcept;
    void toggleStatisticsDialog(bool enable) noexcept;
    void onStatisticsDialogDeleted() noexcept;

    void on_stayOnTopAction_triggered(bool enable) noexcept;
    void on_showMinimalAction_toggled(bool enable) noexcept;

    // Help menu
    void onAboutActionTriggered() noexcept;
    void onAboutQtActionTriggered() noexcept;
    void onOnlineManualActionTriggered() const noexcept;

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
    void toggleLoopReplay(bool checked) noexcept;

    // Service
    void onFlightRestored() noexcept;
    void onLogbookConnectionChanged(bool connected) noexcept;

    // Import / export
    void onImport(QAction *action) noexcept;
    void onExport(QAction *action) noexcept;

    // Settings
    void handleReplayLoopChanged() noexcept;
};
#endif // MAINWINDOW_H
