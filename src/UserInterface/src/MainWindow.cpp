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
#include <QApplication>
#include <QByteArray>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QStandardPaths>
#include <QString>
#include <QTime>
#include <QTimeEdit>
#include <QComboBox>
#include <QSlider>
#include <QLineEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <QMessageBox>
#include <QDoubleValidator>

#include "../../Kernel/src/Export/CSVExport.h"
#include "../../Kernel/src/Import/CSVImport.h"
#include "../../Kernel/src/Version.h"
#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/AircraftInfo.h"
#include "../../Kernel/src/SampleRate.h"
#include "../../SkyConnect/src/SkyManager.h"
#include "../../SkyConnect/src/SkyConnectIntf.h"
#include "../../SkyConnect/src/Connect.h"
#include "Dialogs/AboutDialog.h"
#include "Dialogs/SettingsDialog.h"
#include "Dialogs/SimulationVariablesDialog.h"
#include "Widgets/ActionButton.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"

namespace {
    constexpr int PositionSliderMin = 0;
    constexpr int PositionSliderMax = 1000;
    constexpr double PlaybackSpeedMin = 0.01;
    // A playback speed with 20'000 % (factor 200) should be fast enough
    constexpr double PlaybackSpeedMax = 20000.0;
    constexpr qint64 MilliSecondsPerSecond = 1000;
    constexpr qint64 MilliSecondsPerMinute = 60 * MilliSecondsPerSecond;
    constexpr qint64 MilliSecondsPerHour = 60 * MilliSecondsPerMinute;

    enum PlaybackSpeed {
        PlaybackSpeed10,
        PlaybackSpeed25,
        PlaybackSpeed50,
        PlaybackSpeed75,
        PlaybackSpeed100,
        PlaybackSpeed200,
        PlaybackSpeed400,
        PlaybackSpeed800,
        PlaybackSpeedCustom
    };
}

// PUBLIC

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_skyConnect(SkyManager::getInstance().currentSkyConnect()),
      m_previousState(Connect::State::Idle)
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

MainWindow::~MainWindow()
{
    // SkyConnect might still be recording, and hence a state changed
    // signal would be emitted upon destruction, upon which an attempt to
    // update the UI - which is deleted at this point in time already - would
    // be made, and fail with invalid address access
    m_skyConnect->disconnect();
    // And while we're at it we also disconnect all signals from the aircraft
    m_skyConnect->getAircraft().disconnect();
    delete ui;
}

// PRIVATE

void MainWindow::frenchConnection()
{
    const Aircraft &aircraft = m_skyConnect->getAircraft();
    connect(&aircraft, &Aircraft::dataChanged,
            this, &MainWindow::updateRecordingTime);
    connect(m_skyConnect, &SkyConnectIntf::aircraftDataSent,
            this, &MainWindow::handlePlayPositionChanged);
    connect(m_skyConnect, &SkyConnectIntf::stateChanged,
            this, &MainWindow::updateUi);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(m_playbackSpeedButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::updateControlUi);
    connect(m_playbackSpeedButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::handlePlaybackSpeedSelected);
#else
    connect(m_playbackSpeedButtonGroup, &QButtonGroup::idClicked,
            this, &MainWindow::updateControlUi);
    connect(m_playbackSpeedButtonGroup, &QButtonGroup::idClicked,
            this, &MainWindow::handlePlaybackSpeedSelected);
#endif

    // Actions
    connect(ui->recordAction, &QAction::triggered,
            this, &MainWindow::toggleRecord);
    connect(ui->playAction, &QAction::triggered,
            this, &MainWindow::togglePlay);
    connect(ui->pauseAction, &QAction::triggered,
            this, &MainWindow::togglePause);
    connect(ui->skipToBeginAction, &QAction::triggered,
            this, &MainWindow::skipToBegin);
    connect(ui->backwardAction, &QAction::triggered,
            this, &MainWindow::skipBackward);
    connect(ui->forwardAction, &QAction::triggered,
            this, &MainWindow::skipForward);
    connect(ui->skipToEndAction, &QAction::triggered,
            this, &MainWindow::skipToEnd);

    // Application
    connect(dynamic_cast<QGuiApplication *>(QGuiApplication::instance()), &QGuiApplication::lastWindowClosed,
            this, &MainWindow::handleLastWindowClosed);

    // Settings
    connect(&Settings::getInstance(), &Settings::changed,
            this, &MainWindow::updateMainWindow);
}

