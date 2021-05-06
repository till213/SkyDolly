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
#include <memory>

#include <QApplication>
#include <QByteArray>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
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
#include <QIcon>
#include <QLocale>
#include <QStackedWidget>
#include <QEvent>
#include <QResizeEvent>
#include <QActionGroup>

#include "../../Kernel/src/Version.h"
#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/Enum.h"
#include "../../Kernel/src/SampleRate.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/AircraftData.h"
#include "../../Model/src/AircraftInfo.h"
#include "../../Model/src/Logbook.h"
#include "../../Persistence/src/Dao/DaoFactory.h"
#include "../../Persistence/src/Service/FlightService.h"
#include "../../Persistence/src/Service/DatabaseService.h"
#include "../../Persistence/src/Service/CSVService.h"
#include "../../SkyConnect/src/SkyManager.h"
#include "../../SkyConnect/src/SkyConnectIntf.h"
#include "../../SkyConnect/src/Connect.h"
#include "Dialogs/AboutDialog.h"
#include "Dialogs/AboutLibraryDialog.h"
#include "Dialogs/SettingsDialog.h"
#include "Dialogs/FlightDialog.h"
#include "Dialogs/SimulationVariablesDialog.h"
#include "Dialogs/StatisticsDialog.h"
#include "Widgets/ActionButton.h"
#include "Widgets/ActionRadioButton.h"
#include "Widgets/LogbookWidget.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"

namespace
{
    constexpr int PositionSliderMin = 0;
    constexpr int PositionSliderMax = 1000;
    constexpr double ReplaySpeedMin = 0.01;
    // A replay speed with factor 200 should be fast enough
    constexpr double ReplaySpeedMax = 200.0;
    constexpr double ReplaySpeedDecimalPlaces = 2;
    constexpr char TimestampFormat[] = "hh:mm:ss";
    constexpr qint64 MilliSecondsPerSecond = 1000;
    constexpr qint64 MilliSecondsPerMinute = 60 * MilliSecondsPerSecond;
    constexpr qint64 MilliSecondsPerHour = 60 * MilliSecondsPerMinute;

    enum class ReplaySpeed {
        Slow10,
        Slow25,
        Slow50,
        Slow75,
        Normal,
        Fast2x,
        Fast4x,
        Fast8x,
        Fast16x,
        Custom
    };
}

class MainWindowPrivate
{
public:
    MainWindowPrivate() noexcept
        : skyConnect(SkyManager::getInstance().currentSkyConnect()),
          previousState(Connect::State::Connected),
          replaySpeedButtonGroup(nullptr),
          aboutDialog(nullptr),
          aboutLibraryDialog(nullptr),
          settingsDialog(nullptr),
          flightDialog(nullptr),
          simulationVariablesDialog(nullptr),
          statisticsDialog(nullptr),
          flightService(std::make_unique<FlightService>()),
          databaseService(std::make_unique<DatabaseService>()),
          csvService(std::make_unique<CSVService>(*flightService)),
          showMinimalUi(false),
          customSpeedRadioButton(nullptr)
    {}

    SkyConnectIntf &skyConnect;
    Connect::State previousState;
    QButtonGroup *replaySpeedButtonGroup;
    AboutDialog *aboutDialog;
    AboutLibraryDialog *aboutLibraryDialog;
    SettingsDialog *settingsDialog;
    FlightDialog *flightDialog;
    SimulationVariablesDialog *simulationVariablesDialog;
    StatisticsDialog *statisticsDialog;
    double lastCustomReplaySpeed;
    QLocale locale;
    std::unique_ptr<FlightService> flightService;
    std::unique_ptr<DatabaseService> databaseService;
    std::unique_ptr<CSVService> csvService;
    bool showMinimalUi;
    QSize minimalUiSize;
    QSize lastNormalUiSize;
    ActionRadioButton *customSpeedRadioButton;
    QLineEdit *customSpeedLineEdit;
};

// PUBLIC

MainWindow::MainWindow(QWidget *parent) noexcept
    : QMainWindow(parent),
      ui(std::make_unique<Ui::MainWindow>()),
      d(std::make_unique<MainWindowPrivate>())
{
    ui->setupUi(this);
    connectWithDb();
    initUi();
    updateUi();
    frenchConnection();
}

