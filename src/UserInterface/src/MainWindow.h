#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../../SkyConnect/src/SkyConnect.h"
#include "../../SkyConnect/src/Connect.h"
#include "UserInterfaceLib.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class USERINTERFACE_API MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

private:
    Ui::MainWindow *ui;
    SkyConnect m_skyConnect;

    void frenchConnection();

private slots:
    void on_recordPushButton_clicked(bool checked = false);
    void on_pausePushButton_clicked(bool checked = false);
    void on_playPushButton_clicked(bool checked = false);
    void on_recordFrequencyComboBox_activated(int index);
    void on_playbackFrequencyComboBox_activated(int index);
    void on_timeScaleSlider_valueChanged();
    void on_positionSlider_sliderMoved(int value);

    void initUi();
    void initRecordUi();
    void initSettingsUi();

    void updateUi();
    void updateControlUi();
    void updateInfoUi();
    void updatePositionUi();
    void updateSettingsUi();
    void updateTimeSliderUi();

    // Actions
    void on_quitAction_triggered();
    void on_aboutQtAction_triggered();

    void handlePlayPositionChanged(qint64 timestamp);
};
#endif // MAINWINDOW_H