void MainWindow::initUi()
{
    setWindowIcon(QIcon(":/img/SkyDolly.png"));

    // Dialogs
    m_simulationVariablesDialog = new SimulationVariablesDialog(*m_skyConnect, this);
    m_aboutDialog = new AboutDialog(this);
    m_settingsDialog = new SettingsDialog(this);

    ui->stayOnTopAction->setChecked(Settings::getInstance().isWindowStaysOnTopEnabled());
    this->initControlUi();
}

void MainWindow::initControlUi()
{
    m_playbackSpeedButtonGroup = new QButtonGroup(this);
    m_playbackSpeedButtonGroup->addButton(ui->playbackSpeed10RadioButton, PlaybackSpeed10);
    m_playbackSpeedButtonGroup->addButton(ui->playbackSpeed25RadioButton, PlaybackSpeed25);
    m_playbackSpeedButtonGroup->addButton(ui->playbackSpeed50RadioButton, PlaybackSpeed50);
    m_playbackSpeedButtonGroup->addButton(ui->playbackSpeed75RadioButton, PlaybackSpeed75);
    m_playbackSpeedButtonGroup->addButton(ui->playbackSpeed100RadioButton, PlaybackSpeed100);
    m_playbackSpeedButtonGroup->addButton(ui->playbackSpeed200RadioButton, PlaybackSpeed200);
    m_playbackSpeedButtonGroup->addButton(ui->playbackSpeed400RadioButton, PlaybackSpeed400);
    m_playbackSpeedButtonGroup->addButton(ui->playbackSpeed800RadioButton, PlaybackSpeed800);
    m_playbackSpeedButtonGroup->addButton(ui->customPlaybackSpeedRadioButton, PlaybackSpeedCustom);

    ui->positionSlider->setMinimum(PositionSliderMin);
    ui->positionSlider->setMaximum(PositionSliderMax);
    ui->timestampTimeEdit->setDisplayFormat("hh:mm:ss");

    QDoubleValidator *customPlaybackSpeedValidator = new QDoubleValidator(ui->customPlaybackSpeedLineEdit);
    ui->customPlaybackSpeedLineEdit->setValidator(customPlaybackSpeedValidator);
    customPlaybackSpeedValidator->setBottom(PlaybackSpeedMin);
    customPlaybackSpeedValidator->setTop(PlaybackSpeedMax);

    double playbackSpeed = m_skyConnect->getTimeScale();
    m_lastCustomPlaybackSpeed = playbackSpeed;
    if (qFuzzyCompare(m_skyConnect->getTimeScale(), 1.0)) {
        ui->playbackSpeed100RadioButton->setChecked(true);
    } else {
        ui->customPlaybackSpeedRadioButton->setChecked(true);
        ui->customPlaybackSpeedLineEdit->setText(QString::number(playbackSpeed * 100.0));
    }

    // Record/playback control buttons
    ActionButton *recordButton = new ActionButton(this);
    recordButton->setAction(ui->recordAction);
    ui->controlButtonLayout->insertWidget(0, recordButton);

    ActionButton *skipToStartButton = new ActionButton(this);
    skipToStartButton->setAction(ui->skipToBeginAction);
    ui->controlButtonLayout->insertWidget(1, skipToStartButton);

    ActionButton *skipBackwardButton = new ActionButton(this);
    skipBackwardButton->setAction(ui->backwardAction);
    ui->controlButtonLayout->insertWidget(2, skipBackwardButton);

    ActionButton *pauseButton = new ActionButton(this);
    pauseButton->setAction(ui->pauseAction);
    ui->controlButtonLayout->insertWidget(3, pauseButton);

    ActionButton *playButton = new ActionButton(this);
    playButton->setAction(ui->playAction);
    ui->controlButtonLayout->insertWidget(4, playButton);

    ActionButton *skipForwardButton = new ActionButton(this);
    skipForwardButton->setAction(ui->forwardAction);
    ui->controlButtonLayout->insertWidget(5, skipForwardButton);

    ActionButton *skipToEndButton = new ActionButton(this);
    skipToEndButton->setAction(ui->skipToEndAction);
    ui->controlButtonLayout->insertWidget(6, skipToEndButton);
}