MainWindow::~MainWindow() noexcept
{
    // The SkyConnect instances have been deleted by the SkyManager (singleton)
    // already at this point; no need to disconnect from their "stateChanged"
    // signal

    // Make sure that all widgets having a reference to the flight service
    // are deleted before this MainWindow instance (which owns the flight
    // service); we make sure by simply deleting their parent moduleStackWidget
    delete ui->moduleStackWidget;

#ifdef DEBUG
    qDebug("MainWindow::~MainWindow: DELETED");
#endif
}

// PROTECTED

bool MainWindow::event(QEvent *event)
{
    bool ret = QMainWindow::event(event);
    if (event->type() == QEvent::LayoutRequest && d->showMinimalUi && !d->minimalUiSize.isValid()) {
        adjustSize();
        d->minimalUiSize = size();
        setFixedSize(d->minimalUiSize);
    }
    return ret;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (!d->showMinimalUi) {
        d->lastNormalUiSize = event->size();
    }
}

// PRIVATE

void MainWindow::frenchConnection() noexcept
{
    connect(&d->skyConnect, &SkyConnectIntf::timestampChanged,
            this, &MainWindow::handleTimestampChanged);
    connect(&d->skyConnect, &SkyConnectIntf::stateChanged,
            this, &MainWindow::updateUi);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(d->replaySpeedButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::updateReplaySpeedUi);
    connect(d->replaySpeedButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::handleReplaySpeedSelected);
#else
    connect(d->replaySpeedButtonGroup, &QButtonGroup::idClicked,
            this, &MainWindow::updateReplaySpeedUi);
    connect(d->replaySpeedButtonGroup, &QButtonGroup::idClicked,
            this, &MainWindow::handleReplaySpeedSelected);
#endif

    // Ui elements
    connect(d->customSpeedLineEdit, &QLineEdit::editingFinished,
            this, &MainWindow::handleCustomSpeedChanged);

    // Actions
    connect(ui->recordAction, &QAction::triggered,
            this, &MainWindow::toggleRecord);
    connect(ui->stopAction, &QAction::triggered,
            this, &MainWindow::stop);
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

    // Dialogs
    connect(d->flightDialog, &FlightDialog::visibilityChanged,
            this, &MainWindow::updateWindowMenu);
    connect(d->simulationVariablesDialog, &SimulationVariablesDialog::visibilityChanged,
            this, &MainWindow::updateWindowMenu);
    connect(d->statisticsDialog, &StatisticsDialog::visibilityChanged,
            this, &MainWindow::updateWindowMenu);

    // Settings
    connect(&Settings::getInstance(), &Settings::changed,
            this, &MainWindow::updateMainWindow);

    // Service
    connect(d->flightService.get(), &FlightService::flightRestored,
            this, &MainWindow::handleFlightRestored);
    connect(&d->skyConnect, &SkyConnectIntf::recordingStopped,
            this, &MainWindow::handleRecordingStopped);
}

void MainWindow::initUi() noexcept
{
    setWindowIcon(QIcon(":/img/icons/application-icon.png"));

    // Dialogs
    d->flightDialog = new FlightDialog(d->skyConnect, this);
    d->simulationVariablesDialog = new SimulationVariablesDialog(d->skyConnect, this);
    d->statisticsDialog = new StatisticsDialog(d->skyConnect, this);
    d->aboutDialog = new AboutDialog(this);
    d->aboutLibraryDialog = new AboutLibraryDialog(*d->databaseService, this);
    d->settingsDialog = new SettingsDialog(this);

    // Widgets
    LogbookWidget *logbookWidget = new LogbookWidget(*d->flightService, ui->moduleStackWidget);
    ui->moduleGroupBox->setTitle(logbookWidget->getTitle());
    ui->moduleStackWidget->addWidget(logbookWidget);
    ui->moduleStackWidget->setCurrentWidget(logbookWidget);

    ui->stayOnTopAction->setChecked(Settings::getInstance().isWindowStaysOnTopEnabled());
    ui->showMinimalAction->setChecked(ui->moduleGroupBox->isVisible());
    initControlUi();
    initReplaySpeedUi();
    ui->showMinimalAction->setChecked(d->showMinimalUi);
    on_showMinimalAction_triggered(d->showMinimalUi);
}

