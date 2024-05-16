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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <cstdint>

#include <QMainWindow>
#include <QUuid>

class QButtonGroup;
class QTime;
class QEvent;
class QResizeEvent;
class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;

#include "UserInterfaceLib.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include <PluginManager/Connect/SkyConnectIntf.h>

class AboutDialog;
class SettingsDialog;
class FlightDialog;
class SimulationVariablesDialog;
class StatisticsDialog;
class SecurityToken;
struct MainWindowPrivate;

class USERINTERFACE_API MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &filePath = QString(), QWidget *parent = nullptr) noexcept;
    MainWindow(const MainWindow &rhs) = delete;
    MainWindow(MainWindow &&rhs) = delete;
    MainWindow &operator=(const MainWindow &rhs) = delete;
    MainWindow &operator=(MainWindow &&rhs) = delete;
    ~MainWindow() override;

    bool connectWithLogbook(const QString &filePath) noexcept;

protected:
    void resizeEvent(QResizeEvent *event) noexcept override;
    void closeEvent(QCloseEvent *event) noexcept override;

    void dragEnterEvent(QDragEnterEvent *event) noexcept override;
    void dropEvent(QDropEvent *event) noexcept override;

private:
    std::unique_ptr<Ui::MainWindow> ui;
    const std::unique_ptr<MainWindowPrivate> d;

    void frenchConnection() noexcept;
    void initUi() noexcept;
    void initPlugins() noexcept;
    void initModuleSelectorUi() noexcept;
    void initViewUi() noexcept;
    void initControlUi() noexcept;
    void initReplaySpeedUi() noexcept;
    void createTrayIcon() noexcept;
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
    bool isMinimalUiEnabled() const noexcept;
    void updateReplaySpeedUi() noexcept;

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

    void seek(int value, SkyConnectIntf::SeekMode seekMode) const noexcept;
    
private slots:
    // Ui elements
    void onPositionSliderPressed() noexcept;
    void onPositionSliderValueChanged(int value) noexcept;
    void onPositionSliderReleased() noexcept;
    void onTimeEditChanged(const QTime &time) noexcept;

    void updateWindowSize() noexcept;
    void onTimestampChanged(std::int64_t timestamp) noexcept;
    void onReplaySpeedSelected(QAction *action) noexcept;
    void onCustomSpeedChanged() noexcept;
    void onReplaySpeedUnitSelected(int index) noexcept;

    void updateUi() noexcept;
    void updateControlUi() noexcept;
    void updateTimeUi() noexcept;
    void updatePlayActionTooltip() noexcept;
    void updateControlIcons() noexcept;
    void updateSimulationRateLabel() noexcept;

    void onDefaultMinimalUiButtonTextVisibilityChanged(bool visible) noexcept;
    void onDefaultMinimalUiEssentialButtonVisibilityChanged(bool visible) noexcept;

    void onRecordingStopped() noexcept;
    void onShortcutActivated(FlightSimulatorShortcuts::Action action) noexcept;
    void requestSimulationRate() noexcept;
    void onSimulationRateReceived(float rate) noexcept;

    /*
     * Updates the replay duration (maximum time) and then position slider position.
     */
    void onRecordingDurationChanged() noexcept;

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
    void onModuleActivated(const QString &title, QUuid uuid) noexcept;

    // File menu
    void createNewLogbook() noexcept;
    void openLogbook() noexcept;
    void onRecentFileSelected(const QString &filePath, SecurityToken *securityToken) noexcept;
    void updateRecentFileMenu() noexcept;
    void optimiseLogbook() noexcept;
    void showSettings() noexcept;
    void showLogbookSettings() noexcept;
    void quit() noexcept;

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

    void toggleStayOnTop(bool enable) noexcept;
    void toggleMinimalUi(bool enable) noexcept;

    // Help menu
    void showAboutDialog() noexcept;
    void showOnlineManual() const noexcept;

    // Replay
    void toggleRecord(bool checked) noexcept;
    void togglePause(bool checked) noexcept;
    void togglePlay(bool checked) noexcept;
    void stop() noexcept;

    // Transport / flight
    void skipToBegin() noexcept;
    void skipBackward() noexcept;
    void skipForward() noexcept;
    void skipToEnd() noexcept;
    void toggleLoopReplay(bool checked) noexcept;
    void clearFlight() noexcept;

    // Service
    void onFlightRestored() noexcept;
    void onLogbookConnectionChanged(bool connected) noexcept;

    // Import / export
    void onFlightImport(QAction *action) noexcept;
    void onFlightExport(QAction *action) noexcept;
    void onLocationImport(QAction *action) noexcept;
    void onLocationExport(QAction *action) noexcept;

    // Settings
    void onReplayLoopChanged() noexcept;
    void onStyleKeyChanged() noexcept;
};
#endif // MAINWINDOW_H
