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
#include <vector>

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
#include <QDoubleValidator>
#include <QIcon>
#include <QLocale>
#include <QStackedWidget>
#include <QEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QAction>
#include <QActionGroup>
#include <QSpacerItem>
#include <QTimer>

#include "../../Kernel/src/Const.h"
#include "../../Kernel/src/Replay.h"
#include "../../Kernel/src/Version.h"
#include "../../Kernel/src/Settings.h"
#include "../../Kernel/src/Enum.h"
#include "../../Kernel/src/SampleRate.h"
#include "../../Model/src/Aircraft.h"
#include "../../Model/src/PositionData.h"
#include "../../Model/src/AircraftInfo.h"
#include "../../Model/src/Logbook.h"
#include "../../Persistence/src/Dao/DaoFactory.h"
#include "../../Persistence/src/Service/FlightService.h"
#include "../../Persistence/src/Service/DatabaseService.h"
#include "../../Persistence/src/Service/CSVService.h"
#include "../../SkyConnect/src/SkyManager.h"
#include "../../SkyConnect/src/SkyConnectIntf.h"
#include "../../SkyConnect/src/Connect.h"
#include "../../Module/src/Logbook/LogbookWidget.h"
#include "../../Module/src/ModuleManager.h"
#include "Dialogs/AboutDialog.h"
#include "Dialogs/AboutLogbookDialog.h"
#include "Dialogs/SettingsDialog.h"
#include "Dialogs/FlightDialog.h"
#include "Dialogs/SimulationVariablesDialog.h"
#include "Dialogs/StatisticsDialog.h"
#include "Widgets/ActionButton.h"
#include "Widgets/ActionRadioButton.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"

namespace
{
    constexpr int PositionSliderMin = 0;
    constexpr int PositionSliderMax = 1000;
    constexpr double ReplaySpeedAbsoluteMin = 0.01;
    // A replay speed with factor 200 should be fast enough
    constexpr double ReplaySpeedAbsoluteMax = 200.0;
    constexpr double ReplaySpeedDecimalPlaces = 2;

    constexpr char TimestampFormat[] = "hh:mm:ss";
    constexpr qint64 MilliSecondsPerSecond = 1000;
    constexpr qint64 MilliSecondsPerMinute = 60 * MilliSecondsPerSecond;
    constexpr qint64 MilliSecondsPerHour = 60 * MilliSecondsPerMinute;

    constexpr char ReplaySpeedProperty[] = "ReplaySpeed";

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
        : skyConnect(SkyManager::getInstance().getCurrentSkyConnect()),
          previousState(Connect::State::Connected),
          connectedWithLogbook(false),
          aboutDialog(nullptr),
          aboutLogbookDialog(nullptr),
          settingsDialog(nullptr),
          flightDialog(nullptr),
          simulationVariablesDialog(nullptr),
          statisticsDialog(nullptr),
          flightService(std::make_unique<FlightService>()),
          databaseService(std::make_unique<DatabaseService>()),
          csvService(std::make_unique<CSVService>(*flightService)),
          replaySpeedActionGroup(nullptr),
          customSpeedRadioButton(nullptr),
          customSpeedLineEdit(nullptr),
          replaySpeedUnitComboBox(nullptr),
          customReplaySpeedFactorValidator(nullptr),
          customReplaySpeedPercentValidator(nullptr),
          moduleManager(nullptr),
          activeModuleId(Module::Module::None),
          showModuleSelector(true)
    {}

    SkyConnectIntf &skyConnect;
    Connect::State previousState;
    bool connectedWithLogbook;

    AboutDialog *aboutDialog;
    AboutLogbookDialog *aboutLogbookDialog;
    SettingsDialog *settingsDialog;
    FlightDialog *flightDialog;
    SimulationVariablesDialog *simulationVariablesDialog;
    StatisticsDialog *statisticsDialog;    

    QLocale locale;

    // Services
    std::unique_ptr<FlightService> flightService;
    std::unique_ptr<DatabaseService> databaseService;
    std::unique_ptr<CSVService> csvService;

    QSize lastNormalUiSize;

    QActionGroup *replaySpeedActionGroup ;
    ActionRadioButton *customSpeedRadioButton;
    double lastCustomReplaySpeed;
    QLineEdit *customSpeedLineEdit;
    QComboBox *replaySpeedUnitComboBox;

