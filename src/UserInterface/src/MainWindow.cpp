/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#include <algorithm>
#include <memory>
#include <vector>
#include <cstdint>
#include <cmath>

#include <QApplication>
#include <QByteArray>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QString>
#include <QUuid>
#include <QTime>
#include <QTimeEdit>
#include <QComboBox>
#include <QSlider>
#include <QLineEdit>
#include <QButtonGroup>
#include <QPushButton>
#include <QRadioButton>
#include <QDoubleValidator>
#include <QIcon>
#include <QStackedWidget>
#include <QEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QAction>
#include <QActionGroup>
#include <QSpacerItem>
#include <QTimer>
#include <QStringBuilder>
#include <QDesktopServices>

#include <Kernel/Unit.h>
#include <Kernel/Const.h>
#include <Kernel/Replay.h>
#include <Kernel/Version.h>
#include <Kernel/Settings.h>
#include <Kernel/Enum.h>
#include <Kernel/SampleRate.h>
#include <Model/Aircraft.h>
#include <Model/PositionData.h>
#include <Model/AircraftInfo.h>
#include <Model/Logbook.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/ConnectionManager.h>
#include <Widget/ActionButton.h>
#include <Widget/ActionRadioButton.h>
#include <Widget/ActionCheckBox.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/Connect.h>
#include <PluginManager/PluginManager.h>
#include <Module/ModuleIntf.h>
#include <Module/ModuleManager.h>
#include "Dialog/AboutDialog.h"
#include "Dialog/LogbookSettingsDialog.h"
#include "Dialog/SettingsDialog.h"
#include "Dialog/FlightDialog.h"
#include "Dialog/SimulationVariablesDialog.h"
#include "Dialog/StatisticsDialog.h"
#include "Dialog/LogbookBackupDialog.h"

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
    constexpr std::int64_t MilliSecondsPerSecond = 1000;
    constexpr std::int64_t MilliSecondsPerMinute = 60 * MilliSecondsPerSecond;
    constexpr std::int64_t MilliSecondsPerHour = 60 * MilliSecondsPerMinute;

    constexpr char ReplaySpeedProperty[] = "ReplaySpeed";

    enum struct ReplaySpeed {
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
        : previousState(Connect::State::Connected),
          connectedWithLogbook(false),
          settingsDialog(nullptr),
          flightDialog(nullptr),
          simulationVariablesDialog(nullptr),
          statisticsDialog(nullptr),
          flightService(std::make_unique<FlightService>()),
          databaseService(std::make_unique<DatabaseService>()),
          replaySpeedActionGroup(nullptr),
          customSpeedRadioButton(nullptr),
          lastCustomReplaySpeed(1.0),
          customSpeedLineEdit(nullptr),
          replaySpeedUnitComboBox(nullptr),
          customReplaySpeedFactorValidator(nullptr),
          customReplaySpeedPercentValidator(nullptr),
          importQActionGroup(nullptr),
          exportQActionGroup(nullptr),
          hasImportPlugins(false),
          hasExportPlugins(false),
          moduleManager(nullptr)
    {}

    Connect::State previousState;
    bool connectedWithLogbook;

    SettingsDialog *settingsDialog;
    FlightDialog *flightDialog;
    SimulationVariablesDialog *simulationVariablesDialog;
    StatisticsDialog *statisticsDialog;    

    Unit unit;

    // Services
    std::unique_ptr<FlightService> flightService;
    std::unique_ptr<DatabaseService> databaseService;

    QSize lastNormalUiSize;

    // Replay speed
    QActionGroup *replaySpeedActionGroup;
    ActionRadioButton *customSpeedRadioButton;
    double lastCustomReplaySpeed;
    QLineEdit *customSpeedLineEdit;
    QComboBox *replaySpeedUnitComboBox;
    QDoubleValidator *customReplaySpeedFactorValidator;
    QDoubleValidator *customReplaySpeedPercentValidator;

    // Import / export
    QActionGroup *importQActionGroup;
    QActionGroup *exportQActionGroup;
    bool hasImportPlugins;
    bool hasExportPlugins;

    std::unique_ptr<ModuleManager> moduleManager;
};

// PUBLIC

MainWindow::MainWindow(const QString &filePath, QWidget *parent) noexcept
    : QMainWindow(parent),
      ui(std::make_unique<Ui::MainWindow>()),
      d(std::make_unique<MainWindowPrivate>())
{
    Q_INIT_RESOURCE(SkyDolly);

    ui->setupUi(this);

    // Connect with logbook
    if (!filePath.isNull()) {
        d->connectedWithLogbook = connectWithLogbook(filePath);
    } else {
        const QString logbookPath = Settings::getInstance().getLogbookPath();
        d->connectedWithLogbook = connectWithLogbook(logbookPath);
    }

    initPlugins();
    initUi();
    updateUi();
    frenchConnection();
}

