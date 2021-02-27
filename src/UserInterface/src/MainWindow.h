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
    SimulationVariablesDialog *m_simulationVariablesDialog;
    double m_lastCustomPlaybackSpeed;

    void frenchConnection();

private slots:
    void on_recordPushButton_clicked(bool checked = false);
    void on_pausePushButton_clicked(bool checked = false);
    void on_playPushButton_clicked(bool checked = false);
    void on_recordFrequencyComboBox_activated(int index);
    void on_playbackFrequencyComboBox_activated(int index);
    void on_positionSlider_sliderPressed();
    void on_positionSlider_sliderMoved(int value);
    void on_positionSlider_sliderReleased();
    void on_timestampTimeEdit_timeChanged(const QTime &time);
    void on_customPlaybackSpeedLineEdit_editingFinished();

    void initUi();
    void initControlUi();
    void initSettingsUi();

    void updateUi();
    void updateControlUi();
    void updateRecordingTime();

    // File menu
    void on_quitAction_triggered();
    // View menu
    void on_showSimulationVariablesAction_triggered();
    // About menu
    void on_aboutAction_triggered();
    void on_aboutQtAction_triggered();

    void handlePlayPositionChanged(qint64 timestamp);
    void handlePlaybackSpeedSelected(int selection);
};
#endif // MAINWINDOW_H