// PRIVATE SLOTS

void MainWindow::on_positionSlider_sliderPressed()
{
    m_previousState = m_skyConnect->getState();
    if (m_previousState == Connect::State::Playback) {
        // Pause the playback while sliding the position slider
        m_skyConnect->setPaused(true);
    }
}

void MainWindow::on_positionSlider_valueChanged(int value)
{
    double scale = static_cast<double>(value) / static_cast<double>(PositionSliderMax);
    qint64 timestamp = static_cast<qint64>(qRound(scale * static_cast<double>(m_skyConnect->getAircraft().getLastAircraftData().timestamp)));

    // Prevent the timestampTimeEdit field to set the play position as well
    ui->timestampTimeEdit->blockSignals(true);
    m_skyConnect->setCurrentTimestamp(timestamp);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::on_positionSlider_sliderReleased()
{
    if (m_previousState == Connect::State::Playback) {
        m_skyConnect->setPaused(false);
    }
}

void MainWindow::on_timestampTimeEdit_timeChanged(const QTime &time)
{
    Connect::State state = m_skyConnect->getState();
    if (state == Connect::State::Idle || state == Connect::State::PlaybackPaused) {
        qint64 timestamp = time.hour() * MilliSecondsPerHour + time.minute() * MilliSecondsPerMinute + time.second() * MilliSecondsPerSecond;
        m_skyConnect->setCurrentTimestamp(timestamp);
    }
}

void MainWindow::on_customPlaybackSpeedLineEdit_editingFinished() {
    QString text = ui->customPlaybackSpeedLineEdit->text();
    if (!text.isEmpty()) {
        m_lastCustomPlaybackSpeed = text.toDouble() / 100.0;
        m_skyConnect->setTimeScale(m_lastCustomPlaybackSpeed);
    }
}

void MainWindow::updateUi()
{
    updateControlUi();
    updateFileMenu();
    updateMainWindow();
}

void MainWindow::updateControlUi()
{
    bool hasRecording = m_skyConnect->getAircraft().getAllAircraftData().count() > 0;
    switch (m_skyConnect->getState()) {
    case Connect::Idle:
        // Actions
        ui->recordAction->setEnabled(true);
        ui->recordAction->setChecked(false);
        ui->pauseAction->setEnabled(false);
        ui->pauseAction->setChecked(false);
        ui->playAction->setEnabled(hasRecording);
        ui->playAction->setChecked(false);
        // Transport
        ui->skipToBeginAction->setEnabled(hasRecording);
        ui->backwardAction->setEnabled(hasRecording);
        ui->forwardAction->setEnabled(hasRecording);
        ui->skipToEndAction->setEnabled(hasRecording);
        // Position
        ui->positionSlider->setEnabled(hasRecording);
        ui->timestampTimeEdit->setEnabled(hasRecording);
        break;
    case Connect::Recording:
        // Actions
        ui->recordAction->setEnabled(true);
        ui->recordAction->setChecked(true);
        ui->pauseAction->setEnabled(true);
        ui->pauseAction->setChecked(false);
        ui->playAction->setEnabled(false);
        ui->playAction->setChecked(false);
        // Transport
        ui->skipToBeginAction->setEnabled(false);
        ui->backwardAction->setEnabled(false);
        ui->forwardAction->setEnabled(false);
        ui->skipToEndAction->setEnabled(false);
        // Position
        ui->positionSlider->setEnabled(false);
        ui->positionSlider->setValue(PositionSliderMax);
        ui->timestampTimeEdit->setEnabled(false);
        break;
    case Connect::RecordingPaused:
        // Actions
        ui->recordAction->setChecked(false);
        ui->pauseAction->setChecked(true);
        break;
    case Connect::Playback:
        // Actions
        ui->recordAction->setEnabled(false);
        ui->recordAction->setChecked(false);
        ui->pauseAction->setEnabled(true);
        ui->pauseAction->setChecked(false);
        ui->playAction->setEnabled(true);
        ui->playAction->setChecked(true);
        // Transport
        ui->skipToBeginAction->setEnabled(true);
        ui->backwardAction->setEnabled(true);
        ui->forwardAction->setEnabled(true);
        ui->skipToEndAction->setEnabled(true);
        // Position
        ui->positionSlider->setEnabled(true);
        ui->timestampTimeEdit->setEnabled(false);
        break;
    case Connect::PlaybackPaused:
        // Actions
        ui->pauseAction->setChecked(true);
        ui->playAction->setChecked(false);
        break;
    default:
        break;
    }

    if (ui->customPlaybackSpeedRadioButton->isChecked()) {
        ui->customPlaybackSpeedLineEdit->setEnabled(true);
        ui->customPlaybackSpeedLineEdit->setText(QString::number(m_lastCustomPlaybackSpeed * 100.0));
    } else {
        ui->customPlaybackSpeedLineEdit->setEnabled(false);
        ui->customPlaybackSpeedLineEdit->clear();
    }
}

void MainWindow::updateRecordingTime()
{
    const Aircraft &aircraft = m_skyConnect->getAircraft();
    const AircraftData &aircraftData = aircraft.getLastAircraftData();
    ui->timestampTimeEdit->blockSignals(true);
    QTime time(0, 0, 0, 0);
    if (!aircraftData.isNull()) {
        time = time.addMSecs(aircraftData.timestamp);
        ui->timestampTimeEdit->setTime(time);
    } else {
        ui->timestampTimeEdit->setTime(time);
    }
    ui->timestampTimeEdit->setMaximumTime(time);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::updateFileMenu()
{
    bool hasRecording = m_skyConnect->getAircraft().getAllAircraftData().count() > 0;
    switch (m_skyConnect->getState()) {
    case Connect::State::Recording:
        // Fall-thru intentional
    case Connect::State::RecordingPaused:
        ui->importCSVAction->setEnabled(false);
        ui->exportCSVAction->setEnabled(false);
        break;
    default:
        ui->importCSVAction->setEnabled(true);
        ui->exportCSVAction->setEnabled(hasRecording);
    }
}

void MainWindow::updateMainWindow()
{
    Settings &settings = Settings::getInstance();
    if (settings.isWindowStaysOnTopEnabled() && !(windowFlags() & Qt::WindowStaysOnTopHint)) {
        Qt::WindowFlags flags = windowFlags();
        if (Settings::getInstance().isWindowStaysOnTopEnabled()) {
            this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
            this->show();
        } else {
            this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
            this->show();
        }
    }

    ui->recordAction->setToolTip(tr("Record @%1 Hz").arg(Settings::getInstance().getRecordSampleRateValue()));
    ui->playAction->setToolTip(tr("Play @%1 Hz").arg(Settings::getInstance().getPlaybackSampleRateValue()));
}

void MainWindow::on_importCSVAction_triggered()
{
    QString documentPath;
    QStringList standardLocations = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation);
    if (standardLocations.count() > 0) {
        documentPath = standardLocations.first();
    } else {
        documentPath = ".";
    }
    QString filePath = QFileDialog::getOpenFileName(this, tr("Import CSV"), documentPath, QString("*.csv"));
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        CSVImport csvImport;
        bool ok = csvImport.importData(file, m_skyConnect->getAircraft());
        if (ok) {
            updateUi();
            m_skyConnect->startReplay(true);
            m_skyConnect->setPaused(true);
        }
    }
}