MainWindow::~MainWindow() noexcept
{
    // Make sure that all widgets having a reference to the flight service
    // are deleted before this MainWindow instance (which owns the flight
    // service); we make sure by simply deleting their parent moduleStackWidget
    delete ui->moduleStackWidget;

#ifdef DEBUG
    qDebug("MainWindow::~MainWindow: DELETED");
#endif
}

bool MainWindow::connectWithLogbook(const QString &filePath) noexcept
{
    bool ok = ConnectionManager::getInstance().connectWithLogbook(filePath, this);
    if (!ok) {
        QMessageBox::critical(this, tr("Database error"), tr("The logbook %1 could not be opened.").arg(filePath));
    }
    return ok;
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

    Metadata metaData;
    if (ConnectionManager::getInstance().getMetadata(metaData)) {
        if (QDateTime::currentDateTime() > metaData.nextBackupDate) {
            std::unique_ptr<LogbookBackupDialog> backupDialog = std::make_unique<LogbookBackupDialog>(this);
            backupDialog->exec();
        }
    }

    Settings &settings = Settings::getInstance();
    settings.setWindowGeometry(saveGeometry());
    settings.setWindowState(saveState());
}

// PRIVATE

void MainWindow::frenchConnection() noexcept
{
    // Sky Connect
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::timestampChanged,
            this, &MainWindow::handleTimestampChanged);
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &MainWindow::updateUi);

    connect(d->replaySpeedActionGroup, &QActionGroup::triggered,
            this, &MainWindow::updateReplaySpeedUi);
    connect(d->replaySpeedActionGroup, &QActionGroup::triggered,
            this, &MainWindow::handleReplaySpeedSelected);

    // Flight
    const Logbook &logbook = Logbook::getInstance();
    const Flight &flight = logbook.getCurrentFlight();
    connect(&flight, &Flight::flightRestored,
            this, &MainWindow::handleFlightRestored);
    connect(&flight, &Flight::timeOffsetChanged,
            this, &MainWindow::updateReplayDuration);
    connect(&flight, &Flight::aircraftRemoved,
            this, &MainWindow::updateReplayDuration);
    connect(&flight, &Flight::cleared,
            this, &MainWindow::updateUi);

    // Settings
    connect(&Settings::getInstance(), &Settings::changed,
            this, &MainWindow::updateMainWindow);

    // Logbook connection
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &MainWindow::handleLogbookConnectionChanged);

    // Menu actions
    connect(d->importQActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onImport);
    connect(d->exportQActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onExport);

    // Settings
    connect(&Settings::getInstance(), &Settings::replayLoopChanged,
            this, &MainWindow::handleReplayLoopChanged);

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
    connect(ui->replayLoopPushButton, &QPushButton::toggled,
            this, &MainWindow::toggleLoopReplay);

    // Menus

    // Help menu
    connect(ui->aboutAction, &QAction::triggered,
            this, &MainWindow::onAboutActionTriggered);
    connect(ui->aboutQtAction, &QAction::triggered,
            this, &MainWindow::onAboutQtActionTriggered);
    connect(ui->onlineManualAction, &QAction::triggered,
            this, &MainWindow::onOnlineManualActionTriggered);

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
}

void MainWindow::initUi() noexcept
{
    Settings &settings = Settings::getInstance();
    setWindowIcon(QIcon(":/img/icons/application-icon.png"));

    // Dialogs
    d->flightDialog = new FlightDialog(*d->flightService, this);
    d->simulationVariablesDialog = new SimulationVariablesDialog(this);
    d->statisticsDialog = new StatisticsDialog(this);
    d->settingsDialog = new SettingsDialog(this);
    ui->stayOnTopAction->setChecked(settings.isWindowStaysOnTopEnabled());

    initModuleSelectorUi();
    initViewUi();
    initControlUi();
    initReplaySpeedUi();

    const bool minimalUi = Settings::getInstance().isMinimalUiEnabled();
    ui->showMinimalAction->setChecked(minimalUi);
    updateMinimalUi(minimalUi);

    QByteArray windowGeometry = settings.getWindowGeometry();
    QByteArray windowState = settings.getWindowState();
    if (!windowGeometry.isEmpty()) {
        restoreGeometry(windowGeometry);
        restoreState(windowState);
    }

    const int previewInfoCount = settings.getPreviewInfoDialogCount();
    if (previewInfoCount > 0) {
        QTimer::singleShot(0, this, [this]() {
            Settings &settings = Settings::getInstance();
            int currentPreviewInfoCount = settings.getPreviewInfoDialogCount();
            --currentPreviewInfoCount;
            QMessageBox::information(this, "Preview",
                QString("%1 is in a preview release phase: while it should be stable to use it is not considered feature-complete. Feedback (bug reports, feature ideas, general feedback) is very welcome.\n\n"
                "The release v%2 focuses again on import and export plugins. Besides new import and export formats it is now possible to export an entire formation flight and import all files in a given directory.\n\n"
                "This dialog will be shown %3 more times.").arg(Version::getApplicationName(), Version::getApplicationVersion()).arg(currentPreviewInfoCount),
                QMessageBox::StandardButton::Ok);            
            settings.setPreviewInfoDialogCount(currentPreviewInfoCount);
        });
    }
}

