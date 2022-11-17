/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#include <QCursor>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QUuid>
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
#include <Persistence/Service/LocationService.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/PersistenceManager.h>
#include "Persistence/Metadata.h"
#include <Widget/ActionButton.h>
#include <Widget/ActionRadioButton.h>
#include <Widget/ActionCheckBox.h>
#include <Widget/Platform.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/Connect.h>
#include <PluginManager/PluginManager.h>
#include <PluginManager/ModuleIntf.h>
#include <PluginManager/ModuleManager.h>
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
    constexpr int PositionSliderMin {0};
    constexpr int PositionSliderMax {1000};
    constexpr double ReplaySpeedAbsoluteMin {0.01};
    // A replay speed with factor 200 should be fast enough
    constexpr double ReplaySpeedAbsoluteMax {200.0};
    constexpr double ReplaySpeedDecimalPlaces {2};

    constexpr int CustomSpeedLineEditMinimumWidth {40};

    constexpr char TimestampFormat[] {"hh:mm:ss"};
    constexpr std::int64_t MilliSecondsPerSecond {1000};
    constexpr std::int64_t MilliSecondsPerMinute {60 * MilliSecondsPerSecond};
    constexpr std::int64_t MilliSecondsPerHour {60 * MilliSecondsPerMinute};

    constexpr char ReplaySpeedProperty[] {"ReplaySpeed"};

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

struct MainWindowPrivate
{
    Connect::State previousState {Connect::State::Connected};
    bool connectedWithLogbook {false};

    FlightDialog *flightDialog {nullptr};
    SimulationVariablesDialog *simulationVariablesDialog {nullptr};
    StatisticsDialog *statisticsDialog {nullptr};

    Unit unit;

    // Services
    std::unique_ptr<FlightService> flightService {std::make_unique<FlightService>()};
    std::unique_ptr<LocationService> locationService {std::make_unique<LocationService>()};

    QSize lastNormalUiSize;

    // Replay speed
    QActionGroup *replaySpeedActionGroup {nullptr};
    ActionRadioButton *customSpeedRadioButton {nullptr};
    double lastCustomReplaySpeedFactor {1.0};
    QLineEdit *customSpeedLineEdit {nullptr};
    QComboBox *replaySpeedUnitComboBox {nullptr};
    QDoubleValidator *customReplaySpeedFactorValidator {nullptr};
    QDoubleValidator *customReplaySpeedPercentValidator {nullptr};

    // Import / export
    QActionGroup *flightImportActionGroup {nullptr};
    QActionGroup *flightExportActionGroup {nullptr};
    QActionGroup *locationImportActionGroup {nullptr};
    QActionGroup *locationExportActionGroup {nullptr};
    bool hasFlightImportPlugins {false};
    bool hasFlightExportPlugins {false};
    bool hasLocationImportPlugins {false};
    bool hasLocationExportPlugins {false};

    std::unique_ptr<ModuleManager> moduleManager;
};

// PUBLIC

MainWindow::MainWindow(const QString &filePath, QWidget *parent) noexcept
    : QMainWindow(parent),
      ui(std::make_unique<Ui::MainWindow>()),
      d(std::make_unique<MainWindowPrivate>())
{
    Q_INIT_RESOURCE(UserInterface);

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
#ifdef DEBUG
    qDebug() << "MainWindow::~MainWindow: DELETED";
#endif
}

bool MainWindow::connectWithLogbook(const QString &filePath) noexcept
{
    bool ok = PersistenceManager::getInstance().connectWithLogbook(filePath, this);
    if (!ok) {
        QMessageBox::critical(this, tr("Logbook error"), tr("The logbook %1 could not be opened.").arg(QDir::toNativeSeparators(filePath)));
    }
    return ok;
}

// PROTECTED

