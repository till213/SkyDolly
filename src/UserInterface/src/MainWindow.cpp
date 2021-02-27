#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QTime>
#include <QTimeEdit>
#include <QComboBox>
#include <QSlider>
#include <QMessageBox>

#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/AircraftInfo.h"
#include "../../SkyConnect/src/Frequency.h"
#include "../../SkyConnect/src/SkyConnect.h"
#include "AboutDialog.h"
#include "SimulationVariablesDialog.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"

namespace {
    constexpr int PositionSliderMin = 0;
    constexpr int PositionSliderMax = 100;
    constexpr qint64 MilliSecondsPerSecond = 1000;
    constexpr qint64 MilliSecondsPerMinute = 60 * MilliSecondsPerSecond;
    constexpr qint64 MilliSecondsPerHour = 60 * MilliSecondsPerMinute;
}

// PUBLIC

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_previousState(m_skyConnect.getState())
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

MainWindow::~MainWindow()
{
    // SkyConnect might still be recording, and hence a state changed
    // signal would be emitted upon destruction, upon which an attempt to
    // update the UI - which is deleted at this point in time already - would
    // be made, and fail with invalid address access
    m_skyConnect.disconnect();
    // And while we're at it we also disconnect all signals from the aircraft
    m_skyConnect.getAircraft().disconnect();
    delete ui;
}

// PRIVATE

void MainWindow::frenchConnection()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    connect(&aircraft, &Aircraft::dataChanged,
            this, &MainWindow::updateRecordingTimeEdit);
    connect(&m_skyConnect, &SkyConnect::playPositionChanged,
            this, &MainWindow::handlePlayPositionChanged);
    connect(&m_skyConnect, &SkyConnect::stateChanged,
            this, &MainWindow::updateUi);
}

// PRIVATE SLOTS

void MainWindow::on_recordPushButton_clicked(bool checked)
{
    this->blockSignals(true);
    if (checked) {
        m_skyConnect.startDataSample();
    } else if (m_skyConnect.isPaused()) {
        // The record button also unpauses a paused recording
        m_skyConnect.setPaused(false);
    } else {
        m_skyConnect.stopDataSample();
    }
    this->blockSignals(false);
}

void MainWindow::on_pausePushButton_clicked(bool checked)
{
    m_skyConnect.setPaused(checked);
}

void MainWindow::on_playPushButton_clicked(bool checked)
{
    if (checked) {
        if (m_skyConnect.isPlayPositionAtEnd()) {
            // Jump back to start
            m_skyConnect.setPlayPosition(0);
        }
        m_skyConnect.startReplay(false);
    } else if (m_skyConnect.isPaused()) {
        // The play button also unpauses a paused replay
        m_skyConnect.setPaused(false);
    } else {
        m_skyConnect.stopReplay();
    }
}

void MainWindow::on_recordFrequencyComboBox_activated(int index)
{
    m_skyConnect.setSampleFrequency(static_cast<Frequency::Frequency>(index));
}

void MainWindow::on_playbackFrequencyComboBox_activated(int index)
{
    m_skyConnect.setReplayFrequency(static_cast<Frequency::Frequency>(index));
}

void MainWindow::on_timeScaleSlider_valueChanged()
{
    double timeScale = static_cast<double>(ui->timeScaleSlider->value() / 100.0);
    m_skyConnect.setTimeScale(timeScale);
    this->updateSettingsUi();
}

void MainWindow::on_positionSlider_sliderPressed()
{
    m_previousState = m_skyConnect.getState();
    if (m_previousState == Connect::State::Playback) {
        // Pause the playback while sliding the position slider
        m_skyConnect.setPaused(true);
    }
}