void MainWindow::initPlugins() noexcept
{
    std::vector<PluginManager::Handle> importPlugins;
    std::vector<PluginManager::Handle> exportPlugins;

    d->importQActionGroup = new QActionGroup(this);
    d->exportQActionGroup = new QActionGroup(this);

    PluginManager &pluginManager = PluginManager::getInstance();
    pluginManager.initialise(this);

    // Import
    importPlugins = PluginManager::getInstance().initialiseImportPlugins();
    d->hasImportPlugins = importPlugins.size() > 0;
    if (d->hasImportPlugins) {
        ui->importMenu->setEnabled(true);

        for (const PluginManager::Handle &handle : importPlugins) {
            QAction *importAction = new QAction(handle.second, ui->importMenu);
            // First: plugin uuid
            importAction->setData(handle.first);
            d->importQActionGroup->addAction(importAction);
            ui->importMenu->addAction(importAction);
        }

    } else {
        ui->importMenu->setEnabled(false);
    }

    // Export
    exportPlugins = PluginManager::getInstance().initialiseExportPlugins();
    d->hasExportPlugins = exportPlugins.size() > 0;
    if (d->hasExportPlugins) {
        ui->exportMenu->setEnabled(true);

        for (const PluginManager::Handle &handle : exportPlugins) {
            QAction *exportAction = new QAction(handle.second, ui->exportMenu);
            // First: plugin uuid
            exportAction->setData(handle.first);
            d->exportQActionGroup->addAction(exportAction);
            ui->exportMenu->addAction(exportAction);
        }

    } else {
        ui->exportMenu->setEnabled(false);
    }

    initSkyConnectPlugin();
}