void MainWindow::resizeEvent(QResizeEvent *event) noexcept
{
    if (!isMinimalUiEnabled()) {
        d->lastNormalUiSize = event->size();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) noexcept
{
    QMainWindow::closeEvent(event);

    bool ok {true};
    const Metadata metaData = PersistenceManager::getInstance().getMetadata(&ok);
    if (ok) {
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
            this, &MainWindow::onTimestampChanged);
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &MainWindow::updateUi);
    connect(&skyConnectManager, &SkyConnectManager::recordingStopped,
            this, &MainWindow::onRecordingDurationChanged);

    // Replay speed
    connect(d->replaySpeedActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onReplaySpeedSelected);

    // Flight
    const Logbook &logbook = Logbook::getInstance();
    const Flight &flight = logbook.getCurrentFlight();
    connect(&flight, &Flight::flightRestored,
            this, &MainWindow::onFlightRestored);
    connect(&flight, &Flight::timeOffsetChanged,
            this, &MainWindow::onRecordingDurationChanged);
    connect(&flight, &Flight::aircraftRemoved,
            this, &MainWindow::onRecordingDurationChanged);
    connect(&flight, &Flight::cleared,
            this, &MainWindow::updateUi);

    // Settings
    Settings &settings = Settings::getInstance();
    connect(&settings, &Settings::changed,
            this, &MainWindow::updateMainWindow);
    connect(&settings, &Settings::defaultMinimalUiButtonTextVisibilityChanged,
            this, &MainWindow::onDefaultMinimalUiButtonTextVisibilityChanged);
    connect(&settings, &Settings::defaultMinimalUiNonEssentialButtonVisibilityChanged,
            this, &MainWindow::onDefaultMinimalUiEssentialButtonVisibilityChanged);
    connect(&settings, &Settings::replayLoopChanged,
            this, &MainWindow::onReplayLoopChanged);

    // Logbook connection
    connect(&PersistenceManager::getInstance(), &PersistenceManager::connectionChanged,
            this, &MainWindow::onLogbookConnectionChanged);

    // Ui elements
    connect(d->customSpeedLineEdit, &QLineEdit::editingFinished,
            this, &MainWindow::onCustomSpeedChanged);
    connect(d->replaySpeedUnitComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onReplaySpeedUnitSelected);
    connect(ui->positionSlider, &QSlider::sliderPressed,
            this, &MainWindow::onPositionSliderPressed);
    connect(ui->positionSlider, &QSlider::valueChanged,
            this, &MainWindow::onPositionSliderValueChanged);
    connect(ui->positionSlider, &QSlider::sliderReleased,
            this, &MainWindow::onPositionSliderReleased);
    connect(ui->timestampTimeEdit, &QTimeEdit::timeChanged,
            this, &MainWindow::onTimeStampTimeEditChanged);

    // Actions

    // Control actions
    // Note: we explicitly connect to signal triggered - and not toggled - also in
    //       case of checkable QActions. Because we programmatically change the
    //       checked state of QActions based on connection state changes (in updateControlUi),
    //       which emit the toggled signal. This would then result in a cascade of toggled signals
    //       (e.g. clicking the Pause button also unchecks the Record/Play button) which
    //       is hard to control -> so we simply only react to the general triggered signals
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
    connect(ui->loopReplayAction, &QAction::triggered,
            this, &MainWindow::toggleLoopReplay);

    // Modules
    connect(d->moduleManager.get(), &ModuleManager::activated,
            this, &MainWindow::onModuleActivated);

    // Menus

    // File menu
    connect(ui->newLogbookAction, &QAction::triggered,
            this, &MainWindow::createNewLogbook);
    connect(ui->openLogbookAction, &QAction::triggered,
            this, &MainWindow::openLogbook);
    connect(ui->optimiseLogbookAction, &QAction::triggered,
            this, &MainWindow::optimiseLogbook);
    connect(ui->showSettingsAction, &QAction::triggered,
            this, &MainWindow::showSettings);
    connect(ui->showLogbookSettingsAction, &QAction::triggered,
            this, &MainWindow::showLogbookSettings);
    connect(ui->quitAction, &QAction::triggered,
            this, &MainWindow::quit);

    // Menu actions
    connect(d->flightImportActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onFlightImport);
    connect(d->flightExportActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onFlightExport);
    connect(d->locationImportActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onLocationImport);
    connect(d->locationExportActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onLocationExport);

    // View menu
    // Note: we explicitly connect to signal triggered - and not toggled - also in
    //       case of checkable QActions. Because we programmatically change the
    //       checked state of QActions based on connection state changes),
    //       which emit the toggled signal. This would then result in a cascade of toggled signals
    //       (e.g. going into minimal UI mode may uncheck the "Show Replay Speed" option) which
    //       is hard to control -> so we simply only react to the general triggered signals
    connect(ui->showModulesAction, &QAction::triggered,
            this, &MainWindow::onShowModulesChanged);
    connect(ui->showReplaySpeedAction, &QAction::triggered,
            this, &MainWindow::onShowReplaySpeedChanged);

    // Windows
    connect(ui->showFlightAction, &QAction::toggled,
            this, &MainWindow::toggleFlightDialog);
    connect(ui->showSimulationVariablesAction, &QAction::toggled,
            this, &MainWindow::toggleSimulationVariablesDialog);
    connect(ui->showStatisticsAction, &QAction::toggled,
            this, &MainWindow::toggleStatisticsDialog);

    connect(ui->stayOnTopAction, &QAction::toggled,
            this, &MainWindow::toggleStayOnTop);
    connect(ui->showMinimalAction, &QAction::toggled,
            this, &MainWindow::toggleMinimalUi);

    // Help menu
    connect(ui->aboutAction, &QAction::triggered,
            this, &MainWindow::showAboutDialog);
    connect(ui->onlineManualAction, &QAction::triggered,
            this, &MainWindow::showOnlineManual);
}

void MainWindow::initUi() noexcept
{
    Settings &settings = Settings::getInstance();
    setWindowIcon(QIcon(":/img/icons/application-icon.png"));

    // Window menu
    ui->stayOnTopAction->setChecked(settings.isWindowStaysOnTopEnabled());

    initModuleSelectorUi();
    initViewUi();
    initControlUi();
    initReplaySpeedUi();

    const bool minimalUi = isMinimalUiEnabled();
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
                QString("%1 is in a preview release phase: while it should be stable to use it is not considered feature-complete.\n\n"
                "This release v%2 also implements the modules as plugins, reducing memory (RAM) usage even further.\n\n"
                "At the same time a new Locations module is introduced, allowing to quickly teleport to the selected location. "
                "It comes with an initial set of default (system) locations that are neither editable nor deletable.\n\n"
                "Own user locations can either be captured from an active flight (even during recording), or copy & pasted from the system clipboard (with CTRL + V).\n\n"
                "Note that the Locations module is work in progress: filtering and additional attributes are yet to be implemented.\n\n"
                "This dialog will be shown %3 more times.").arg(Version::getApplicationName(), Version::getApplicationVersion()).arg(currentPreviewInfoCount),
                QMessageBox::StandardButton::Ok);            
            settings.setPreviewInfoDialogCount(currentPreviewInfoCount);
        });
    }
}

