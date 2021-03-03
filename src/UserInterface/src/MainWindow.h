#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../../SkyConnect/src/SkyConnect.h"
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

class USERINTERFACE_API MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private:
    Ui::MainWindow *ui;
    SkyConnect m_skyConnect;
    Connect::State m_previousState;
    QButtonGroup *m_playbackSpeedButtonGroup;
    AboutDialog *m_aboutDialog;
    SettingsDialog *m_settingsDialog;
    SimulationVariablesDialog *m_simulationVariablesDialog;
    double m_lastCustomPlaybackSpeed;

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

    // File menu
    void on_exportCSVAction_triggered();
    void on_showSettingsAction_triggered();
    void on_quitAction_triggered();
    // View menu
    void on_showSimulationVariablesAction_triggered(bool enabled);
    // About menu
    void on_aboutAction_triggered();
    void on_aboutQtAction_triggered();

    void handlePlayPositionChanged(qint64 timestamp);
    void handlePlaybackSpeedSelected(int selection);

    // Playback
    void toggleRecord(bool checked);
    void togglePause(bool checked);
    void togglePlay(bool checked);

    // Transport
    void skipToBegin();
    void skipBackward();
    void skipForward();
    void skipToEnd();

    // Application
    void handleLastWindowClosed();

};
#endif // MAINWINDOW_H