void MainWindow::initModuleSelectorUi() noexcept
{
    // Modules
    d->moduleManager = std::make_unique<ModuleManager>(*ui->moduleStackWidget, *d->databaseService, *d->flightService);
    ActionCheckBox *actionCheckBox = new ActionCheckBox(false, this);
    actionCheckBox->setAction(ui->showModulesAction);
    actionCheckBox->setFocusPolicy(Qt::NoFocus);
    const QString css =
"QCheckBox::indicator:unchecked {"
"    image: url(:/img/icons/checkbox-expand-normal.png);"
"}"
"QCheckBox::indicator:checked {"
"    image: url(:/img/icons/checkbox-collapse-normal.png);"
"}";
    actionCheckBox->setStyleSheet(css);
    actionCheckBox->setContentsMargins(0, 0, 0, 0);

    std::unique_ptr<QSpacerItem> horizontalSpacerLeft = std::make_unique<QSpacerItem>(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    std::unique_ptr<QSpacerItem> horizontalSpacerRight = std::make_unique<QSpacerItem>(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    std::unique_ptr<QHBoxLayout> moduleVisibilityLayout = std::make_unique<QHBoxLayout>();
    moduleVisibilityLayout->setContentsMargins(0, 0, 0, 0);
    // Transfer ownership to the moduleVisibilityLayout...
    moduleVisibilityLayout->addSpacerItem(horizontalSpacerLeft.release());
    moduleVisibilityLayout->addWidget(actionCheckBox);
    moduleVisibilityLayout->addSpacerItem(horizontalSpacerRight.release());
    // ... and to the moduleVisibilityWidget
    ui->moduleVisibilityWidget->setLayout(moduleVisibilityLayout.release());

    const ModuleIntf &activeModule = d->moduleManager->getActiveModule();
    ui->moduleGroupBox->setTitle(activeModule.getModuleName());

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

void MainWindow::initViewUi() noexcept
{
    Settings &settings = Settings::getInstance();

    const bool moduleSelectorVisible = settings.isModuleSelectorVisible();
    ui->showModulesAction->setChecked(moduleSelectorVisible);
    const bool replaySpeedVisible = settings.isReplaySpeedVisible();
    ui->showReplaySpeedAction->setChecked(replaySpeedVisible);
    ui->replaySpeedGroupBox->setVisible(replaySpeedVisible);
}

void MainWindow::initControlUi() noexcept
{
    ui->positionSlider->setRange(PositionSliderMin, PositionSliderMax);
    ui->positionSlider->setToolTip(tr("%1 ms (%2)").arg(d->unit.formatTimestamp(0), d->unit.formatHHMMSS(0)));
    ui->timestampTimeEdit->setDisplayFormat(TimestampFormat);

    // Record/replay control buttons
    ui->recordButton->setAction(ui->recordAction);
    ui->skipToBeginButton->setAction(ui->skipToBeginAction);
    ui->backwardButton->setAction(ui->backwardAction);
    ui->stopButton->setAction(ui->stopAction);
    ui->pauseButton->setAction(ui->pauseAction);
    ui->playButton->setAction(ui->playAction);
    ui->forwardButton->setAction(ui->forwardAction);
    ui->skipToEndButton->setAction(ui->skipToEndAction);

    // Completely flat button (no border)
    ui->replayLoopPushButton->setStyleSheet("QPushButton {border-style: outset; border-width: 0px;}");
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
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        const double replaySpeed = skyConnect->get().getReplaySpeedFactor();
        Settings &settings = Settings::getInstance();
        if (settings.getReplaySpeeedUnit() == Replay::SpeedUnit::Absolute) {
            d->lastCustomReplaySpeed = replaySpeed;
        } else {
            d->lastCustomReplaySpeed = replaySpeed * 100.0;
        }
        if (qFuzzyCompare(skyConnect->get().getReplaySpeedFactor(), 1.0)) {
            normalSpeedRadioButton->setChecked(true);
        } else {
            d->customSpeedRadioButton ->setChecked(true);
            d->customSpeedLineEdit->setText(d->unit.formatNumber(d->lastCustomReplaySpeed, ReplaySpeedDecimalPlaces));
        }
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
    }

    replaySpeedLayout->addWidget(d->replaySpeedUnitComboBox);
}

void MainWindow::initSkyConnectPlugin() noexcept
{
    Settings &settings = Settings::getInstance();
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    std::vector<SkyConnectManager::Handle> skyConnectPlugins = skyConnectManager.initialisePlugins();

    QUuid uuid = settings.getSkyConnectPluginUuid();
    // Try to load plugin as stored in the settings
    bool ok = !uuid.isNull() && skyConnectManager.tryAndSetCurrentSkyConnect(uuid);
    if (!ok) {
        // First attempt or not a valid plugin (anymore), so try the other plugins
        if (skyConnectPlugins.size() == 1) {
            // There is only one plugin
            uuid = skyConnectPlugins.front().first;
            ok = skyConnectManager.tryAndSetCurrentSkyConnect(uuid);
        } else if (skyConnectPlugins.size() > 1) {
            // Check if an actual flight simulator instance is running
            for (auto &plugin : skyConnectPlugins) {
                if (FlightSimulator::isRunning(plugin.second.flightSimulatorId)) {
                    uuid = plugin.first;
                    ok = skyConnectManager.tryAndSetCurrentSkyConnect(uuid);
                }
                if (ok) {
                    break;
                }
            }
            if (!ok) {
                // No instance running (or no valid plugin), so try again and
                // check if the given flight simulator is installed
                for (auto &plugin : skyConnectPlugins) {
                    if (FlightSimulator::isInstalled(plugin.second.flightSimulatorId)) {
                        uuid = plugin.first;
                        ok = skyConnectManager.tryAndSetCurrentSkyConnect(uuid);
                    }
                    if (ok) {
                        break;
                    }
                }
            }
            if (!ok) {
                // Default to the Flight Simulator 2020 plugin
                for (auto &plugin : skyConnectPlugins) {
                    if (plugin.second.flightSimulatorId == FlightSimulator::Id::FS2020) {
                        uuid = plugin.first;
                        ok = skyConnectManager.tryAndSetCurrentSkyConnect(uuid);
                    }
                    if (ok) {
                        break;
                    }
                }
            }
            if (!ok) {
                // Everything failed, so as a last resort try with a generic ("All") plugin
                for (auto &plugin : skyConnectPlugins) {
                    if (plugin.second.flightSimulatorId == FlightSimulator::Id::All) {
                        uuid = plugin.first;
                        ok = skyConnectManager.tryAndSetCurrentSkyConnect(uuid);
                    }
                    if (ok) {
                        break;
                    }
                }
            }
        } else {
            // No plugins found
            ok = false;
        }
    }
    if (ok) {
        settings.setSkyConnectPluginUuid(uuid);
    } else {
        QMessageBox::warning(this, tr("No valid connection plugin found"), tr("No valid connection plugin has been found in the plugin directory! Sky Dolly will launch with reduced functionality."));
    }
}

void MainWindow::updateMinimalUi(bool enabled)
{
    Settings &settings = Settings::getInstance();
    if (enabled) {
        ui->moduleVisibilityWidget->setHidden(true);
        ui->moduleSelectorWidget->setHidden(true);
        ui->showModulesAction->setChecked(false);
        ui->showModulesAction->setEnabled(false);
        ui->showReplaySpeedAction->setEnabled(false);
    } else {
        ui->moduleVisibilityWidget->setVisible(true);
        ui->moduleSelectorWidget->setVisible(settings.isModuleSelectorVisible());
        ui->showModulesAction->setChecked(settings.isModuleSelectorVisible());
        ui->showModulesAction->setEnabled(true);
        ui->showReplaySpeedAction->setEnabled(true);
    }
    ui->moduleGroupBox->setHidden(enabled);
    settings.setMinimalUiEnabled(enabled);
    // When hiding a widget it takes some time for the layout manager to
    // get notified, so we return to the Qt event queue first
    QTimer::singleShot(0, this, &MainWindow::updateWindowSize);
}

void MainWindow::updateRecordingDuration(std::int64_t timestamp) noexcept
{
    ui->timestampTimeEdit->blockSignals(true);
    QTime time = QTime::fromMSecsSinceStartOfDay(timestamp);
    ui->timestampTimeEdit->setMaximumTime(time);
    ui->timestampTimeEdit->setTime(time);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::updatePositionSlider(std::int64_t timestamp) noexcept
{
    const std::int64_t totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
    const std::int64_t ts = std::min(timestamp, totalDuration);

    const int sliderPosition = totalDuration > 0 ?
                               static_cast<int>(std::round(PositionSliderMax * (static_cast<double>(ts) / static_cast<double>(totalDuration)))) :
                               0;

    ui->positionSlider->blockSignals(true);
    ui->positionSlider->setValue(sliderPosition);
    ui->positionSlider->setToolTip(tr("%1 ms (%2)").arg(d->unit.formatTimestamp(timestamp), d->unit.formatHHMMSS(timestamp)));
    ui->positionSlider->blockSignals(false);

    const QTime time = QTime::fromMSecsSinceStartOfDay(timestamp);
    ui->timestampTimeEdit->blockSignals(true);
    ui->timestampTimeEdit->setTime(time);
    ui->timestampTimeEdit->blockSignals(false);
}

double MainWindow::getCustomSpeedFactor() const
{
    double customSpeedFactor;
    const QString text = d->customSpeedLineEdit->text();
    if (!text.isEmpty()) {
        switch (Settings::getInstance().getReplaySpeeedUnit()) {
        case Replay::SpeedUnit::Absolute:
            customSpeedFactor = d->unit.toNumber(text);
            break;
        case Replay::SpeedUnit::Percent:
            customSpeedFactor = d->unit.toNumber(text) / 100.0;
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
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        d->previousState = skyConnect->get().getState();
        if (d->previousState == Connect::State::Replay) {
            // Pause the replay while sliding the position slider
            skyConnect->get().setPaused(true);
        }
    }
}

void MainWindow::on_positionSlider_valueChanged(int value) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        const double scale = static_cast<double>(value) / static_cast<double>(PositionSliderMax);
        const std::int64_t totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
        const std::int64_t timestamp = static_cast<std::int64_t>(std::round(scale * static_cast<double>(totalDuration)));
        ui->positionSlider->setToolTip(tr("%1 ms (%2)").arg(d->unit.formatTimestamp(timestamp), d->unit.formatHHMMSS(timestamp)));

        // Prevent the timestampTimeEdit field to set the replay position as well
        ui->timestampTimeEdit->blockSignals(true);
        skyConnect->get().seek(timestamp);
        ui->timestampTimeEdit->blockSignals(false);
    }
}

void MainWindow::on_positionSlider_sliderReleased() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (d->previousState == Connect::State::Replay) {
            skyConnect->get().setPaused(false);
        }
    }
}