void MainWindow::initPlugins() noexcept
{
    std::vector<PluginManager::Handle> flightImportPlugins;
    std::vector<PluginManager::Handle> flightExportPlugins;
    std::vector<PluginManager::Handle> locationImportPlugins;
    std::vector<PluginManager::Handle> locationExportPlugins;

    d->flightImportActionGroup = new QActionGroup(this);
    d->flightExportActionGroup = new QActionGroup(this);
    d->locationImportActionGroup = new QActionGroup(this);
    d->locationExportActionGroup = new QActionGroup(this);

    PluginManager &pluginManager = PluginManager::getInstance();
    pluginManager.initialise(this);

    // Flight import
    flightImportPlugins = pluginManager.initialiseFlightImportPlugins();
    d->hasFlightImportPlugins = flightImportPlugins.size() > 0;
    if (d->hasFlightImportPlugins) {
        ui->flightImportMenu->setEnabled(true);
        for (const PluginManager::Handle &handle : flightImportPlugins) {
            QAction *flightImportAction = new QAction(handle.second, ui->flightImportMenu);
            // First: plugin uuid
            flightImportAction->setData(handle.first);
            d->flightImportActionGroup->addAction(flightImportAction);
            ui->flightImportMenu->addAction(flightImportAction);
        }
    } else {
        ui->flightImportMenu->setEnabled(false);
    }

    // Flight export
    flightExportPlugins = pluginManager.initialiseFlightExportPlugins();
    d->hasFlightExportPlugins = flightExportPlugins.size() > 0;
    if (d->hasFlightExportPlugins) {
        ui->flightExportMenu->setEnabled(true);
        for (const PluginManager::Handle &handle : flightExportPlugins) {
            QAction *flightExportAction = new QAction(handle.second, ui->flightExportMenu);
            // First: plugin uuid
            flightExportAction->setData(handle.first);
            d->flightExportActionGroup->addAction(flightExportAction);
            ui->flightExportMenu->addAction(flightExportAction);
        }
    } else {
        ui->flightExportMenu->setEnabled(false);
    }

    // Location import
    locationImportPlugins = pluginManager.initialiseLocationImportPlugins();
    d->hasLocationImportPlugins = locationImportPlugins.size() > 0;
    if (d->hasLocationImportPlugins) {
        ui->locationImportMenu->setEnabled(true);
        for (const PluginManager::Handle &handle : locationImportPlugins) {
            QAction *locationImportAction = new QAction(handle.second, ui->locationImportMenu);
            // First: plugin uuid
            locationImportAction->setData(handle.first);
            d->locationImportActionGroup->addAction(locationImportAction);
            ui->locationImportMenu->addAction(locationImportAction);
        }
    } else {
        ui->locationImportMenu->setEnabled(false);
    }

    // Location export
    locationExportPlugins = pluginManager.initialiseLocationExportPlugins();
    d->hasLocationExportPlugins = locationExportPlugins.size() > 0;
    if (d->hasLocationExportPlugins) {
        ui->locationExportMenu->setEnabled(true);
        for (const PluginManager::Handle &handle : locationExportPlugins) {
            QAction *locationExportAction = new QAction(handle.second, ui->locationExportMenu);
            // First: plugin uuid
            locationExportAction->setData(handle.first);
            d->locationExportActionGroup->addAction(locationExportAction);
            ui->locationExportMenu->addAction(locationExportAction);
        }
    } else {
        ui->locationExportMenu->setEnabled(false);
    }

    initSkyConnectPlugin();
}