void MainWindow::initControlUi() noexcept
{

    ui->positionSlider->setRange(PositionSliderMin, PositionSliderMax);
    ui->timestampTimeEdit->setDisplayFormat(TimestampFormat);

    // Record/replay control buttons
    ActionButton *recordButton = new ActionButton(this);
    recordButton->setAction(ui->recordAction);
    recordButton->setFlat(true);
    ui->controlButtonLayout->insertWidget(0, recordButton);

    ActionButton *skipToStartButton = new ActionButton(this);
    skipToStartButton->setAction(ui->skipToBeginAction);
    skipToStartButton->setFlat(true);
    ui->controlButtonLayout->addWidget(skipToStartButton);

    ActionButton *skipBackwardButton = new ActionButton(this);
    skipBackwardButton->setAction(ui->backwardAction);
    skipBackwardButton->setFlat(true);
    skipBackwardButton->setAutoRepeat(true);
    ui->controlButtonLayout->addWidget(skipBackwardButton);

    ActionButton *stopButton = new ActionButton(this);
    stopButton->setAction(ui->stopAction);
    stopButton->setFlat(true);
    ui->controlButtonLayout->addWidget(stopButton);

    ActionButton *pauseButton = new ActionButton(this);
    pauseButton->setAction(ui->pauseAction);
    pauseButton->setFlat(true);
    ui->controlButtonLayout->addWidget(pauseButton);

    ActionButton *playButton = new ActionButton(this);
    playButton->setAction(ui->playAction);
    playButton->setFlat(true);
    ui->controlButtonLayout->addWidget(playButton);

    ActionButton *skipForwardButton = new ActionButton(this);
    skipForwardButton->setAction(ui->forwardAction);
    skipForwardButton->setFlat(true);
    skipForwardButton->setAutoRepeat(true);
    ui->controlButtonLayout->addWidget(skipForwardButton);

    ActionButton *skipToEndButton = new ActionButton(this);
    skipToEndButton->setAction(ui->skipToEndAction);
    skipToEndButton->setFlat(true);
    ui->controlButtonLayout->addWidget(skipToEndButton);
}