    QDoubleValidator *customReplaySpeedFactorValidator;
    QDoubleValidator *customReplaySpeedPercentValidator;

    std::unique_ptr<ModuleManager> moduleManager;
    Module::Module activeModuleId;
    bool showModuleSelector;
};

// PUBLIC

MainWindow::MainWindow(QWidget *parent) noexcept
    : QMainWindow(parent),
      ui(std::make_unique<Ui::MainWindow>()),
      d(std::make_unique<MainWindowPrivate>())
{
    ui->setupUi(this);
    const QString logbookPath = Settings::getInstance().getLogbookPath();
    d->connectedWithLogbook = d->databaseService->connectWithLogbook(logbookPath, this);
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

void MainWindow::resizeEvent(QResizeEvent *event) noexcept
{
    if (!Settings::getInstance().isMinimalUiEnabled()) {
        d->lastNormalUiSize = event->size();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) noexcept
{
    QMainWindow::closeEvent(event);
    Settings &settings = Settings::getInstance();
    settings.setWindowGeometry(saveGeometry());
    settings.setWindowState(saveState());
}

// PRIVATE

void MainWindow::frenchConnection() noexcept
{
    connect(&d->skyConnect, &SkyConnectIntf::timestampChanged,
            this, &MainWindow::handleTimestampChanged);
    connect(&d->skyConnect, &SkyConnectIntf::stateChanged,
            this, &MainWindow::updateUi);

    connect(d->replaySpeedActionGroup, &QActionGroup::triggered,
            this, &MainWindow::updateReplaySpeedUi);
    connect(d->replaySpeedActionGroup, &QActionGroup::triggered,
            this, &MainWindow::handleReplaySpeedSelected);    

    // Ui elements
    connect(d->customSpeedLineEdit, &QLineEdit::editingFinished,
            this, &MainWindow::handleCustomSpeedChanged);
    connect(d->replaySpeedUnitComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::handleReplaySpeedUnitSelected);

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

    // Modules
    connect(d->moduleManager.get(), &ModuleManager::activated,
            this, &MainWindow::handleModuleActivated);

    // Settings
    connect(&Settings::getInstance(), &Settings::changed,
            this, &MainWindow::updateMainWindow);

    // Service
    connect(d->flightService.get(), &FlightService::flightRestored,
            this, &MainWindow::handleFlightRestored);
    connect(d->databaseService.get(), &DatabaseService::logbookConnectionChanged,
            this, &MainWindow::handleLogbookConnectionChanged);
}

void MainWindow::initUi() noexcept
{
    setWindowIcon(QIcon(":/img/icons/application-icon.png"));

    // Dialogs
    d->flightDialog = new FlightDialog(*d->flightService, this);
    d->simulationVariablesDialog = new SimulationVariablesDialog(this);
    d->statisticsDialog = new StatisticsDialog(this);
    d->aboutDialog = new AboutDialog(this);
    d->aboutLogbookDialog = new AboutLogbookDialog(*d->databaseService, this);
    d->settingsDialog = new SettingsDialog(this);

    // Modules
    d->moduleManager = std::make_unique<ModuleManager>(*ui->moduleStackWidget, *d->databaseService, *d->flightService);

    const ModuleIntf &activeModule = d->moduleManager->getActiveModule();
    ui->moduleGroupBox->setTitle(activeModule.getModuleName());
    ui->moduleSelectorVisibleCheckBox->setChecked(d->showModuleSelector);
    d->activeModuleId = activeModule.getModuleId();

    ui->stayOnTopAction->setChecked(Settings::getInstance().isWindowStaysOnTopEnabled());
    ui->showMinimalAction->setChecked(d->showModuleSelector);

    initModuleSelectorUi();
    initControlUi();
    initReplaySpeedUi();

    const bool minimalUi = Settings::getInstance().isMinimalUiEnabled();
    ui->showMinimalAction->setChecked(minimalUi);
    updateMinimalUi(minimalUi);

    Settings &settings = Settings::getInstance();
    QByteArray windowGeometry = settings.getWindowGeometry();
    QByteArray windowState = settings.getWindowState();
    if (!windowGeometry.isEmpty()) {
        restoreGeometry(windowGeometry);
        restoreState(windowState);
    }
}

void MainWindow::initModuleSelectorUi() noexcept
{
    for (const auto &item : d->moduleManager->getModules()) {
        QAction &moduleAction = item->getAction();
        ui->moduleMenu->addAction(&moduleAction);
        ActionButton *actionButton = new ActionButton(this);
        actionButton->setAction(&moduleAction);
        actionButton->setFlat(true);
        actionButton->setText(moduleAction.text().toUpper());
        ui->moduleSelectorLayout->addWidget(actionButton);
    }
}

void MainWindow::initControlUi() noexcept
{
    ui->positionSlider->setRange(PositionSliderMin, PositionSliderMax);
    ui->timestampTimeEdit->setDisplayFormat(TimestampFormat);

    // Record/replay control buttons
    ActionButton *recordButton = new ActionButton(this);
    recordButton->setAction(ui->recordAction);
    recordButton->setFlat(true);
    ui->controlButtonLayout->addWidget(recordButton);

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

    QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->controlButtonLayout->addSpacerItem(horizontalSpacer);
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
    slowActions.at(0)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
    slowActions.at(0)->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Slow10));
    slowActions.at(1)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
    slowActions.at(1)->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Slow25));
    slowActions.at(2)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
    slowActions.at(2)->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Slow50));
    slowActions.at(3)->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_4));
    slowActions.at(3)->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Slow75));

    QAction *normalSpeedAction = new QAction(tr("Normal"), this);
    normalSpeedAction->setCheckable(true);
    normalSpeedAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_1));
    normalSpeedAction->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Normal));

    QList<QAction *> fastActions {
        new QAction("2x", this),
        new QAction("4x", this),
        new QAction("8x", this),
        new QAction("16x", this)
    };
    fastActions.at(0)->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_2));
    fastActions.at(0)->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Fast2x));
    fastActions.at(1)->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_3));
    fastActions.at(1)->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Fast4x));
    fastActions.at(2)->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_4));
    fastActions.at(2)->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Fast8x));
    fastActions.at(3)->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_5));
    fastActions.at(3)->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Fast16x));

    QAction *customSpeedAction = new QAction(tr("Custom"), this);
    customSpeedAction->setCheckable(true);
    customSpeedAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_6));
    customSpeedAction->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Custom));

    // Action group
    d->replaySpeedActionGroup = new QActionGroup(this);
    for (QAction *action : slowActions) {
        action->setCheckable(true);
        d->replaySpeedActionGroup->addAction(action);
    }
    d->replaySpeedActionGroup->addAction(normalSpeedAction);
    for (QAction *action : fastActions) {
        action->setCheckable(true);
        d->replaySpeedActionGroup->addAction(action);
    }
    d->replaySpeedActionGroup->addAction(customSpeedAction);

    // Menus
    ui->slowMenu->addActions(slowActions);
    ui->fastMenu->addActions(fastActions);
    ui->replayMenu->addAction(normalSpeedAction);
    ui->replayMenu->addAction(customSpeedAction);

    QLayout *replaySpeedLayout = ui->replaySpeedGroupBox->layout();

    // Action radio buttons
    ActionRadioButton *slow10RadioButton = new ActionRadioButton(this);
    slow10RadioButton->setAction(slowActions.at(0));
    replaySpeedLayout->addWidget(slow10RadioButton);

    ActionRadioButton *slow25RadioButton = new ActionRadioButton(this);
    slow25RadioButton->setAction(slowActions.at(1));
    replaySpeedLayout->addWidget(slow25RadioButton);

    ActionRadioButton *slow50RadioButton = new ActionRadioButton(this);
    slow50RadioButton->setAction(slowActions.at(2));
    replaySpeedLayout->addWidget(slow50RadioButton);

    ActionRadioButton *slow75RadioButton = new ActionRadioButton(this);
    slow75RadioButton->setAction(slowActions.at(3));
    replaySpeedLayout->addWidget(slow75RadioButton);

    ActionRadioButton *normalSpeedRadioButton = new ActionRadioButton(this);
    normalSpeedRadioButton->setAction(normalSpeedAction);
    replaySpeedLayout->addWidget(normalSpeedRadioButton);

    ActionRadioButton *fast2xRadioButton = new ActionRadioButton(this);
    fast2xRadioButton->setAction(fastActions.at(0));
    replaySpeedLayout->addWidget(fast2xRadioButton);

    ActionRadioButton *fast4xRadioButton = new ActionRadioButton(this);
    fast4xRadioButton->setAction(fastActions.at(1));
    replaySpeedLayout->addWidget(fast4xRadioButton);

    ActionRadioButton *fast8xRadioButton = new ActionRadioButton(this);
    fast8xRadioButton->setAction(fastActions.at(2));
    replaySpeedLayout->addWidget(fast8xRadioButton);

    ActionRadioButton *fast16xRadioButton = new ActionRadioButton(this);
    fast16xRadioButton->setAction(fastActions.at(3));
    replaySpeedLayout->addWidget(fast16xRadioButton);

    // Custom speed
    d->customSpeedRadioButton = new ActionRadioButton(this);
    d->customSpeedRadioButton->setAction(customSpeedAction);
    replaySpeedLayout->addWidget(d->customSpeedRadioButton);

    d->customSpeedLineEdit = new QLineEdit(this);
    d->customSpeedLineEdit->setMinimumWidth(160);
    replaySpeedLayout->addWidget(d->customSpeedLineEdit);

    d->customReplaySpeedFactorValidator = new QDoubleValidator(d->customSpeedLineEdit);
    d->customReplaySpeedFactorValidator->setRange(ReplaySpeedAbsoluteMin, ReplaySpeedAbsoluteMax, ReplaySpeedDecimalPlaces);
    d->customReplaySpeedPercentValidator = new QDoubleValidator(d->customSpeedLineEdit);
    d->customReplaySpeedPercentValidator->setRange(ReplaySpeedAbsoluteMin * 100.0, ReplaySpeedAbsoluteMax * 100.0, ReplaySpeedDecimalPlaces);

    // The replay speed factor in SkyConnect is always an absolute factor
    const double replaySpeed = d->skyConnect.getReplaySpeedFactor();
    Settings &settings = Settings::getInstance();
    if (settings.getReplaySpeeedUnit() == Replay::SpeedUnit::Absolute) {
        d->lastCustomReplaySpeed = replaySpeed;
    } else {
        d->lastCustomReplaySpeed = replaySpeed * 100.0;
    }
    if (qFuzzyCompare(d->skyConnect.getReplaySpeedFactor(), 1.0)) {
        normalSpeedRadioButton->setChecked(true);
    } else {
        d->customSpeedRadioButton ->setChecked(true);
        d->customSpeedLineEdit->setText(d->locale.toString(d->lastCustomReplaySpeed, 'f', ReplaySpeedDecimalPlaces));
    }

    d->replaySpeedUnitComboBox = new QComboBox(this);
    d->replaySpeedUnitComboBox->addItem(tr("Factor [f]"), Enum::toUnderlyingType(Replay::SpeedUnit::Absolute));
    d->replaySpeedUnitComboBox->addItem(tr("Percent [%]"), Enum::toUnderlyingType(Replay::SpeedUnit::Percent));
    switch (Settings::getInstance().getReplaySpeeedUnit()) {
    case Replay::SpeedUnit::Absolute:
        d->replaySpeedUnitComboBox->setCurrentIndex(0);
        break;
    case Replay::SpeedUnit::Percent:
        d->replaySpeedUnitComboBox->setCurrentIndex(1);
        break;
    default:
        d->replaySpeedUnitComboBox->setCurrentIndex(0);
        break;
    }

    replaySpeedLayout->addWidget(d->replaySpeedUnitComboBox);
}