void MainWindow::initModuleSelectorUi() noexcept
{
    // Modules
    d->moduleManager = std::make_unique<ModuleManager>(*ui->moduleGroupBox->layout());
    ActionCheckBox *actionCheckBox = new ActionCheckBox(false, this);
    actionCheckBox->setAction(ui->showModulesAction);
    actionCheckBox->setFocusPolicy(Qt::NoFocus);
    const QString css = QStringLiteral(
"QCheckBox::indicator:unchecked {"
"    image: url(:/img/icons/checkbox-expand-normal.png);"
"}"
"QCheckBox::indicator:checked {"
"    image: url(:/img/icons/checkbox-collapse-normal.png);"
"}");
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

    const std::optional<std::reference_wrapper<ModuleIntf>> activeModule = d->moduleManager->getActiveModule();
    if (activeModule) {
        ui->moduleGroupBox->setTitle(activeModule->get().getModuleName());
    }

    int moduleCount {0};
    for (const auto &item : d->moduleManager->getActionRegistry()) {
        if (moduleCount > 0) {
            QFrame *verticalLine = new QFrame(this);
            verticalLine->setFrameShape(QFrame::VLine);
            verticalLine->setFrameShadow(QFrame::Sunken);
            ui->moduleSelectorLayout->addWidget(verticalLine);
        }

        QAction *moduleAction = item.second;
        ui->moduleMenu->addAction(moduleAction);
        ActionButton *actionButton = new ActionButton(this, ActionButton::Capitalisation::AllCaps);
        actionButton->setAction(moduleAction);
        actionButton->setFlat(true);
        ui->moduleSelectorLayout->addWidget(actionButton);

        ++moduleCount;
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
    ui->loopReplayButton->setAction(ui->loopReplayAction);

    // Common CSS: Completely flat buttons (no border) - platform dependent
    ui->replayGroupBox->setStyleSheet(Platform::getFlatButtonCss());

    // Specific CSS: completely flat button (no border) - on all platforms
    ui->loopReplayButton->setStyleSheet("QPushButton {border-style: outset; border-width: 0px; padding: 6px 12px;}");
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

    ui->normalSpeedAction->setCheckable(true);
    ui->normalSpeedAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_1));
    ui->normalSpeedAction->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Normal));

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

    ui->customSpeedAction->setCheckable(true);
    ui->customSpeedAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_6));
    ui->customSpeedAction->setProperty(ReplaySpeedProperty, Enum::toUnderlyingType(ReplaySpeed::Custom));

    // Action group
    d->replaySpeedActionGroup = new QActionGroup(this);
    for (QAction *action : slowActions) {
        action->setCheckable(true);
        d->replaySpeedActionGroup->addAction(action);
    }
    d->replaySpeedActionGroup->addAction(ui->normalSpeedAction);
    for (QAction *action : fastActions) {
        action->setCheckable(true);
        d->replaySpeedActionGroup->addAction(action);
    }
    d->replaySpeedActionGroup->addAction(ui->customSpeedAction);

    // Menus
    ui->slowMenu->addActions(slowActions);
    ui->fastMenu->addActions(fastActions);

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
    normalSpeedRadioButton->setAction(ui->normalSpeedAction);
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
    d->customSpeedRadioButton->setAction(ui->customSpeedAction);
    replaySpeedLayout->addWidget(d->customSpeedRadioButton);

    d->customSpeedLineEdit = new QLineEdit(this);
    d->customSpeedLineEdit->setMinimumWidth(::CustomSpeedLineEditMinimumWidth);
    replaySpeedLayout->addWidget(d->customSpeedLineEdit);

    d->customReplaySpeedFactorValidator = new QDoubleValidator(d->customSpeedLineEdit);
    d->customReplaySpeedFactorValidator->setRange(::ReplaySpeedAbsoluteMin, ::ReplaySpeedAbsoluteMax, ::ReplaySpeedDecimalPlaces);
    d->customReplaySpeedPercentValidator = new QDoubleValidator(d->customSpeedLineEdit);
    d->customReplaySpeedPercentValidator->setRange(::ReplaySpeedAbsoluteMin * 100.0, ::ReplaySpeedAbsoluteMax * 100.0, ::ReplaySpeedDecimalPlaces);

    // The replay speed factor in SkyConnect is always an absolute factor
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const double factor = skyConnectManager.getReplaySpeedFactor();
    Settings &settings = Settings::getInstance();
    if (settings.getReplaySpeeedUnit() == Replay::SpeedUnit::Absolute) {
        d->lastCustomReplaySpeedFactor = factor;
    } else {
        d->lastCustomReplaySpeedFactor = factor * 100.0;
    }
    if (qFuzzyCompare(factor, 1.0)) {
        normalSpeedRadioButton->setChecked(true);
    } else {
        d->customSpeedRadioButton ->setChecked(true);
        d->customSpeedLineEdit->setText(d->unit.formatNumber(d->lastCustomReplaySpeedFactor, ReplaySpeedDecimalPlaces));
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

FlightDialog &MainWindow::getFlightDialog() noexcept
{
    if (d->flightDialog == nullptr) {
        d->flightDialog = new FlightDialog(*d->flightService, this);
        d->flightDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(d->flightDialog, &FlightDialog::visibilityChanged,
                this, &MainWindow::updateWindowMenu);
        connect(d->flightDialog, &QWidget::destroyed,
                this, &MainWindow::onFlightDialogDeleted);
    }
    return *d->flightDialog;
}

inline bool MainWindow::hasFlightDialog() const noexcept
{
    return d->flightDialog != nullptr;
}

SimulationVariablesDialog &MainWindow::getSimulationVariablesDialog() noexcept
{
    if (d->simulationVariablesDialog == nullptr) {
        d->simulationVariablesDialog = new SimulationVariablesDialog(this);
        d->simulationVariablesDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(d->simulationVariablesDialog, &SimulationVariablesDialog::visibilityChanged,
                this, &MainWindow::updateWindowMenu);
        connect(d->simulationVariablesDialog, &QWidget::destroyed,
                this, &MainWindow::onSimulationVariablesDialogDeleted);
    }
    return *d->simulationVariablesDialog;
}

inline bool MainWindow::hasSimulationVariablesDialog() const noexcept
{
    return d->simulationVariablesDialog != nullptr;
}

StatisticsDialog &MainWindow::getStatisticsDialog() noexcept
{
    if (d->statisticsDialog == nullptr) {
        d->statisticsDialog = new StatisticsDialog(this);
        d->statisticsDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(d->statisticsDialog, &StatisticsDialog::visibilityChanged,
                this, &MainWindow::updateWindowMenu);
        connect(d->statisticsDialog, &QWidget::destroyed,
                this, &MainWindow::onStatisticsDialogDeleted);
    }
    return *d->statisticsDialog;
}

inline bool MainWindow::hasStatisticsDialog() const noexcept
{
    return d->statisticsDialog != nullptr;
}

void MainWindow::updateMinimalUi(bool enable)
{
    Settings &settings = Settings::getInstance();
    settings.setMinimalUiEnabled(enable);
    const bool hasModules = d->moduleManager->getActiveModule().has_value();
    ui->showMinimalAction->setEnabled(hasModules);

    const bool minimalUi = isMinimalUiEnabled();
    if (minimalUi) {
        ui->moduleVisibilityWidget->setHidden(true);
        ui->moduleSelectorWidget->setHidden(true);
        ui->showModulesAction->setChecked(false);
        ui->showModulesAction->setEnabled(false);  
    } else {
        const bool moduleSelectorVisible = settings.isModuleSelectorVisible();
        ui->moduleVisibilityWidget->setVisible(true);
        ui->moduleSelectorWidget->setVisible(moduleSelectorVisible);
        ui->showModulesAction->setChecked(moduleSelectorVisible);
        ui->showModulesAction->setEnabled(true);
    }
    updateMinimalUiButtonTextVisibility();
    updateMinimalUiEssentialButtonVisibility();
    updateReplaySpeedVisibility(minimalUi);
    updatePositionSliderTickInterval();
    ui->moduleGroupBox->setHidden(minimalUi);

    // When hiding a widget it takes some time for the layout manager to
    // get notified, so we return to the Qt event queue first
    QTimer::singleShot(0, this, &MainWindow::updateWindowSize);
}

bool MainWindow::isMinimalUiEnabled() const noexcept
{
    Settings &settings = Settings::getInstance();
    const bool minimalUi = settings.isMinimalUiEnabled();
    const bool hasModules = d->moduleManager->getActiveModule().has_value();
    // Also enable minimal UI mode if no module plugins are available
    return minimalUi || !hasModules;
}

void MainWindow::updateReplaySpeedUi() noexcept
{
    if (d->customSpeedRadioButton->isChecked()) {
        d->customSpeedLineEdit->setEnabled(true);
        d->customSpeedLineEdit->setText(d->unit.formatNumber(d->lastCustomReplaySpeedFactor, ReplaySpeedDecimalPlaces));

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
    int sliderPosition {0};
    if (SkyConnectManager::getInstance().isInRecordingState()) {
        sliderPosition = ::PositionSliderMax;
        ui->positionSlider->setToolTip(tr("Recording"));
    } else {
        const std::int64_t totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
        const std::int64_t ts = std::min(timestamp, totalDuration);
        if (ts > 0) {
            sliderPosition = static_cast<int>(std::round(::PositionSliderMax * (static_cast<double>(ts) / static_cast<double>(totalDuration))));
        }
        ui->positionSlider->setToolTip(tr("%1 ms (%2)").arg(d->unit.formatTimestamp(timestamp), d->unit.formatHHMMSS(timestamp)));
    }

    ui->positionSlider->blockSignals(true);
    ui->positionSlider->setValue(sliderPosition);
    ui->positionSlider->blockSignals(false);

    const QTime time = QTime::fromMSecsSinceStartOfDay(timestamp);
    ui->timestampTimeEdit->blockSignals(true);
    ui->timestampTimeEdit->setTime(time);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::updateMinimalUiButtonTextVisibility() noexcept
{
    if (isMinimalUiEnabled()) {
        const bool buttonTextVisible = Settings::getInstance().getDefaultMinimalUiButtonTextVisibility();
        ui->recordButton->setShowText(buttonTextVisible);
        ui->skipToBeginButton->setShowText(buttonTextVisible);
        ui->backwardButton->setShowText(buttonTextVisible);
        ui->stopButton->setShowText(buttonTextVisible);
        ui->pauseButton->setShowText(buttonTextVisible);
        ui->playButton->setShowText(buttonTextVisible);
        ui->forwardButton->setShowText(buttonTextVisible);
        ui->skipToEndButton->setShowText(buttonTextVisible);
        ui->loopReplayButton->setShowText(buttonTextVisible);
    } else {
        ui->recordButton->setShowText(true);
        ui->skipToBeginButton->setShowText(true);
        ui->backwardButton->setShowText(true);
        ui->stopButton->setShowText(true);
        ui->pauseButton->setShowText(true);
        ui->playButton->setShowText(true);
        ui->forwardButton->setShowText(true);
        ui->skipToEndButton->setShowText(true);
        ui->loopReplayButton->setShowText(true);
    }
}

void MainWindow::updateMinimalUiEssentialButtonVisibility() noexcept
{
    if (isMinimalUiEnabled()) {
        const bool nonEssentialButtonVisible = Settings::getInstance().getDefaultMinimalUiNonEssentialButtonVisibility();
        ui->skipToBeginButton->setVisible(nonEssentialButtonVisible);
        ui->backwardButton->setVisible(nonEssentialButtonVisible);
        ui->skipToEndButton->setVisible(nonEssentialButtonVisible);
        ui->forwardButton->setVisible(nonEssentialButtonVisible);
        ui->skipToEndButton->setVisible(nonEssentialButtonVisible);
    } else {
        ui->skipToBeginButton->setVisible(true);
        ui->backwardButton->setVisible(true);
        ui->skipToEndButton->setVisible(true);
        ui->forwardButton->setVisible(true);
        ui->skipToEndButton->setVisible(true);
    }
}

void MainWindow::updateReplaySpeedVisibility(bool enterMinimalUi) noexcept
{
    Settings &settings = Settings::getInstance();
    bool replaySpeedVisible;
    if (enterMinimalUi) {
        // When switching to minimal UI mode the default replay speed visibility takes precedence
        replaySpeedVisible = settings.getDefaultMinimalUiReplaySpeedVisibility() && settings.isReplaySpeedVisible();
    } else {
        // The current replay speed visibility setting decides (only)
        replaySpeedVisible = settings.isReplaySpeedVisible();
    }
    ui->showReplaySpeedAction->setChecked(replaySpeedVisible);
    ui->replaySpeedGroupBox->setVisible(replaySpeedVisible);
}

void MainWindow::updatePositionSliderTickInterval() noexcept
{
    int tickInterval {10};
    if (isMinimalUiEnabled()) {
        if (!ui->showReplaySpeedAction->isChecked()) {
            Settings &settings = Settings::getInstance();
            if (settings.getDefaultMinimalUiButtonTextVisibility()) {
                if (settings.getDefaultMinimalUiButtonTextVisibility()) {
                    tickInterval = 10;
                } else {
                    tickInterval = 20;
                }
            } else {
                if (settings.getDefaultMinimalUiButtonTextVisibility()) {
                    tickInterval = 20;
                } else {
                    tickInterval = 40;
                }
            }
        } else {
            tickInterval = 10;
        }
    }
    ui->positionSlider->setTickInterval(tickInterval);
}

double MainWindow::getCustomSpeedFactor() const
{
    double customSpeedFactor {1.0};
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
    }
    return customSpeedFactor;
}

// PRIVATE SLOTS

void MainWindow::onPositionSliderPressed() noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    d->previousState = skyConnectManager.getState();
    if (d->previousState == Connect::State::Replay) {
        // Pause the replay while sliding the position slider
        skyConnectManager.setPaused(true);
    }
}

void MainWindow::onPositionSliderValueChanged(int value) noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const double factor = static_cast<double>(value) / static_cast<double>(PositionSliderMax);
    const std::int64_t totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
    const std::int64_t timestamp = static_cast<std::int64_t>(std::round(factor * static_cast<double>(totalDuration)));

    // Prevent the timestampTimeEdit field to set the replay position as well
    ui->timestampTimeEdit->blockSignals(true);
    skyConnectManager.seek(timestamp);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::onPositionSliderReleased() noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    if (d->previousState == Connect::State::Replay) {
        skyConnectManager.setPaused(false);
    }
}