void MainWindow::on_timestampTimeEdit_timeChanged(const QTime &time) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect && (skyConnect->get().isIdle() || skyConnect->get().getState() == Connect::State::ReplayPaused)) {
        std::int64_t timestamp = time.hour() * MilliSecondsPerHour + time.minute() * MilliSecondsPerMinute + time.second() * MilliSecondsPerSecond;
        skyConnect->get().seek(timestamp);
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

void MainWindow::handleTimestampChanged(std::int64_t timestamp) noexcept
{
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        if (skyConnect->get().isRecording()) {
            updateRecordingDuration(timestamp);
        } else {
            updatePositionSlider(timestamp);
        };
    }
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
    }

    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().setReplaySpeedFactor(replaySpeedFactor);
    }
}

void MainWindow::handleCustomSpeedChanged() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const double customReplaySpeedFactor = getCustomSpeedFactor();
    if (skyConnect) {
        skyConnect->get().setReplaySpeedFactor(customReplaySpeedFactor);
    }
    switch (Settings::getInstance().getReplaySpeeedUnit()) {
    case Replay::SpeedUnit::Absolute:
        d->lastCustomReplaySpeed = customReplaySpeedFactor;
        break;
    case Replay::SpeedUnit::Percent:
        d->lastCustomReplaySpeed = customReplaySpeedFactor * 100.0;
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
    }
    settings.setReplaySpeedUnit(replaySpeedUnit);
    updateReplaySpeedUi();
}