void MainWindow::updateMinimalUi(bool enabled)
{
    if (enabled) {
        ui->moduleVisibilityWidget->setHidden(enabled);
        ui->moduleSelectorWidget->setHidden(enabled);
    } else {
        ui->moduleVisibilityWidget->setVisible(true);
        ui->moduleSelectorWidget->setVisible(d->showModuleSelector);
    }
    ui->moduleGroupBox->setHidden(enabled);
    Settings::getInstance().setMinimalUiEnabled(enabled);
    // When hiding a widget it takes some time for the layout manager to
    // get notified, so we return to the Qt event queue first
    QTimer::singleShot(0, this, &MainWindow::updateWindowSize);
}

double MainWindow::getCustomSpeedFactor() const
{
    double customSpeedFactor;
    const QString text = d->customSpeedLineEdit->text();
    if (!text.isEmpty()) {
        switch (Settings::getInstance().getReplaySpeeedUnit()) {
        case Replay::SpeedUnit::Absolute:
            customSpeedFactor = d->locale.toDouble(text);
            break;
        case Replay::SpeedUnit::Percent:
            customSpeedFactor = d->locale.toDouble(text) / 100.0;
            break;
        default:
            customSpeedFactor = 1.0;
            break;
        }
    } else {
        customSpeedFactor = 1.0;
    }
    return customSpeedFactor;
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
    if (d->skyConnect.isIdle() || d->skyConnect.getState() == Connect::State::ReplayPaused) {
        qint64 timestamp = time.hour() * MilliSecondsPerHour + time.minute() * MilliSecondsPerMinute + time.second() * MilliSecondsPerSecond;
        d->skyConnect.seek(timestamp);
    }
}