void MainWindow::onTimeStampTimeEditChanged(const QTime &time) noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    if (skyConnectManager.isIdle() || skyConnectManager.getState() == Connect::State::ReplayPaused) {
        std::int64_t timestamp = time.hour() * MilliSecondsPerHour + time.minute() * MilliSecondsPerMinute + time.second() * MilliSecondsPerSecond;
        skyConnectManager.seek(timestamp);
    }
}

void MainWindow::updateWindowSize() noexcept
{
    const bool minimalUi = isMinimalUiEnabled();
    if (minimalUi) {
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

void MainWindow::onTimestampChanged(std::int64_t timestamp) noexcept
{
    if (SkyConnectManager::getInstance().isInRecordingState()) {
        updateRecordingDuration(timestamp);
    } else {
        updatePositionSlider(timestamp);
    };
}

void MainWindow::onReplaySpeedSelected(QAction *action) noexcept
{
    ReplaySpeed replaySpeed = static_cast<ReplaySpeed>(action->property(ReplaySpeedProperty).toInt());
    double replaySpeedFactor {1.0};
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

    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    skyConnectManager.setReplaySpeedFactor(replaySpeedFactor);

    updateReplaySpeedUi();
}

void MainWindow::onCustomSpeedChanged() noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const double customReplaySpeedFactor = getCustomSpeedFactor();
    skyConnectManager.setReplaySpeedFactor(customReplaySpeedFactor);
    switch (Settings::getInstance().getReplaySpeeedUnit()) {
    case Replay::SpeedUnit::Absolute:
        d->lastCustomReplaySpeedFactor = customReplaySpeedFactor;
        break;
    case Replay::SpeedUnit::Percent:
        d->lastCustomReplaySpeedFactor = customReplaySpeedFactor * 100.0;
        break;
    }
}

void MainWindow::onReplaySpeedUnitSelected(int index) noexcept
{
    Settings &settings = Settings::getInstance();
    Replay::SpeedUnit replaySpeedUnit = static_cast<Replay::SpeedUnit>(d->replaySpeedUnitComboBox->itemData(index).toInt());
    switch (replaySpeedUnit) {
    case Replay::SpeedUnit::Absolute:
        if (settings.getReplaySpeeedUnit() != Replay::SpeedUnit::Absolute) {
            // Percent to absolute factor
            d->lastCustomReplaySpeedFactor /= 100.0;
        }
        break;
    case Replay::SpeedUnit::Percent:
        if (settings.getReplaySpeeedUnit() != Replay::SpeedUnit::Percent) {
            // Absolute factor to percent
            d->lastCustomReplaySpeedFactor *= 100.0;
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
    updateFileMenu();
    updateModuleActions();
    updateWindowMenu();
    updateMainWindow();
}

void MainWindow::updateControlUi() noexcept
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const bool hasRecording = aircraft.hasRecording();

    const SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const bool hasSkyConnectPlugins = skyConnectManager.hasPlugins();
    switch (skyConnectManager.getState()) {
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
    ui->loopReplayAction->setChecked(loopReplayEnabled);
}

void MainWindow::updateControlIcons() noexcept
{
    const QIcon &recordIcon = d->moduleManager->getRecordIcon();
    ui->recordAction->setIcon(recordIcon);
}

void MainWindow::onDefaultMinimalUiButtonTextVisibilityChanged(bool visible) noexcept
{
    updateMinimalUiButtonTextVisibility();
    updatePositionSliderTickInterval();
    if (!visible && isMinimalUiEnabled()) {
        // Shrink to minimal size
        QTimer::singleShot(0, this, &MainWindow::updateWindowSize);
    }
}

void MainWindow::onDefaultMinimalUiEssentialButtonVisibilityChanged(bool visible) noexcept
{
    updateMinimalUiEssentialButtonVisibility();
    updatePositionSliderTickInterval();
    if (!visible && isMinimalUiEnabled()) {
        // Shrink to minimal size
        QTimer::singleShot(0, this, &MainWindow::updateWindowSize);
    }
}

void MainWindow::onRecordingDurationChanged() noexcept
{
    updateReplayDuration();
    updatePositionSlider(SkyConnectManager::getInstance().getCurrentTimestamp());
}

void MainWindow::updateReplayDuration() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const std::int64_t totalDuration = flight.getTotalDurationMSec();
    const QTime time = QTime::fromMSecsSinceStartOfDay(totalDuration);
    ui->timestampTimeEdit->blockSignals(true);
    ui->timestampTimeEdit->setMaximumTime(time);
    ui->timestampTimeEdit->blockSignals(false);
}

void MainWindow::updateFileMenu() noexcept
{
    const Aircraft &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const bool hasRecording = aircraft.hasRecording();
    if (SkyConnectManager::getInstance().isInRecordingState()) {
        ui->newLogbookAction->setEnabled(false);
        ui->openLogbookAction->setEnabled(false);
        ui->flightImportMenu->setEnabled(false);
        ui->flightExportMenu->setEnabled(false);
        ui->locationImportMenu->setEnabled(false);
        ui->locationExportMenu->setEnabled(false);
        ui->optimiseLogbookAction->setEnabled(false);
    } else {
        ui->newLogbookAction->setEnabled(true);
        ui->openLogbookAction->setEnabled(true);
        ui->flightImportMenu->setEnabled(d->hasFlightImportPlugins && d->connectedWithLogbook);
        ui->flightExportMenu->setEnabled(d->hasFlightExportPlugins && hasRecording);
        ui->locationImportMenu->setEnabled(d->hasFlightImportPlugins && d->connectedWithLogbook);
        ui->locationExportMenu->setEnabled(d->hasFlightExportPlugins && d->connectedWithLogbook);
        ui->optimiseLogbookAction->setEnabled(d->connectedWithLogbook);
    }
}

void MainWindow::updateModuleActions() noexcept
{
    const bool recording = SkyConnectManager::getInstance().isInRecordingState();
    for (auto &item : d->moduleManager->getActionRegistry()) {
        item.second->setEnabled(!recording);
    }
}

void MainWindow::updateWindowMenu() noexcept
{
    bool visible = hasFlightDialog() ?  getFlightDialog().isVisible() : false;
    ui->showFlightAction->setChecked(visible);

    visible = hasSimulationVariablesDialog() ?  getSimulationVariablesDialog().isVisible() : false;
    ui->showSimulationVariablesAction->setChecked(visible);

    visible = hasStatisticsDialog() ?  getStatisticsDialog().isVisible() : false;
    ui->showStatisticsAction->setChecked(visible);
}

void MainWindow::updateMainWindow() noexcept
{
    const Settings &settings = Settings::getInstance();
    const Qt::WindowFlags flags = windowFlags();
    if (settings.isWindowStaysOnTopEnabled() != flags.testFlag(Qt::WindowStaysOnTopHint)) {
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

void MainWindow::onModuleActivated(const QString title, [[maybe_unused]] QUuid uuid) noexcept
{
    ui->moduleGroupBox->setTitle(title);
    // Disable the minimal UI (if activated)
    ui->showMinimalAction->setChecked(false);
    updateControlIcons();
}

// File menu

void MainWindow::createNewLogbook() noexcept
{
    const QString logbookPath = DatabaseService::getNewLogbookPath(this);
    if (!logbookPath.isNull()) {
        const bool ok = PersistenceManager::getInstance().connectWithLogbook(logbookPath, this);
        if (!ok) {
            QMessageBox::critical(this, tr("Logbook error"), tr("The logbook %1 could not be created.").arg(QDir::toNativeSeparators(logbookPath)));
        }
    }
}

void MainWindow::openLogbook() noexcept
{
    QString filePath = DatabaseService::getExistingLogbookPath(this);
    if (!filePath.isEmpty()) {
        connectWithLogbook(filePath);
    }
}

void MainWindow::optimiseLogbook() noexcept
{
    PersistenceManager &persistenceManager = PersistenceManager::getInstance();
    QString logbookPath = persistenceManager.getLogbookPath();
    QFileInfo fileInfo = QFileInfo(logbookPath);

    std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
    messageBox->setIcon(QMessageBox::Question);
    const std::int64_t oldSize = fileInfo.size();
    QPushButton *optimiseButton = messageBox->addButton(tr("&Optimise"), QMessageBox::AcceptRole);
    messageBox->setWindowTitle(tr("Optimise Logbook"));
    messageBox->setText(tr("Logbook optimisation will regain unused space. The current %1 size is %2. Do you want to optimise the logbook?").arg(fileInfo.fileName(), d->unit.formatMemory(oldSize)));
    messageBox->setInformativeText(tr("The optimisation operation may take a while, depending on the logbook file size."));
    messageBox->setStandardButtons(QMessageBox::Cancel);
    messageBox->setDefaultButton(optimiseButton);

    messageBox->exec();
    const QAbstractButton *clickedButton = messageBox->clickedButton();
    if (clickedButton == optimiseButton) {
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        const bool ok = persistenceManager.optimise();
        QGuiApplication::restoreOverrideCursor();
        if (ok) {
            fileInfo.refresh();
            messageBox = std::make_unique<QMessageBox>(this);
            messageBox->setIcon(QMessageBox::Information);
            messageBox->setWindowTitle(tr("Success"));
            messageBox->setText(tr("The logbook %1 optimisation was successful.").arg(fileInfo.fileName()));
            messageBox->setInformativeText(tr("The new file size is: %1 (previous size: %2).")
                                           .arg(d->unit.formatMemory(fileInfo.size()), d->unit.formatMemory(oldSize)));
            messageBox->exec();
        } else {
            QMessageBox::critical(this, tr("Logbook error"), tr("The logbook could not be optimised."));
        }
    }
}

void MainWindow::showSettings() noexcept
{
    std::unique_ptr<SettingsDialog> settingsDialog = std::make_unique<SettingsDialog>(this);
    settingsDialog->exec();
}

void MainWindow::showLogbookSettings() noexcept
{
    std::unique_ptr<LogbookSettingsDialog> logbookSettingsDialog = std::make_unique<LogbookSettingsDialog>(this);
    logbookSettingsDialog->exec();
}

void MainWindow::quit() noexcept
{
    SkyConnectManager::getInstance().stop();
    close();
}

// View menu

void MainWindow::onShowModulesChanged(bool enable) noexcept
{
    Settings &settings = Settings::getInstance();
    settings.setModuleSelectorVisible(enable);
    ui->moduleSelectorWidget->setVisible(enable);
}

void MainWindow::onShowReplaySpeedChanged(bool enable) noexcept
{
    Settings &settings = Settings::getInstance();
    settings.setReplaySpeedVisible(enable);
    updateReplaySpeedVisibility(false);
    updatePositionSliderTickInterval();

    // Readjust size (minimum size when in minimal UI mode)
    QTimer::singleShot(0, this, &MainWindow::updateWindowSize);
}

// Window menu

void MainWindow::toggleFlightDialog(bool enable) noexcept
{
    FlightDialog &dialog = getFlightDialog();
    dialog.setVisible(enable);
}

void MainWindow::onFlightDialogDeleted() noexcept
{
    d->flightDialog = nullptr;
}

void MainWindow::toggleSimulationVariablesDialog(bool enable) noexcept
{
    SimulationVariablesDialog &dialog = getSimulationVariablesDialog();
    dialog.setVisible(enable);
}

void MainWindow::onSimulationVariablesDialogDeleted() noexcept
{
    d->simulationVariablesDialog = nullptr;
}

void MainWindow::toggleStatisticsDialog(bool enable) noexcept
{
    StatisticsDialog &dialog = getStatisticsDialog();
    dialog.setVisible(enable);
}

void MainWindow::onStatisticsDialogDeleted() noexcept
{
    d->statisticsDialog = nullptr;
}

void MainWindow::toggleStayOnTop(bool enable) noexcept
{
    Settings::getInstance().setWindowStaysOnTopEnabled(enable);
}

void MainWindow::toggleMinimalUi(bool enable) noexcept
{
    updateMinimalUi(enable);
}

// Help menu

void MainWindow::showAboutDialog() noexcept
{
    std::unique_ptr<AboutDialog> aboutDialog = std::make_unique<AboutDialog>(this);
    aboutDialog->exec();
}

void MainWindow::showOnlineManual() const noexcept
{
    QDesktopServices::openUrl(QUrl("https://till213.github.io/SkyDolly/manual/en/"));
}

// Replay

void MainWindow::toggleRecord(bool enable) noexcept
{
    d->moduleManager->setRecording(enable);
}

void MainWindow::togglePause(bool enable) noexcept
{
    d->moduleManager->setPaused(enable);
}

void MainWindow::togglePlay(bool enable) noexcept
{
     d->moduleManager->setPlaying(enable);
}

void MainWindow::stop() noexcept
{
    SkyConnectManager::getInstance().stop();
}

// Transport

void MainWindow::skipToBegin() noexcept
{
    SkyConnectManager::getInstance().skipToBegin();
}

void MainWindow::skipBackward() noexcept
{
    SkyConnectManager::getInstance().skipBackward();
}

void MainWindow::skipForward() noexcept
{
    SkyConnectManager::getInstance().skipForward();
}

void MainWindow::skipToEnd() noexcept
{
    SkyConnectManager::getInstance().skipToEnd();
}

void MainWindow::toggleLoopReplay(bool checked) noexcept
{
    Settings::getInstance().setLoopReplayEnabled(checked);
}

// Service

void MainWindow::onFlightRestored() noexcept
{
    updateUi();
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    skyConnectManager.skipToBegin();
    if (skyConnectManager.isConnected()) {
        // Make sure we are unpaused...
        d->moduleManager->setPaused(false);
        // ... play the first frame (which will "move" to the new location)...
        d->moduleManager->setPlaying(true);
        // ... and pause again (such that the new scenery can be loaded)
        d->moduleManager->setPaused(true);
    }
}

void MainWindow::onLogbookConnectionChanged(bool connected) noexcept
{
    d->connectedWithLogbook = connected;
    updateUi();
}

void MainWindow::onFlightImport(QAction *action) noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    const QUuid pluginUuid = action->data().toUuid();
    const bool ok = PluginManager::getInstance().importFlight(pluginUuid, *d->flightService, flight);
    if (ok) {
        updateUi();
        SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
        skyConnectManager.skipToBegin();
        if (skyConnectManager.isConnected()) {
            d->moduleManager->setPlaying(true);
            d->moduleManager->setPaused(true);
        }
    }
}

void MainWindow::onFlightExport(QAction *action) noexcept
{
    const QUuid pluginUuid = action->data().toUuid();
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    PluginManager::getInstance().exportFlight(flight, pluginUuid);
}

void MainWindow::onLocationImport(QAction *action) noexcept
{
    const QUuid pluginUuid = action->data().toUuid();
    PluginManager::getInstance().importLocations(pluginUuid, *d->locationService);
}

void MainWindow::onLocationExport(QAction *action) noexcept
{
    const QUuid pluginUuid = action->data().toUuid();
    PluginManager::getInstance().exportLocations(pluginUuid, *d->locationService);
}

void MainWindow::onReplayLoopChanged() noexcept
{
    updateControlUi();
}