void MainWindow::updateUi() noexcept
{
    updateControlUi();
    updateControlIcons();
    updateReplaySpeedUi();
    updateReplayDuration();
    updateFileMenu();
    updateWindowMenu();
    updateMainWindow();
}

void MainWindow::updateControlUi() noexcept
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const bool hasRecording = aircraft.hasRecording();

    const SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const bool hasSkyConnectPlugins = skyConnectManager.hasPlugins();
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = skyConnectManager.getCurrentSkyConnect();
    const Connect::State state = skyConnect ? skyConnect->get().getState() : Connect::State::Disconnected;
    switch (state) {
    case Connect::State::Disconnected:
        // Fall-thru intended: each time a control element is triggered a connection
        // attempt is made, so we enable the same elements as in connected state
        [[fallthrough]];
    case Connect::State::Connected:
        // Actions
        ui->recordAction->setEnabled(d->connectedWithLogbook && hasSkyConnectPlugins);
        ui->recordAction->setChecked(false);
        ui->stopAction->setEnabled(false);
        ui->pauseAction->setEnabled(false);
        ui->pauseAction->setChecked(false);
        ui->playAction->setEnabled(hasRecording && hasSkyConnectPlugins);
        ui->playAction->setChecked(false);
        // Transport
        ui->skipToBeginAction->setEnabled(hasRecording && hasSkyConnectPlugins);
        ui->backwardAction->setEnabled(hasRecording && hasSkyConnectPlugins);
        ui->forwardAction->setEnabled(hasRecording && hasSkyConnectPlugins);
        ui->skipToEndAction->setEnabled(hasRecording && hasSkyConnectPlugins);
        // Position
        ui->positionSlider->setEnabled(hasRecording && hasSkyConnectPlugins);
        ui->timestampTimeEdit->setEnabled(hasRecording && hasSkyConnectPlugins);
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
    }

    const bool loopReplayEnabled = Settings::getInstance().isReplayLoopEnabled();
    ui->replayLoopPushButton->setChecked(loopReplayEnabled);
    if (loopReplayEnabled) {
        ui->replayLoopPushButton->setToolTip(tr("Replay loop is enabled."));
    } else {
        ui->replayLoopPushButton->setToolTip(tr("Replay stops at end."));
    }
}

void MainWindow::updateControlIcons() noexcept
{
    QIcon recordIcon;
    switch (d->moduleManager->getActiveModule().getModuleId()) {
    case Module::Module::None:
        [[fallthrough]];
    case Module::Module::Logbook:
        recordIcon.addFile(":/img/icons/record-normal.png", QSize(), QIcon::Normal, QIcon::Off);
        recordIcon.addFile(":/img/icons/record-normal-on.png", QSize(), QIcon::Normal, QIcon::On);
        break;
    case Module::Module::Formation:
        recordIcon.addFile(":/img/icons/record-add-normal.png", QSize(), QIcon::Normal, QIcon::Off);
        recordIcon.addFile(":/img/icons/record-add-normal-on.png", QSize(), QIcon::Normal, QIcon::On);
        break;

    }
    ui->recordAction->setIcon(recordIcon);
}