void MainWindow::on_exportCSVAction_triggered()
{
    QMessageBox message;
    Version version;

    QMessageBox::StandardButton reply;
    int previewInfoCount = Settings::getInstance().getPreviewInfoDialogCount();
    if (previewInfoCount > 0) {
        --previewInfoCount;
        reply = QMessageBox::question(this, "Preview",
            QString("%1 %2 is an early preview version. The format of the exported CSV values may change partially or completely, "
                    "even rendering the current exported values unreadable in future versions.\n\n"
                    "This dialog will be shown %3 more times.").arg(Version::getApplicationName(), Version::getApplicationVersion()).arg(previewInfoCount),
            QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Abort);
        Settings::getInstance().setPreviewInfoDialogCount(previewInfoCount);
    } else {
        reply = QMessageBox::StandardButton::Ok;
    }

    if (reply == QMessageBox::StandardButton::Ok) {
        QString documentPath;
        QStringList standardLocations = QStandardPaths::standardLocations(QStandardPaths::StandardLocation::DocumentsLocation);
        if (standardLocations.count() > 0) {
            documentPath = standardLocations.first();
        } else {
            documentPath = ".";
        }
        QString filePath = QFileDialog::getSaveFileName(this, tr("Export CSV"), documentPath, QString("*.csv"));
        if (!filePath.isEmpty()) {
            QFile file(filePath);
            CSVExport csvExport;
            csvExport.exportData(m_skyConnect->getAircraft(), file);
        }
    }
}