void MainWindow::on_positionSlider_sliderMoved(int value)
{
    double scale = static_cast<double>(value) / 100.0f;
    qint64 timestamp = static_cast<qint64>(qRound(scale * static_cast<double>(m_skyConnect.getAircraft().getLastAircraftData().timestamp)));

    // Prevent the timestampTimeEdit field to set the play position as well
    ui->timestampTimeEdit->blockSignals(true);
    m_skyConnect.setPlayPosition(timestamp);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::on_positionSlider_sliderReleased()
{
    if (m_previousState == Connect::State::Playback) {
        m_skyConnect.setPaused(false);
    }
}

void MainWindow::on_timestampTimeEdit_timeChanged(const QTime &time)
{
    Connect::State state = m_skyConnect.getState();
    if (state == Connect::State::Idle || state == Connect::State::PlaybackPaused) {
        qint64 timestamp = time.hour() * MilliSecondsPerHour + time.minute() * MilliSecondsPerMinute + time.second() * MilliSecondsPerSecond;
        m_skyConnect.setPlayPosition(timestamp);
    }
}

void MainWindow::initUi()
{
    setWindowIcon(QIcon(":/img/SkyDolly.png"));

    // Dialogs
    Qt::WindowFlags flags;
    flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    m_simulationVariablesDialog = new SimulationVariablesDialog(m_skyConnect, this);
    m_simulationVariablesDialog->setWindowFlags(flags);
    m_aboutDialog = new AboutDialog(this);
    m_aboutDialog->setWindowFlags(flags);

    this->initSettingsUi();
    this->initRecordUi();
    this->updateUi();
}

void MainWindow::initRecordUi()
{
    ui->positionSlider->setMinimum(PositionSliderMin);
    ui->positionSlider->setMaximum(PositionSliderMax);
    ui->timestampTimeEdit->setDisplayFormat("hh:mm:ss");
}

void MainWindow::initSettingsUi()
{
    // Record
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz1, tr("1 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz2, tr("2 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz5, tr("5 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz10, tr("10 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz15, tr("15 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz20, tr("20 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz24, tr("24 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz25, tr("25 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz30, tr("30 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz45, tr("45 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz50, tr("50 Hz"));
    ui->recordFrequencyComboBox->insertItem(Frequency::Hz60, tr("60 Hz"));

    // Playback
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz1, tr("1 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz2, tr("2 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz5, tr("5 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz10, tr("10 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz15, tr("15 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz20, tr("20 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz24, tr("24 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz25, tr("25 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz30, tr("30 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz45, tr("45 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz50, tr("50 Hz"));
    ui->playbackFrequencyComboBox->insertItem(Frequency::Hz60, tr("60 Hz"));

    // Initial values
    ui->recordFrequencyComboBox->setCurrentIndex(m_skyConnect.getSampleFrequency());
    ui->playbackFrequencyComboBox->setCurrentIndex(m_skyConnect.getReplayFrequency());
    int percent = static_cast<int>(m_skyConnect.getTimeScale() * PositionSliderMax);
    ui->timeScaleSlider->setValue(percent);
    this->updateSettingsUi();
}

void MainWindow::updateUi()
{
    updateControlUi();
}

void MainWindow::updateControlUi()
{
    bool hasRecording = m_skyConnect.getAircraft().getAircraftData().count() > 0;
    switch (m_skyConnect.getState()) {
    case Connect::Idle:
        ui->recordPushButton->setEnabled(true);
        ui->recordPushButton->setChecked(false);
        ui->pausePushButton->setEnabled(false);
        ui->pausePushButton->setChecked(false);
        ui->playPushButton->setEnabled(hasRecording);
        ui->playPushButton->setChecked(false);
        ui->positionSlider->setEnabled(hasRecording);
        ui->timestampTimeEdit->setEnabled(hasRecording);
        break;
    case Connect::Recording:
        ui->recordPushButton->setEnabled(true);
        ui->recordPushButton->setChecked(true);
        ui->pausePushButton->setEnabled(true);
        ui->pausePushButton->setChecked(false);
        ui->playPushButton->setEnabled(false);
        ui->playPushButton->setChecked(false);
        ui->positionSlider->setEnabled(false);
        ui->positionSlider->setValue(PositionSliderMax);
        ui->timestampTimeEdit->setEnabled(false);
        break;
    case Connect::RecordingPaused:
        ui->recordPushButton->setEnabled(true);
        ui->recordPushButton->setChecked(true);
        ui->pausePushButton->setEnabled(true);
        ui->pausePushButton->setChecked(true);
        ui->playPushButton->setEnabled(false);
        ui->playPushButton->setChecked(false);
        ui->positionSlider->setEnabled(true);
        ui->timestampTimeEdit->setEnabled(false);
        break;
    case Connect::Playback:
        ui->recordPushButton->setEnabled(false);
        ui->recordPushButton->setChecked(false);
        ui->pausePushButton->setEnabled(true);
        ui->pausePushButton->setChecked(false);
        ui->playPushButton->setEnabled(true);
        ui->playPushButton->setChecked(true);
        ui->positionSlider->setEnabled(true);
        ui->timestampTimeEdit->setEnabled(false);
        break;
    case Connect::PlaybackPaused:
        ui->recordPushButton->setEnabled(false);
        ui->recordPushButton->setChecked(false);
        ui->pausePushButton->setEnabled(true);
        ui->pausePushButton->setChecked(true);
        ui->playPushButton->setEnabled(true);
        ui->playPushButton->setChecked(true);
        ui->positionSlider->setEnabled(true);
        ui->timestampTimeEdit->setEnabled(true);
        break;
    default:
        break;
    }
}

void MainWindow::updateSettingsUi()
{
    int percent = ui->timeScaleSlider->value();
    ui->timeScalePercentLabel->setText(QString::number(percent));
}

void MainWindow::updateRecordingTimeEdit()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    const AircraftData &aircraftData = aircraft.getLastAircraftData();
    QTime time(0, 0, 0, 0);
    if (!aircraftData.isNull()) {
        time = time.addMSecs(aircraftData.timestamp);
        ui->timestampTimeEdit->setTime(time);
    } else {
        ui->timestampTimeEdit->setTime(time);
    }
    ui->timestampTimeEdit->setMaximumTime(time);
}

void MainWindow::on_quitAction_triggered()
{
    QApplication::quit();
}

void MainWindow::on_showSimulationVariablesAction_triggered()
{
    m_simulationVariablesDialog->show();
}

void MainWindow::on_aboutAction_triggered()
{
    m_aboutDialog->exec();
}

void MainWindow::on_aboutQtAction_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::handlePlayPositionChanged(qint64 timestamp) {
    qint64 endTimeStamp = m_skyConnect.getAircraft().getLastAircraftData().timestamp;
    qint64 ts = qMin(timestamp, endTimeStamp);

    int percent;
    if (endTimeStamp > 0) {
        percent = qRound(PositionSliderMax * (static_cast<double>(ts) / static_cast<double>(endTimeStamp)));
    } else {
        percent = 0;
    }
    ui->positionSlider->blockSignals(true);
    ui->positionSlider->setValue(percent);
    ui->positionSlider->blockSignals(false);

    QTime time(0, 0, 0, 0);
    time = time.addMSecs(timestamp);
    ui->timestampTimeEdit->setTime(time);
}