void MainWindow::updateWindowSize() noexcept
{
    if (Settings::getInstance().isMinimalUiEnabled()) {
        setMinimumSize(0, 0);
        // Let the layout manager realise that a widget has been hidden, which is an
        // asynchronous process
        QApplication::processEvents();
        resize(0, 0);
        setFixedSize(minimumSize());
    } else {
        setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
        resize(d->lastNormalUiSize);
    }
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

void MainWindow::handleReplaySpeedSelected(QAction *action) noexcept
{
    ReplaySpeed replaySpeed = static_cast<ReplaySpeed>(action->property(ReplaySpeedProperty).toInt());
    double replaySpeedFactor;
    switch (replaySpeed) {
    case ReplaySpeed::Slow10:
        replaySpeedFactor = 0.1;
        break;
    case ReplaySpeed::Slow25:
        replaySpeedFactor = 0.25;
        break;
    case ReplaySpeed::Slow50:
        replaySpeedFactor = 0.5;
        break;
    case ReplaySpeed::Slow75:
        replaySpeedFactor = 0.75;
        break;
    case ReplaySpeed::Normal:
        replaySpeedFactor = 1.0;
        break;
    case ReplaySpeed::Fast2x:
        replaySpeedFactor = 2.0;
        break;
    case ReplaySpeed::Fast4x:
        replaySpeedFactor = 4.0;
        break;
    case ReplaySpeed::Fast8x:
        replaySpeedFactor = 8.0;
        break;
    case ReplaySpeed::Fast16x:
        replaySpeedFactor = 16.0;
        break;
    case ReplaySpeed::Custom:
        replaySpeedFactor = getCustomSpeedFactor();
        break;
    default:
        replaySpeedFactor = 1.0;
        break;
    }

    d->skyConnect.setReplaySpeedFactor(replaySpeedFactor);
}

void MainWindow::handleCustomSpeedChanged() noexcept
{
    const double customReplaySpeedFactor = getCustomSpeedFactor();
    d->skyConnect.setReplaySpeedFactor(customReplaySpeedFactor);
    switch (Settings::getInstance().getReplaySpeeedUnit()) {
    case Replay::SpeedUnit::Absolute:
        d->lastCustomReplaySpeed = customReplaySpeedFactor;
        break;
    case Replay::SpeedUnit::Percent:
        d->lastCustomReplaySpeed = customReplaySpeedFactor * 100.0;
        break;
    default:
        break;
    }
}

void MainWindow::handleReplaySpeedUnitSelected(int index) noexcept
{
    Settings &settings = Settings::getInstance();
    Replay::SpeedUnit replaySpeedUnit = static_cast<Replay::SpeedUnit>(d->replaySpeedUnitComboBox->itemData(index).toInt());
    switch (replaySpeedUnit) {
    case Replay::SpeedUnit::Absolute:
        if (settings.getReplaySpeeedUnit() != Replay::SpeedUnit::Absolute) {
            // Percent to absolute factor
            d->lastCustomReplaySpeed /= 100.0;
        }
        break;
    case Replay::SpeedUnit::Percent:
        if (settings.getReplaySpeeedUnit() != Replay::SpeedUnit::Percent) {
            // Absolute factor to percent
            d->lastCustomReplaySpeed *= 100.0;
        }
        break;
    default:
        break;
    }
    settings.setReplaySpeedUnit(replaySpeedUnit);
    updateReplaySpeedUi();
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
        ui->recordAction->setEnabled(d->connectedWithLogbook);
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

        switch (Settings::getInstance().getReplaySpeeedUnit()) {
        case Replay::SpeedUnit::Absolute:
            d->customSpeedLineEdit->setToolTip(tr("Custom replay speed factor in [%L1, %L2]").arg(ReplaySpeedAbsoluteMin).arg(ReplaySpeedAbsoluteMax));
            d->customSpeedLineEdit->setValidator(d->customReplaySpeedFactorValidator);
            break;
        case Replay::SpeedUnit::Percent:
            d->customSpeedLineEdit->setToolTip(tr("Custom replay speed % in [%L1%, %L2%]").arg(ReplaySpeedAbsoluteMin * 100.0).arg(ReplaySpeedAbsoluteMax * 100.0));
            d->customSpeedLineEdit->setValidator(d->customReplaySpeedPercentValidator);
            break;
        default:
            d->customSpeedLineEdit->setToolTip("");
            break;
        }

    } else {
        d->customSpeedLineEdit->setEnabled(false);
        d->customSpeedLineEdit->clear();
        d->customSpeedLineEdit->setToolTip("");
    }    
}

