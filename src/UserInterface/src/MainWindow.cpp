#include <QApplication>
#include <QByteArray>
#include <QString>
#include <QTime>
#include <QComboBox>
#include <QSlider>
#include <QMessageBox>

#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/AircraftInfo.h"
#include "../../SkyConnect/src/Frequency.h"
#include "../../SkyConnect/src/SkyConnect.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"

// Public

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Private

void MainWindow::frenchConnection()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    connect(&aircraft, &Aircraft::infoChanged,
            this, &MainWindow::updateInfoUi);
    connect(&aircraft, &Aircraft::positionChanged,
            this, &MainWindow::updatePositionUi);
    connect(&aircraft, &Aircraft::positionChanged,
            this, &MainWindow::updateTimeSliderUi);
    connect(&m_skyConnect, &SkyConnect::playPositionChanged,
            this, &MainWindow::handlePlayPositionChanged);
    connect(&m_skyConnect, &SkyConnect::stateChanged,
            this, &MainWindow::updateUi);
}

// Private slots

void MainWindow::on_recordPushButton_clicked(bool checked)
{
    this->blockSignals(true);
    if (checked) {
        m_skyConnect.startDataSample();
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
        m_skyConnect.startReplay();
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

void MainWindow::on_positionSlider_sliderMoved(int value)
{
    // @todo IMPLEMENT ME Set current position for playback
    // m_skyConnect.getAircraft().set ...
}

void MainWindow::initUi()
{
    this->initSettingsUi();
    this->initRecordUi();
    this->updateUi();
}

void MainWindow::initRecordUi()
{
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
    int percent = static_cast<int>(m_skyConnect.getTimeScale() * 100);
    ui->timeScaleSlider->setValue(percent);
    this->updateSettingsUi();

}

void MainWindow::updateUi()
{
    updateControlUi();
    updateInfoUi();
    updatePositionUi();
}

void MainWindow::updateControlUi()
{
    bool hasRecording = m_skyConnect.getAircraft().getPositions().count() > 0;
    switch (m_skyConnect.getState()) {
    case Connect::Idle:
        ui->recordPushButton->setEnabled(true);
        ui->recordPushButton->setChecked(false);
        ui->pausePushButton->setEnabled(false);
        ui->pausePushButton->setChecked(false);
        ui->playPushButton->setEnabled(hasRecording);
        ui->playPushButton->setChecked(false);
        ui->positionSlider->setEnabled(hasRecording);
        break;
    case Connect::Recording:
        ui->recordPushButton->setEnabled(true);
        ui->recordPushButton->setChecked(true);
        ui->pausePushButton->setEnabled(true);
        ui->pausePushButton->setChecked(false);
        ui->playPushButton->setEnabled(false);
        ui->playPushButton->setChecked(false);
        ui->positionSlider->setEnabled(false);
        ui->positionSlider->setValue(0);
        break;
    case Connect::RecordingPaused:
        ui->recordPushButton->setEnabled(true);
        ui->recordPushButton->setChecked(false);
        ui->pausePushButton->setEnabled(true);
        ui->pausePushButton->setChecked(true);
        ui->playPushButton->setEnabled(false);
        ui->playPushButton->setChecked(false);
        ui->positionSlider->setEnabled(true);
        break;
    case Connect::Playback:
        ui->recordPushButton->setEnabled(false);
        ui->recordPushButton->setChecked(false);
        ui->pausePushButton->setEnabled(true);
        ui->pausePushButton->setChecked(false);
        ui->playPushButton->setEnabled(true);
        ui->playPushButton->setChecked(true);
        ui->positionSlider->setEnabled(true);
        break;
    case Connect::PlaybackPaused:
        ui->recordPushButton->setEnabled(false);
        ui->recordPushButton->setChecked(false);
        ui->pausePushButton->setEnabled(true);
        ui->pausePushButton->setChecked(true);
        ui->playPushButton->setEnabled(true);
        ui->playPushButton->setChecked(true);
        ui->positionSlider->setEnabled(true);
        break;
    default:
        break;
    }
}

void MainWindow::updateInfoUi()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();

    ui->nameLineEdit->setText(aircraftInfo.name);
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);
    ui->initialAirspeedLineEdit->setText(QString::number(aircraftInfo.initialAirspeed));
}

void MainWindow::updatePositionUi()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    const Position &position = aircraft.getLastPosition();

    ui->latitudeLineEdit->setText(QString::number(position.latitude));
    ui->longitudeLineEdit->setText(QString::number(position.longitude));
    ui->altitudeLineEdit->setText(QString::number(position.altitude));
    ui->pitchLineEdit->setText(QString::number(position.pitch));
    ui->bankLineEdit->setText(QString::number(position.bank));
    ui->headingLineEdit->setText(QString::number(position.heading));
}

void MainWindow::updateSettingsUi()
{
    int percent = ui->timeScaleSlider->value();
    ui->timeScalePercentLabel->setText(QString::number(percent));
}

void MainWindow::updateTimeSliderUi()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    const Position &position = aircraft.getLastPosition();
    QTime time(0, 0, 0, 0);
    if (position.isValid()) {
        time = time.addMSecs(position.timestamp);
        ui->timestampTimeEdit->setTime(time);
    } else {
        ui->timestampTimeEdit->setTime(time);
    }
}

void MainWindow::on_quitAction_triggered()
{
    QApplication::quit();
}

void MainWindow::on_aboutQtAction_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::handlePlayPositionChanged(qint64 timestamp) {
    qDebug("Play position timestamp: %lld", timestamp);
    qint64 endTimeStamp = m_skyConnect.getAircraft().getLastPosition().timestamp;
    qint64 ts = qMin(timestamp, endTimeStamp);

    int percent;
    if (endTimeStamp > 0) {
        percent = qRound(100.0 * (static_cast<float>(ts) / static_cast<float>(endTimeStamp)));
    } else {
        percent = 0;
    }
    ui->positionSlider->setValue(percent);

    QTime time(0, 0, 0, 0);
    time = time.addMSecs(timestamp);
    ui->timestampTimeEdit->setTime(time);
}