void MainWindow::on_showSettingsAction_triggered()
{
    m_settingsDialog->exec();
}

void MainWindow::on_quitAction_triggered()
{
    QApplication::quit();
}

void MainWindow::on_showSimulationVariablesAction_triggered(bool enabled)
{
    if (enabled) {
        m_simulationVariablesDialog->show();
    } else {
        m_simulationVariablesDialog->close();
    }
}

void MainWindow::on_stayOnTopAction_triggered(bool enabled)
{
    Settings::getInstance().setWindowStaysOnTopEnabled(enabled);
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
    qint64 endTimeStamp = m_skyConnect->getAircraft().getLastAircraftData().timestamp;
    qint64 ts = qMin(timestamp, endTimeStamp);

    int sliderPosition;
    if (endTimeStamp > 0) {
        sliderPosition = qRound(PositionSliderMax * (static_cast<double>(ts) / static_cast<double>(endTimeStamp)));
    } else {
        sliderPosition = 0;
    }
    ui->positionSlider->blockSignals(true);
    ui->positionSlider->setValue(sliderPosition);
    ui->positionSlider->blockSignals(false);

    QTime time(0, 0, 0, 0);
    time = time.addMSecs(timestamp);
    ui->timestampTimeEdit->blockSignals(true);
    ui->timestampTimeEdit->setTime(time);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::handlePlaybackSpeedSelected(int selection) {

    double timeScale;
    switch (selection) {
    case PlaybackSpeed10:
        timeScale = 0.1;
        break;
    case PlaybackSpeed25:
        timeScale = 0.25;
        break;
    case PlaybackSpeed50:
        timeScale = 0.5;
        break;
    case PlaybackSpeed75:
        timeScale = 0.75;
        break;
    case PlaybackSpeed100:
        timeScale = 1.0;
        break;
    case PlaybackSpeed200:
        timeScale = 2.0;
        break;
    case PlaybackSpeed400:
        timeScale = 4.0;
        break;
    case PlaybackSpeed800:
        timeScale = 8.0;
        break;
    case PlaybackSpeedCustom:
        timeScale = ui->customPlaybackSpeedLineEdit->text().toDouble() / 100.0;
        break;
    default:
        timeScale = 1.0;
        break;
    }

    m_skyConnect->setTimeScale(timeScale);
}

void MainWindow::toggleRecord(bool enable)
{
    this->blockSignals(true);

    switch (m_skyConnect->getState()) {
    case Connect::State::Recording:
        if (!enable) {
            m_skyConnect->stopDataSample();
        }
        break;
    case Connect::State::RecordingPaused:
        if (enable) {
            // The record button also unpauses a paused recording
            m_skyConnect->setPaused(false);
        }
        break;
    default:
        if (enable) {
            m_skyConnect->startDataSample();
        }
        break;
    }
    this->blockSignals(false);
}

void MainWindow::togglePause(bool enable)
{
    m_skyConnect->setPaused(enable);
}

void MainWindow::togglePlay(bool enable)
{
    if (enable) {
        if (m_skyConnect->isAtEnd()) {
            // Jump back to start
            m_skyConnect->setCurrentTimestamp(0);
        }
        m_skyConnect->startReplay(false);
    } else if (m_skyConnect->isPaused()) {
        // The play button also unpauses a paused replay
        m_skyConnect->setPaused(false);
    } else {
        m_skyConnect->stopReplay();
    }
}

void MainWindow::skipToBegin()
{
    m_skyConnect->skipToBegin();
}

void MainWindow::skipBackward()
{
    m_skyConnect->skipBackward();
}

void MainWindow::skipForward()
{
    m_skyConnect->skipForward();
}

void MainWindow::skipToEnd()
{
    m_skyConnect->skipToEnd();
}

void MainWindow::handleLastWindowClosed()
{
    // Destroying the settings singleton also persists the settings
    Settings::destroyInstance();
    SkyManager::destroyInstance();
}