void MainWindow::updateReplaySpeedUi() noexcept
{
    if (d->customSpeedRadioButton->isChecked()) {
        d->customSpeedLineEdit->setEnabled(true);
        d->customSpeedLineEdit->setText(d->unit.formatNumber(d->lastCustomReplaySpeed, ReplaySpeedDecimalPlaces));

        switch (Settings::getInstance().getReplaySpeeedUnit()) {
        case Replay::SpeedUnit::Absolute:
            d->customSpeedLineEdit->setToolTip(tr("Custom replay speed factor in [%L1, %L2].").arg(ReplaySpeedAbsoluteMin).arg(ReplaySpeedAbsoluteMax));
            d->customSpeedLineEdit->setValidator(d->customReplaySpeedFactorValidator);
            break;
        case Replay::SpeedUnit::Percent:
            d->customSpeedLineEdit->setToolTip(tr("Custom replay speed % in [%L1%, %L2%].").arg(ReplaySpeedAbsoluteMin * 100.0).arg(ReplaySpeedAbsoluteMax * 100.0));
            d->customSpeedLineEdit->setValidator(d->customReplaySpeedPercentValidator);
            break;
        }

    } else {
        d->customSpeedLineEdit->setEnabled(false);
        d->customSpeedLineEdit->clear();
        d->customSpeedLineEdit->setToolTip("");
    }    
}

void MainWindow::updateReplayDuration() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const std::int64_t totalDuration = flight.getTotalDurationMSec();
    ui->timestampTimeEdit->blockSignals(true);
    const QTime time = QTime::fromMSecsSinceStartOfDay(totalDuration);
    ui->timestampTimeEdit->setMaximumTime(time);
    ui->timestampTimeEdit->blockSignals(false);
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const std::int64_t timestamp = skyConnect ? skyConnect->get().getCurrentTimestamp() : 0;
    updatePositionSlider(timestamp);
}

void MainWindow::updateFileMenu() noexcept
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const bool hasRecording = aircraft.hasRecording();
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const Connect::State state = skyConnect ? skyConnect->get().getState() : Connect::State::Disconnected;
    switch (state) {
    case Connect::State::Recording:
        // Fall-thru intentional
    case Connect::State::RecordingPaused:
        ui->importMenu->setEnabled(false);
        ui->exportMenu->setEnabled(false);
        ui->optimiseLogbookAction->setEnabled(false);
        break;
    default:        
        ui->importMenu->setEnabled(d->hasImportPlugins && d->connectedWithLogbook);
        ui->exportMenu->setEnabled(d->hasExportPlugins && hasRecording);
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

    if (settings.getRecordingSampleRate() != SampleRate::SampleRate::Auto) {
        ui->recordAction->setToolTip(tr("Record [@%1 Hz].").arg(Settings::getInstance().getRecordingSampleRateValue()));
    } else {
        ui->recordAction->setToolTip(tr("Record [auto sample rate]."));
    }

    if (settings.isAbsoluteSeekEnabled()) {
        double seekIntervalSeconds = settings.getSeekIntervalSeconds();
        ui->forwardAction->setToolTip(tr("Fast forward [%1 sec].").arg(seekIntervalSeconds));
        ui->backwardAction->setToolTip(tr("Rewind [%1 sec].").arg(seekIntervalSeconds));
    } else {
        double seekIntervalPercent = settings.getSeekIntervalPercent();
        ui->forwardAction->setToolTip(tr("Fast forward [%1 %].").arg(seekIntervalPercent));
        ui->backwardAction->setToolTip(tr("Rewind [%1 %].").arg(seekIntervalPercent));
    }

    if (settings.isModuleSelectorVisible()) {
        ui->showModulesAction->setToolTip(tr("Hide modules."));
    } else {
        ui->showModulesAction->setToolTip(tr("Show modules."));
    }
}

void MainWindow::handleModuleActivated(const QString title, [[maybe_unused]] Module::Module moduleId) noexcept
{
    ui->moduleGroupBox->setTitle(title);
    const bool minimalUi = Settings::getInstance().isMinimalUiEnabled();
    if (minimalUi) {
        updateMinimalUi(false);
    }
    updateControlIcons();
}

// File menu

void MainWindow::on_newLogbookAction_triggered() noexcept
{
    const QString logbookPath = DatabaseService::getNewLogbookPath(this);
    if (!logbookPath.isNull()) {
        const bool ok = ConnectionManager::getInstance().connectWithLogbook(logbookPath, this);
        if (!ok) {
            QMessageBox::critical(this, tr("Database error"), tr("The logbook %1 could not be created.").arg(logbookPath));
        }
    }
}