void MainWindow::initReplaySpeedUi() noexcept
{
    // Actions
    QList<QAction *> slowActions {
        new QAction("10 %", this),
        new QAction("25 %", this),
        new QAction("50 %", this),
        new QAction("75 %", this)
    };
    QAction *normalSpeedAction = new QAction("1x", this);
    normalSpeedAction->setCheckable(true);
    QList<QAction *> fastActions {
        new QAction("2x", this),
        new QAction("4x", this),
        new QAction("8x", this),
        new QAction("16x", this)
    };
    QAction *customSpeedAction = new QAction(tr("Custom"), this);
    customSpeedAction->setCheckable(true);

    // Action group
    QActionGroup *replaySpeedGroup = new QActionGroup(this);
    for (QAction *action : slowActions) {
        action->setCheckable(true);
        replaySpeedGroup->addAction(action);
    }
    replaySpeedGroup->addAction(normalSpeedAction);
    for (QAction *action : fastActions) {
        action->setCheckable(true);
        replaySpeedGroup->addAction(action);
    }
    replaySpeedGroup->addAction(customSpeedAction);


    // Menus
    ui->slowMenu->addActions(slowActions);
    ui->fastMenu->addActions(fastActions);

    // Action radio buttons
    ActionRadioButton *slow10RadioButton = new ActionRadioButton(this);
    slow10RadioButton->setAction(slowActions.at(0));
    ui->replaySpeedGroupBox->layout()->addWidget(slow10RadioButton);

    ActionRadioButton *slow25RadioButton = new ActionRadioButton(this);
    slow25RadioButton->setAction(slowActions.at(1));
    ui->replaySpeedGroupBox->layout()->addWidget(slow25RadioButton);

    ActionRadioButton *slow50RadioButton = new ActionRadioButton(this);
    slow50RadioButton->setAction(slowActions.at(2));
    ui->replaySpeedGroupBox->layout()->addWidget(slow50RadioButton);

    ActionRadioButton *slow75RadioButton = new ActionRadioButton(this);
    slow75RadioButton->setAction(slowActions.at(3));
    ui->replaySpeedGroupBox->layout()->addWidget(slow75RadioButton);

    ActionRadioButton *normalSpeedRadioButton = new ActionRadioButton(this);
    normalSpeedRadioButton->setAction(normalSpeedAction);
    ui->replaySpeedGroupBox->layout()->addWidget(normalSpeedRadioButton);

    ActionRadioButton *fast2xRadioButton = new ActionRadioButton(this);
    fast2xRadioButton->setAction(fastActions.at(0));
    ui->replaySpeedGroupBox->layout()->addWidget(fast2xRadioButton);

    ActionRadioButton *fast4xRadioButton = new ActionRadioButton(this);
    fast4xRadioButton->setAction(fastActions.at(1));
    ui->replaySpeedGroupBox->layout()->addWidget(fast4xRadioButton);

    ActionRadioButton *fast8xRadioButton = new ActionRadioButton(this);
    fast8xRadioButton->setAction(fastActions.at(2));
    ui->replaySpeedGroupBox->layout()->addWidget(fast8xRadioButton);

    ActionRadioButton *fast16xRadioButton = new ActionRadioButton(this);
    fast16xRadioButton->setAction(fastActions.at(3));
    ui->replaySpeedGroupBox->layout()->addWidget(fast16xRadioButton);

    d->customSpeedRadioButton = new ActionRadioButton(this);
    d->customSpeedRadioButton->setAction(customSpeedAction);
    ui->replaySpeedGroupBox->layout()->addWidget(d->customSpeedRadioButton);

    d->customSpeedLineEdit = new QLineEdit(this);
    d->customSpeedLineEdit->setMinimumWidth(160);
    ui->replaySpeedGroupBox->layout()->addWidget(d->customSpeedLineEdit);

    d->replaySpeedButtonGroup = new QButtonGroup(this);
    d->replaySpeedButtonGroup->addButton(slow10RadioButton, Enum::toUnderlyingType(ReplaySpeed::Slow10));
    d->replaySpeedButtonGroup->addButton(slow25RadioButton, Enum::toUnderlyingType(ReplaySpeed::Slow25));
    d->replaySpeedButtonGroup->addButton(slow25RadioButton, Enum::toUnderlyingType(ReplaySpeed::Slow25));
    d->replaySpeedButtonGroup->addButton(slow50RadioButton, Enum::toUnderlyingType(ReplaySpeed::Slow50));
    d->replaySpeedButtonGroup->addButton(slow75RadioButton, Enum::toUnderlyingType(ReplaySpeed::Slow75));
    d->replaySpeedButtonGroup->addButton(normalSpeedRadioButton, Enum::toUnderlyingType(ReplaySpeed::Normal));
    d->replaySpeedButtonGroup->addButton(fast2xRadioButton, Enum::toUnderlyingType(ReplaySpeed::Fast2x));
    d->replaySpeedButtonGroup->addButton(fast4xRadioButton, Enum::toUnderlyingType(ReplaySpeed::Fast4x));
    d->replaySpeedButtonGroup->addButton(fast8xRadioButton, Enum::toUnderlyingType(ReplaySpeed::Fast8x));
    d->replaySpeedButtonGroup->addButton(fast16xRadioButton, Enum::toUnderlyingType(ReplaySpeed::Fast16x));
    d->replaySpeedButtonGroup->addButton(d->customSpeedRadioButton , Enum::toUnderlyingType(ReplaySpeed::Custom));


    QDoubleValidator *customReplaySpeedValidator = new QDoubleValidator(d->customSpeedLineEdit);
    d->customSpeedLineEdit->setValidator(customReplaySpeedValidator);
    customReplaySpeedValidator->setRange(ReplaySpeedMin, ReplaySpeedMax, ReplaySpeedDecimalPlaces);

    const double replaySpeed = d->skyConnect.getTimeScale();
    d->lastCustomReplaySpeed = replaySpeed;
    if (qFuzzyCompare(d->skyConnect.getTimeScale(), 1.0)) {
        normalSpeedRadioButton->setChecked(true);
    } else {
        d->customSpeedRadioButton ->setChecked(true);
        d->customSpeedLineEdit->setText(d->locale.toString(replaySpeed, 'f', ReplaySpeedDecimalPlaces));
    }
    d->customSpeedLineEdit->setToolTip(tr("Custom replay factor in [%L1, %L2]").arg(ReplaySpeedMin).arg(ReplaySpeedMax));

}

