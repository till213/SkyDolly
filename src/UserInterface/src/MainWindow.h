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
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private:
    Ui::MainWindow *ui;
    MainWindowPrivate *d;

    void frenchConnection();
    void initUi();
    void initControlUi();

private slots:
    void on_positionSlider_sliderPressed();
    void on_positionSlider_valueChanged(int value);
    void on_positionSlider_sliderReleased();
    void on_timestampTimeEdit_timeChanged(const QTime &time);
    void on_customPlaybackSpeedLineEdit_editingFinished();

    void updateUi();
    void updateControlUi();
    void updateRecordingTime();
    void updateFileMenu();
    void updateMainWindow();

    // File menu
    void on_importCSVAction_triggered();
    void on_exportCSVAction_triggered();
    void on_showSettingsAction_triggered();
    void on_quitAction_triggered();
    // Window menu
    void on_showSimulationVariablesAction_triggered(bool enabled);
    void on_stayOnTopAction_triggered(bool enabled);
    // About menu
    void on_aboutAction_triggered();
    void on_aboutQtAction_triggered();

    void handlePlayPositionChanged(qint64 timestamp);
    void handlePlaybackSpeedSelected(int selection);

    // Playback
    void toggleRecord(bool checked);
    void togglePause(bool checked);
    void togglePlay(bool checked);
    void stop();

    // Transport
    void skipToBegin();
    void skipBackward();
    void skipForward();
    void skipToEnd();

    // Application
    void handleLastWindowClosed();

};
#endif // MAINWINDOW_H
