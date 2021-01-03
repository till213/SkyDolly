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

// PUBLIC

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initUi();
    this->frenchConnection();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// PRIVATE

void MainWindow::frenchConnection()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    connect(&aircraft, &Aircraft::infoChanged,
            this, &MainWindow::updateInfoUi);
    connect(&aircraft, &Aircraft::positionChanged,
            this, &MainWindow::updatePositionUi);
    connect(&aircraft, &Aircraft::positionIndexChanged,
            this, &MainWindow::updatePlaybackUi);
}

// PRIVATE SLOTS

void MainWindow::on_recordPushButton_clicked(bool checked)
{
    if (checked) {
        m_skyConnect.startDataSample();
    } else {
        m_skyConnect.stopDataSample();
    }
}

void MainWindow::on_deletePushButton_clicked()
{
    Aircraft &aircraft = m_skyConnect.getAircraft();
    aircraft.clear();
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
    updateRecordUi();
    updateInfoUi();
    updatePositionUi();
}

void MainWindow::updateRecordUi()
{
    int length = m_skyConnect.getAircraft().getPositions().length();
    ui->positionSlider->setMaximum(length);
}

void MainWindow::updatePlaybackUi(int positionIndex)
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    int index = ui->positionSlider->value();
    const Position &position = aircraft.getPositions().at(index);

    QTime time(0, 0, 0, 0);
    time = time.addMSecs(position.timestamp);
    ui->timestampTimeEdit->setTime(time);

    ui->positionSlider->setValue(positionIndex);
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
    QTime time(0, 0, 0, 0);
    time = time.addMSecs(position.timestamp);
    ui->timestampTimeEdit->setTime(time);
}

void MainWindow::updateSettingsUi()
{
    int percent = ui->timeScaleSlider->value();
    ui->timeScalePercentLabel->setText(QString::number(percent));
}

void MainWindow::on_quitAction_triggered()
{
    QApplication::quit();
}

void MainWindow::on_aboutQtAction_triggered()
{
    QMessageBox::aboutQt(this);
}