bool MainWindow::connectWithDb() noexcept
{
    QString filePath = Settings::getInstance().getLibraryPath();
    bool ok;
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getSaveFileName(this, tr("Library"), ".", QString("*") + DatabaseService::LibraryExtension);
        Settings::getInstance().setLibraryPath(filePath);
    }
    if (!filePath.isEmpty()) {
        ok = d->databaseService->connectDb();
    } else {
        ok = false;
    }
    return ok;
}

// PRIVATE SLOTS

void MainWindow::on_positionSlider_sliderPressed() noexcept
{
    d->previousState = d->skyConnect.getState();
    if (d->previousState == Connect::State::Replay) {
        // Pause the replay while sliding the position slider
        d->skyConnect.setPaused(true);
    }
}

void MainWindow::on_positionSlider_valueChanged(int value) noexcept
{
    const double scale = static_cast<double>(value) / static_cast<double>(PositionSliderMax);
    const qint64 totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
    const qint64 timestamp = static_cast<qint64>(qRound(scale * static_cast<double>(totalDuration)));

    // Prevent the timestampTimeEdit field to set the play position as well
    ui->timestampTimeEdit->blockSignals(true);
    d->skyConnect.seek(timestamp);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::on_positionSlider_sliderReleased() noexcept
{
    if (d->previousState == Connect::State::Replay) {
        d->skyConnect.setPaused(false);
    }
}

void MainWindow::on_timestampTimeEdit_timeChanged(const QTime &time) noexcept
{
    const Connect::State state = d->skyConnect.getState();
    if (state == Connect::State::Connected || state == Connect::State::ReplayPaused) {
        qint64 timestamp = time.hour() * MilliSecondsPerHour + time.minute() * MilliSecondsPerMinute + time.second() * MilliSecondsPerSecond;
        d->skyConnect.seek(timestamp);
    }
}

void MainWindow::handleCustomSpeedChanged() noexcept
{
    const QString text = d->customSpeedLineEdit->text();
    if (!text.isEmpty()) {
        d->lastCustomReplaySpeed = d->locale.toDouble(text);
        d->skyConnect.setTimeScale(d->lastCustomReplaySpeed);
    }
}

void MainWindow::updateUi() noexcept
{
    updateControlUi();
    updateReplaySpeedUi();
    updateTimestamp();
    updateFileMenu();
    updateWindowMenu();
    updateMainWindow();
}

void MainWindow::updateControlUi() noexcept
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraftConst();
    const bool hasRecording = aircraft.hasRecording();
    switch (d->skyConnect.getState()) {
    case Connect::State::Disconnected:
        // Fall-thru intened: each time a control element is triggered a connection
        // attempt is made, so we enable the same elements as in connected state
    case Connect::State::Connected:
        // Actions
        ui->recordAction->setEnabled(true);
        ui->recordAction->setChecked(false);
        ui->stopAction->setEnabled(false);
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
    case Connect::State::Recording:
        // Actions
        ui->recordAction->setEnabled(true);
        ui->recordAction->setChecked(true);
        ui->stopAction->setEnabled(true);
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
    case Connect::State::RecordingPaused:
        // Actions
        ui->recordAction->setChecked(false);
        ui->pauseAction->setChecked(true);
        break;
    case Connect::State::Replay:
        // Actions
        ui->recordAction->setEnabled(false);
        ui->recordAction->setChecked(false);
        ui->stopAction->setEnabled(true);
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
    case Connect::State::ReplayPaused:
        // Actions
        ui->pauseAction->setChecked(true);
        ui->playAction->setChecked(false);
        ui->timestampTimeEdit->setEnabled(true);
        break;
    default:
        break;
    }
}