void MainWindow::updateTimestamp() noexcept
{
    const qint64 totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
    ui->timestampTimeEdit->blockSignals(true);
    QTime time(0, 0, 0, 0);
    time = time.addMSecs(totalDuration);
    if (d->skyConnect.isRecording()) {
        ui->timestampTimeEdit->setTime(time);
    }
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
        ui->backupLogbookAction->setEnabled(false);
        ui->optimiseLogbookAction->setEnabled(false);
        break;
    default:        
        ui->importCSVAction->setEnabled(d->connectedWithLogbook);
        ui->exportCSVAction->setEnabled(hasRecording);
        ui->backupLogbookAction->setEnabled(d->connectedWithLogbook);
        ui->optimiseLogbookAction->setEnabled(d->connectedWithLogbook);
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

void MainWindow::handleModuleActivated(const QString title, Module::Module moduleId) noexcept
{
    ui->moduleGroupBox->setTitle(title);
    d->activeModuleId = moduleId;
    const bool minimalUi = Settings::getInstance().isMinimalUiEnabled();
    if (minimalUi) {
        updateMinimalUi(false);
    }
}

void MainWindow::on_moduleSelectorVisibleCheckBox_clicked(bool enabled) noexcept
{
    d->showModuleSelector = enabled;
    ui->moduleSelectorWidget->setVisible(d->showModuleSelector);
}

void MainWindow::on_newLogbookAction_triggered() noexcept
{
    const QString logbookPath = DatabaseService::getNewLogbookPath(this);
    if (!logbookPath.isNull()) {
        const bool ok = d->databaseService->connectWithLogbook(logbookPath, this);
        if (!ok) {
            QMessageBox::critical(this, tr("Database error"), tr("The logbook %1 could not be created.").arg(logbookPath));
        }
    }
}

void MainWindow::on_openLogbookAction_triggered() noexcept
{
    QString existingLogbookPath = DatabaseService::getExistingLogbookPath(this);
    if (!existingLogbookPath.isEmpty()) {
        bool ok = d->databaseService->connectWithLogbook(existingLogbookPath, this);
        if (!ok) {
            QMessageBox::critical(this, tr("Database error"), tr("The logbook %1 could not be opened.").arg(existingLogbookPath));
        }
    }
}

void MainWindow::on_backupLogbookAction_triggered() noexcept
{
    bool ok = d->databaseService->backup();
    if (!ok) {
        QMessageBox::critical(this, tr("Database error"), tr("The logbook backup could not be created."));
    }
}

void MainWindow::on_optimiseLogbookAction_triggered() noexcept
{
    bool ok = d->databaseService->optimise();
    if (!ok) {
        QMessageBox::critical(this, tr("Database error"), tr("The logbook could not be optimised."));
    }
}

void MainWindow::on_importCSVAction_triggered() noexcept
{
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
    updateMinimalUi(enabled);
}

void MainWindow::on_aboutLogbookAction_triggered() noexcept
{
    d->aboutLogbookDialog->exec();
}

void MainWindow::on_aboutAction_triggered() noexcept
{
    d->aboutDialog->exec();
}

void MainWindow::on_aboutQtAction_triggered() noexcept
{
    QMessageBox::aboutQt(this);
}

void MainWindow::toggleRecord(bool enable) noexcept
{
    blockSignals(true);
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
            const bool addFormationAircraft = d->activeModuleId == Module::Module::Formation;
            d->skyConnect.startRecording(addFormationAircraft);
        }
        break;
    }
    blockSignals(false);
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

void MainWindow::handleLogbookConnectionChanged(bool connected) noexcept
{
    d->connectedWithLogbook = connected;
    updateUi();
}