void MainWindow::on_openLogbookAction_triggered() noexcept
{
    QString filePath = DatabaseService::getExistingLogbookPath(this);
    if (!filePath.isEmpty()) {
        connectWithLogbook(filePath);
    }
}

void MainWindow::on_optimiseLogbookAction_triggered() noexcept
{
    bool ok = ConnectionManager::getInstance().optimise();
    if (!ok) {
        QMessageBox::critical(this, tr("Database error"), tr("The logbook could not be optimised."));
    }
}

void MainWindow::on_showSettingsAction_triggered() noexcept
{
    d->settingsDialog->exec();
}

void MainWindow::on_showLogbookSettingsAction_triggered() noexcept
{
    std::unique_ptr<LogbookSettingsDialog> logbookSettingsDialog = std::make_unique<LogbookSettingsDialog>(this);
    logbookSettingsDialog->exec();
}

void MainWindow::on_quitAction_triggered() noexcept
{
    close();
}

// View menu

void MainWindow::on_showModulesAction_triggered(bool enabled) noexcept
{
    Settings &settings = Settings::getInstance();
    settings.setModuleSelectorVisible(enabled);
    ui->moduleSelectorWidget->setVisible(enabled);
}

void MainWindow::on_showReplaySpeedAction_triggered(bool enabled) noexcept
{
    Settings &settings = Settings::getInstance();
    settings.setReplaySpeedVisible(enabled);
    ui->replaySpeedGroupBox->setVisible(enabled);
}

// Window menu

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

// Help menu

void MainWindow::onAboutActionTriggered() noexcept
{
    std::unique_ptr<AboutDialog> aboutDialog = std::make_unique<AboutDialog>(this);
    aboutDialog->exec();
}

void MainWindow::onAboutQtActionTriggered() noexcept
{
    QMessageBox::aboutQt(this);
}

void MainWindow::onOnlineManualActionTriggered() const noexcept
{
    QDesktopServices::openUrl(QUrl("https://till213.github.io/SkyDolly/manual/en/"));
}

// Replay

void MainWindow::toggleRecord(bool enable) noexcept
{
    blockSignals(true);
    d->moduleManager->getActiveModule().setRecording(enable);
    blockSignals(false);
}

void MainWindow::togglePause(bool enable) noexcept
{
    d->moduleManager->getActiveModule().setPaused(enable);
}

void MainWindow::togglePlay(bool enable) noexcept
{
     d->moduleManager->getActiveModule().setPlaying(enable);
}

void MainWindow::stop() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().stop();
    }
}

// Transport

void MainWindow::skipToBegin() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipToBegin();
    }
}

void MainWindow::skipBackward() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipBackward();
    }
}

void MainWindow::skipForward() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipForward();
    }
}

void MainWindow::skipToEnd() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipToEnd();
    }
}

void MainWindow::toggleLoopReplay(bool checked) noexcept
{
    Settings::getInstance().setLoopReplayEnabled(checked);
}

// Service

void MainWindow::handleFlightRestored() noexcept
{
    updateUi();
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().skipToBegin();
        ModuleIntf &module = d->moduleManager->getActiveModule();
        if (skyConnect->get().isConnected()) {
            // Make sure we are unpaused...
            module.setPaused(false);
            // ... play the first frame (which will "move" to the new location)...
            module.setPlaying(true);
            // ... and pause again (such that the new scenery can be loaded)
            module.setPaused(true);
        }
    }
}

void MainWindow::handleLogbookConnectionChanged(bool connected) noexcept
{
    d->connectedWithLogbook = connected;
    updateUi();
}

void MainWindow::onImport(QAction *action) noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    const QUuid pluginUuid = action->data().toUuid();
    const bool ok = PluginManager::getInstance().importFlight(pluginUuid, *d->flightService, flight);
    if (ok) {
        updateUi();
        std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
        if (skyConnect) {
            skyConnect->get().skipToBegin();
            ModuleIntf &module = d->moduleManager->getActiveModule();
            if (skyConnect->get().isConnected()) {
                module.setPlaying(true);
                module.setPaused(true);
            }
        }
    }
}

void MainWindow::onExport(QAction *action) noexcept
{
    const QUuid pluginUuid = action->data().toUuid();
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    PluginManager::getInstance().exportFlight(flight, pluginUuid);
}

void MainWindow::handleReplayLoopChanged() noexcept
{
    updateControlUi();
}