void MainWindow::updateReplaySpeedUi() noexcept
{
    if (d->customSpeedRadioButton->isChecked()) {
        d->customSpeedLineEdit->setEnabled(true);
        d->customSpeedLineEdit->setText(d->locale.toString(d->lastCustomReplaySpeed, 'f', ReplaySpeedDecimalPlaces));
    } else {
        d->customSpeedLineEdit->setEnabled(false);
        d->customSpeedLineEdit->clear();
    }
}

void MainWindow::updateTimestamp() noexcept
{
    const qint64 totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
    ui->timestampTimeEdit->blockSignals(true);
    QTime time(0, 0, 0, 0);
    time = time.addMSecs(totalDuration);
    ui->timestampTimeEdit->setTime(time);
    ui->timestampTimeEdit->setMaximumTime(time);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::updateFileMenu() noexcept
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraftConst();
    const bool hasRecording = aircraft.hasRecording();
    switch (d->skyConnect.getState()) {
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

void MainWindow::updateWindowMenu() noexcept
{
    ui->showFlightAction->setChecked(d->flightDialog->isVisible());
    ui->showSimulationVariablesAction->setChecked(d->simulationVariablesDialog->isVisible());
    ui->showStatisticsAction->setChecked(d->statisticsDialog->isVisible());
}

void MainWindow::updateMainWindow() noexcept
{
    const Settings &settings = Settings::getInstance();
    const Qt::WindowFlags flags = windowFlags();
    if (settings.isWindowStaysOnTopEnabled() != (flags & Qt::WindowStaysOnTopHint)) {
        if (Settings::getInstance().isWindowStaysOnTopEnabled()) {
            this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
            this->show();
        } else {
            this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
            this->show();
        }
    }

    if (settings.getRecordSampleRate() != SampleRate::SampleRate::Auto) {
        ui->recordAction->setToolTip(tr("Record [@%1 Hz]").arg(Settings::getInstance().getRecordSampleRateValue()));
    } else {
        ui->recordAction->setToolTip(tr("Record [auto sample rate]"));
    }

    if (settings.isAbsoluteSeekEnabled()) {
        double seekIntervalSeconds = settings.getSeekIntervalSeconds();
        ui->forwardAction->setToolTip(tr("Fast forward [%1 sec]").arg(seekIntervalSeconds));
        ui->backwardAction->setToolTip(tr("Rewind [%1 sec]").arg(seekIntervalSeconds));
    } else {
        double seekIntervalPercent = settings.getSeekIntervalPercent();
        ui->forwardAction->setToolTip(tr("Fast forward [%1 %]").arg(seekIntervalPercent));
        ui->backwardAction->setToolTip(tr("Rewind [%1 %]").arg(seekIntervalPercent));
    }
}

void MainWindow::on_newLibraryAction_triggered() noexcept
{
    Settings &settings = Settings::getInstance();
    QString existingLibraryPath = QFileInfo(settings.getLibraryPath()).absolutePath();
    bool retry = true;
    while (retry) {
        QString libraryPath = QFileDialog::getSaveFileName(this, tr("New library"), existingLibraryPath, QString("*") + DatabaseService::LibraryExtension);
        if (!libraryPath.isEmpty()) {
            if (!QFileInfo::exists(libraryPath)) {
                settings.setLibraryPath(libraryPath);
                bool ok = d->databaseService->connectDb();
                if (!ok) {
                    QMessageBox::critical(this, tr("Database error"), tr("The library %1 could not be created.").arg(libraryPath));
                }
                retry = false;
            } else {
                QMessageBox::information(this, tr("Database exists"), tr("The library %1 already exists. Please choose another path.").arg(libraryPath));
            }
        } else {
            retry = false;
        }
    }
}

void MainWindow::on_openLibraryAction_triggered() noexcept
{
    Settings &settings = Settings::getInstance();
    QString existingLibraryPath = QFileInfo(settings.getLibraryPath()).absolutePath();
    QString libraryPath = QFileDialog::getOpenFileName(this, tr("Open library"), existingLibraryPath, QString("*") + DatabaseService::LibraryExtension);
    if (!libraryPath.isEmpty()) {
        settings.setLibraryPath(libraryPath);
        bool ok = d->databaseService->connectDb();
        if (!ok) {
            QMessageBox::critical(this, tr("Database error"), tr("The library %1 could not be opened.").arg(libraryPath));
        }
    }
}

void MainWindow::on_backupLibraryAction_triggered() noexcept
{
    bool ok = d->databaseService->backup();
    if (!ok) {
        QMessageBox::critical(this, tr("Database error"), tr("The library backup could not be created."));
    }
}

void MainWindow::on_optimiseLibraryAction_triggered() noexcept
{
    bool ok = d->databaseService->optimise();
    if (!ok) {
        QMessageBox::critical(this, tr("Database error"), tr("The library could not be optimised."));
    }
}

void MainWindow::on_importCSVAction_triggered() noexcept
{
    const QMessageBox message;
    const Version version;

    QMessageBox::StandardButton reply;
    int previewInfoCount = Settings::getInstance().getPreviewInfoDialogCount();
    if (previewInfoCount > 0) {
        --previewInfoCount;
        reply = QMessageBox::question(this, "Preview",
            QString("%1 %2 is an early preview version. The format of the exported CSV values in this version has changed compared with the previous version 0.4.2, "
                    "making the data invalid. Upcoming preview versions may still change the format in an incompatible way yet again.\n\n"
                    "This dialog will be shown %3 more times.").arg(Version::getApplicationName(), Version::getApplicationVersion()).arg(previewInfoCount),
            QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Abort);
        Settings::getInstance().setPreviewInfoDialogCount(previewInfoCount);
    } else {
        reply = QMessageBox::StandardButton::Ok;
    }

    if (reply == QMessageBox::StandardButton::Ok) {
        QString exportPath = Settings::getInstance().getExportPath();
        const QString filePath = QFileDialog::getOpenFileName(this, tr("Import CSV"), exportPath, QString("*.csv"));
        if (!filePath.isEmpty()) {

            bool ok = d->csvService->importAircraft(filePath);
            if (ok) {
                updateUi();
                d->skyConnect.skipToBegin();
                if (d->skyConnect.isConnected()) {
                    d->skyConnect.startReplay(true);
                    d->skyConnect.setPaused(true);
                }
                exportPath = QFileInfo(filePath).absolutePath();
                Settings::getInstance().setExportPath(exportPath);
            } else {
                QMessageBox::critical(this, tr("Import error"), tr("The CSV file %1 could not be read.").arg(filePath));
            }
        }
    }
}

void MainWindow::on_exportCSVAction_triggered() noexcept
{
    QString exportPath = Settings::getInstance().getExportPath();
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Export CSV"), exportPath, QString("*.csv"));
    if (!filePath.isEmpty()) {
        const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraftConst();
        bool ok = d->csvService->exportAircraft(aircraft, filePath);
        if (ok) {
            exportPath = QFileInfo(filePath).absolutePath();
            Settings::getInstance().setExportPath(exportPath);
        } else {
            QMessageBox::critical(this, tr("Export error"), tr("The CSV file %1 could not be written.").arg(filePath));
        }
    }
}

void MainWindow::on_showSettingsAction_triggered() noexcept
{
    d->settingsDialog->exec();
}

void MainWindow::on_quitAction_triggered() noexcept
{
    QApplication::quit();
}

void MainWindow::on_showFlightAction_triggered(bool enabled) noexcept
{
    if (enabled) {
        d->flightDialog->show();
    } else {
        d->flightDialog->close();
    }
}

void MainWindow::on_showSimulationVariablesAction_triggered(bool enabled) noexcept
{
    if (enabled) {
        d->simulationVariablesDialog->show();
    } else {
        d->simulationVariablesDialog->close();
    }
}

void MainWindow::on_showStatisticsAction_triggered(bool enabled) noexcept
{
    if (enabled) {
        d->statisticsDialog->show();
    } else {
        d->statisticsDialog->close();
    }
}

void MainWindow::on_stayOnTopAction_triggered(bool enabled) noexcept
{
    Settings::getInstance().setWindowStaysOnTopEnabled(enabled);
}

void MainWindow::on_showMinimalAction_triggered(bool enabled) noexcept
{
    d->showMinimalUi = enabled;
    ui->moduleGroupBox->setHidden(enabled);
    if (enabled) {
        if (d->minimalUiSize.isValid()) {
            setFixedSize(d->minimalUiSize);
        }
    } else {
        QSize size = d->lastNormalUiSize;
        setMinimumSize(QSize(0, 0));
        setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
        resize(size);
    }
}

void MainWindow::on_aboutLibraryAction_triggered() noexcept
{
    d->aboutLibraryDialog->exec();
}

void MainWindow::on_aboutAction_triggered() noexcept
{
    d->aboutDialog->exec();
}

void MainWindow::on_aboutQtAction_triggered() noexcept
{
    QMessageBox::aboutQt(this);
}

void MainWindow::handleTimestampChanged(qint64 timestamp) noexcept
{
    if (d->skyConnect.isRecording()) {
        updateTimestamp();
    } else {
        const qint64 totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
        const qint64 ts = qMin(timestamp, totalDuration);

        int sliderPosition;
        if (totalDuration > 0) {
            sliderPosition = qRound(PositionSliderMax * (static_cast<double>(ts) / static_cast<double>(totalDuration)));
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
    };
}

void MainWindow::handleReplaySpeedSelected(int selection) noexcept
{
    double timeScale;
    switch (static_cast<ReplaySpeed>(selection)) {
    case ReplaySpeed::Slow10:
        timeScale = 0.1;
        break;
    case ReplaySpeed::Slow25:
        timeScale = 0.25;
        break;
    case ReplaySpeed::Slow50:
        timeScale = 0.5;
        break;
    case ReplaySpeed::Slow75:
        timeScale = 0.75;
        break;
    case ReplaySpeed::Normal:
        timeScale = 1.0;
        break;
    case ReplaySpeed::Fast2x:
        timeScale = 2.0;
        break;
    case ReplaySpeed::Fast4x:
        timeScale = 4.0;
        break;
    case ReplaySpeed::Fast8x:
        timeScale = 8.0;
        break;
    case ReplaySpeed::Fast16x:
        timeScale = 16.0;
        break;
    case ReplaySpeed::Custom:
        timeScale = d->locale.toDouble(d->customSpeedLineEdit->text());
        break;
    default:
        timeScale = 1.0;
        break;
    }

    d->skyConnect.setTimeScale(timeScale);
}

void MainWindow::toggleRecord(bool enable) noexcept
{
    this->blockSignals(true);
    switch (d->skyConnect.getState()) {
    case Connect::State::Recording:
        if (!enable) {
            d->skyConnect.stopRecording();
        }
        break;
    case Connect::State::RecordingPaused:
        if (enable) {
            // The record button also unpauses a paused recording
            d->skyConnect.setPaused(false);
        }
        break;
    default:
        if (enable) {
            d->skyConnect.startRecording();
        }
        break;
    }
    this->blockSignals(false);
}

void MainWindow::togglePause(bool enable) noexcept
{
    d->skyConnect.setPaused(enable);
}

void MainWindow::togglePlay(bool enable) noexcept
{
    if (enable) {
        d->skyConnect.startReplay(d->skyConnect.isAtEnd());
    } else if (d->skyConnect.isPaused()) {
        // The play button also unpauses a paused replay
        d->skyConnect.setPaused(false);
    } else {
        d->skyConnect.stopReplay();
    }
}

void MainWindow::stop() noexcept
{
    d->skyConnect.stop();
}

void MainWindow::skipToBegin() noexcept
{
    d->skyConnect.skipToBegin();
}

void MainWindow::skipBackward() noexcept
{
    d->skyConnect.skipBackward();
}

void MainWindow::skipForward() noexcept
{
    d->skyConnect.skipForward();
}

void MainWindow::skipToEnd() noexcept
{
    d->skyConnect.skipToEnd();
}

void MainWindow::handleFlightRestored() noexcept
{
    updateUi();
    d->skyConnect.skipToBegin();
    if (d->skyConnect.isConnected()) {
        d->skyConnect.startReplay(true);
        d->skyConnect.setPaused(true);
    }
}

void MainWindow::handleRecordingStopped() noexcept
{
    d->flightService->store(Logbook::getInstance().getCurrentFlight());
}
