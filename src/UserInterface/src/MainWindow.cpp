/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <optional>
#include <vector>
#include <cstdint>
#include <cmath>

#include <QCoreApplication>
#include <QByteArray>
#include <QList>
#include <QFileDialog>
#include <QMessageBox>
#include <QCursor>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>
#include <QUuid>
#include <QDateTime>
#include <QTimer>
#include <QTimeEdit>
#include <QComboBox>
#include <QSlider>
#include <QLineEdit>
#include <QButtonGroup>
#include <QPushButton>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QRadioButton>
#include <QLabel>
#include <QDoubleValidator>
#include <QIcon>
#include <QEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QAction>
#include <QActionGroup>
#include <QSpacerItem>
#include <QDesktopServices>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QProcess>

#include <Kernel/Unit.h>
#include <Kernel/Const.h>
#include <Kernel/Replay.h>
#include <Kernel/Version.h>
#include <Kernel/Settings.h>
#include <Kernel/Enum.h>
#include <Kernel/SampleRate.h>
#include <Kernel/SecurityToken.h>
#include <Kernel/RecentFile.h>
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
#include <Widget/TimestampEdit.h>
#include <Widget/Platform.h>
#include <Widget/RecentFileMenu.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Connect/Connect.h>
#include <PluginManager/Connect/FlightSimulatorShortcuts.h>
#include <PluginManager/PluginManager.h>
#include <PluginManager/Module/ModuleIntf.h>
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

using namespace std::chrono_literals;

namespace
{
    constexpr int PositionSliderMin {0};
    constexpr int PositionSliderMax {1000};
    constexpr double ReplaySpeedAbsoluteMin {0.01};
    // A replay speed with factor 99999 should be fast enough (one day has 86400 seconds: this allows
    // for a speed-up of one second per day)
    constexpr double ReplaySpeedAbsoluteMax {99999.0};
    constexpr double ReplaySpeedDecimalPlaces {2};

    constexpr int CustomSpeedLineEditMinimumWidth {40};

    constexpr const char *ReplaySpeedProperty {"ReplaySpeed"};

    enum struct ReplaySpeed: std::uint8_t {
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

    RecentFileMenu *recentFileMenu {nullptr};
    FlightDialog *flightDialog {nullptr};
    SimulationVariablesDialog *simulationVariablesDialog {nullptr};
    StatisticsDialog *statisticsDialog {nullptr};

    QMenu *trayIconMenu {nullptr};
    QSystemTrayIcon *trayIcon {nullptr};

    Unit unit;
    QSize lastNormalUiSize;

    // Replay speed
    QActionGroup *replaySpeedActionGroup {nullptr};
    ActionRadioButton *customSpeedRadioButton {nullptr};
    float lastCustomReplaySpeedFactor {1.0f};
    QLineEdit *customSpeedLineEdit {nullptr};
    QComboBox *replaySpeedUnitComboBox {nullptr};
    QDoubleValidator *customReplaySpeedFactorValidator {nullptr};
    QDoubleValidator *customReplaySpeedPercentValidator {nullptr};
    QLabel *simulationRateLabel {nullptr};
    float simulationRate {1.0f};

    // Import / export
    QActionGroup *flightImportActionGroup {nullptr};
    QActionGroup *flightExportActionGroup {nullptr};
    QActionGroup *locationImportActionGroup {nullptr};
    QActionGroup *locationExportActionGroup {nullptr};
    bool hasFlightImportPlugins {false};
    bool hasFlightExportPlugins {false};
    bool hasLocationImportPlugins {false};
    bool hasLocationExportPlugins {false};
    bool continuousSeek {false};

    std::unique_ptr<ModuleManager> moduleManager;
};

// PUBLIC

MainWindow::MainWindow(const QString &filePath, QWidget *parent) noexcept
    : QMainWindow {parent},
      ui {std::make_unique<Ui::MainWindow>()},
      d {std::make_unique<MainWindowPrivate>()}
{
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

MainWindow::~MainWindow()
{
    // Don't get notified about the dialog destruction in case
    // the Qt event queue tries to delete this MainWindow first
    // (which then also implicitly deletes the dialogs due to
    // them being children of this MainWindow); otherwise we
    // would try to access the invalid private data 'd' in
    // the various onXYZDeleted slots
    if (d->flightDialog != nullptr) {
        d->flightDialog->disconnect();
    }
    if (d->simulationVariablesDialog != nullptr) {
        d->simulationVariablesDialog->disconnect();
    }
    if (d->statisticsDialog != nullptr) {
        d->statisticsDialog->disconnect();
    }
}

bool MainWindow::connectWithLogbook(const QString &filePath) noexcept
{
    d->connectedWithLogbook = PersistenceManager::getInstance().connectWithLogbook(filePath, this);
    if (d->connectedWithLogbook) {
        RecentFile::getInstance().addRecentFile(filePath);
    } else {
        QMessageBox::critical(this, tr("Logbook Error"), tr("The logbook %1 could not be opened.").arg(QDir::toNativeSeparators(filePath)));
    }
    return d->connectedWithLogbook;
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

    auto &settings = Settings::getInstance();
    settings.setWindowGeometry(saveGeometry());
    settings.setWindowState(saveState());
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) noexcept
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        const QList<QUrl> urlList = mimeData->urls();
        // Accept the proposed drop action if at least one
        // URL looks to be a Sky Dolly logbook (*.sdlog)
        for (const auto &url : urlList) {
            if (url.isLocalFile() && url.fileName().endsWith(Const::LogbookExtension)) {
                event->acceptProposedAction();
                break;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event) noexcept
{
    QString filePath;
    const QMimeData *mimeData = event->mimeData();
    const QList<QUrl> urlList = mimeData->urls();
    for (const auto &url : urlList) {
        if (url.isLocalFile() && url.fileName().endsWith(Const::LogbookExtension)) {
            filePath = url.toLocalFile();
            break;
        }
    }
    if (!filePath.isEmpty()) {
        connectWithLogbook(filePath);
    }
}

// PRIVATE

void MainWindow::frenchConnection() noexcept
{
    // Sky Connect
    auto &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::timestampChanged,
            this, &MainWindow::onTimestampChanged);
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &MainWindow::updateUi);
    connect(&skyConnectManager, &SkyConnectManager::recordingStopped,
            this, &MainWindow::onRecordingStopped);
    connect(&skyConnectManager, &SkyConnectManager::actionActivated,
            this, &MainWindow::onActionActivated);
    connect(&skyConnectManager, &SkyConnectManager::simulationRateReceived,
            this, &MainWindow::onSimulationRateReceived);

    // Replay speed
    connect(d->replaySpeedActionGroup, &QActionGroup::triggered,
            this, &MainWindow::onReplaySpeedSelected);

    // Flight
    const auto &logbook = Logbook::getInstance();
    const auto &flight = logbook.getCurrentFlight();
    connect(&flight, &Flight::flightRestored,
            this, &MainWindow::onFlightRestored);
    connect(&flight, &Flight::timeOffsetChanged,
            this, &MainWindow::onRecordingDurationChanged);
    connect(&flight, &Flight::aircraftStored,
            this, &MainWindow::onRecordingDurationChanged);
    connect(&flight, &Flight::aircraftRemoved,
            this, &MainWindow::onRecordingDurationChanged);
    connect(&flight, &Flight::cleared,
            this, &MainWindow::updateUi);
    connect(&flight, &Flight::flightConditionChanged,
            this, &MainWindow::updateTimeUi);

    // Settings
    const auto &settings = Settings::getInstance();
    connect(&settings, &Settings::changed,
            this, &MainWindow::updateMainWindow);
    connect(&settings, &Settings::defaultMinimalUiButtonTextVisibilityChanged,
            this, &MainWindow::onDefaultMinimalUiButtonTextVisibilityChanged);
    connect(&settings, &Settings::defaultMinimalUiNonEssentialButtonVisibilityChanged,
            this, &MainWindow::onDefaultMinimalUiEssentialButtonVisibilityChanged);
    connect(&settings, &Settings::replayLoopChanged,
            this, &MainWindow::onReplayLoopChanged);
    connect(&settings, &Settings::styleKeyChanged,
            this, &MainWindow::onStyleKeyChanged);

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
    connect(ui->timestampEdit, &TimestampEdit::timestampChanged,
            this, &MainWindow::onTimestampEditChanged);

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
    connect(ui->clearFlightAction, &QAction::triggered,
            this, &MainWindow::clearFlight);

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

    // Recent files
    connect(&RecentFile::getInstance(), &RecentFile::recentFileSelected,
            this, &MainWindow::onRecentFileSelected);
    connect(d->recentFileMenu, &RecentFileMenu::actionGroupChanged,
            this, &MainWindow::updateRecentFileMenu);

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
    const auto &settings = Settings::getInstance();
    setWindowIcon(QIcon(":/img/icons/application-icon.png"));

    // File menu
    d->recentFileMenu = new RecentFileMenu(this);
    updateRecentFileMenu();

    // Window menu
    ui->stayOnTopAction->setChecked(settings.isWindowStaysOnTopEnabled());

    initModuleSelectorUi();
    initViewUi();
    initControlUi();
    initReplaySpeedUi();
    createTrayIcon();

    const bool minimalUi = isMinimalUiEnabled();
    ui->showMinimalAction->setChecked(minimalUi);
    updateMinimalUi(minimalUi);

    QByteArray windowGeometry = settings.getWindowGeometry();
    QByteArray windowState = settings.getWindowState();
    if (!windowGeometry.isEmpty()) {
        restoreGeometry(windowGeometry);
        restoreState(windowState);
    }

    // Drag and drop support
    setAcceptDrops(true);

    const int previewInfoCount = settings.getPreviewInfoDialogCount();
    if (previewInfoCount > 0) {
        QTimer::singleShot(0, this, [this]() {
            auto &settings = Settings::getInstance();
            int currentPreviewInfoCount = settings.getPreviewInfoDialogCount();
            --currentPreviewInfoCount;
            QMessageBox::information(
                this,
                "Preview",
                QStringLiteral(
                    "%1 is in a preview release phase: while it should be stable to use it is not "
                    "considered feature-complete.\n\n"
                    "This release v%2 \"%3\" does not provide many new features. Instead it intends to spread "
                    "the Olympic spirit by providing over 200 hand-selected locations of each "
                    "country being represented by their Olympic athletes in the Olympic games in France 2024.\n\n"
                    "While under a tight time budget I tried to find visually attractive places in each "
                    "country, taking into consideration the actual representation in MS Flight Simulator itself.\n\n"
                    "Sky Dolly invites everyone to take a moment and marvel at the diversity and beauty of our planet - happy flying!\n\n"
                    "This dialog will be shown %4 more times.")
                    .arg(Version::getApplicationName(), Version::getApplicationVersion())
                    .arg(Version::getCodeName())
                    .arg(currentPreviewInfoCount),
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

    auto &pluginManager = PluginManager::getInstance();
    pluginManager.initialise(this);

    // Flight import
    flightImportPlugins = pluginManager.initialiseFlightImportPlugins();
    d->hasFlightImportPlugins = flightImportPlugins.size() > 0;
    if (d->hasFlightImportPlugins) {
        ui->flightImportMenu->setEnabled(true);
        for (const auto &handle : flightImportPlugins) {
            auto flightImportAction = new QAction(handle.second, ui->flightImportMenu);
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
        for (const auto &handle : flightExportPlugins) {
            auto flightExportAction = new QAction(handle.second, ui->flightExportMenu);
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
        for (const auto &handle : locationImportPlugins) {
            auto locationImportAction = new QAction(handle.second, ui->locationImportMenu);
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
        for (const auto &handle : locationExportPlugins) {
            auto locationExportAction = new QAction(handle.second, ui->locationExportMenu);
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
    auto actionCheckBox = new ActionCheckBox(false, this);
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

    const std::optional<std::reference_wrapper<ModuleIntf>> activeModule = d->moduleManager->getActiveModule();
    if (activeModule) {
        ui->moduleGroupBox->setTitle(activeModule->get().getModuleName());
    }

    int moduleCount {0};
    for (const auto &item : d->moduleManager->getActionRegistry()) {
        if (moduleCount > 0) {
            auto *verticalLine = new QFrame(this);
            verticalLine->setFrameShape(QFrame::VLine);
            verticalLine->setFrameShadow(QFrame::Sunken);
            ui->moduleSelectorLayout->addWidget(verticalLine);
        }

        QAction *moduleAction = item.second;
        ui->moduleMenu->addAction(moduleAction);
        auto actionButton = new ActionButton(this, ActionButton::Capitalisation::AllCaps);
        actionButton->setAction(moduleAction);
        actionButton->setFlat(true);
        ui->moduleSelectorLayout->addWidget(actionButton);

        ++moduleCount;
    }
}

void MainWindow::initViewUi() noexcept
{
    const auto &settings = Settings::getInstance();

    const bool moduleSelectorVisible = settings.isModuleSelectorVisible();
    ui->showModulesAction->setChecked(moduleSelectorVisible);
    const bool replaySpeedVisible = settings.isReplaySpeedVisible();
    ui->showReplaySpeedAction->setChecked(replaySpeedVisible);
    ui->replaySpeedGroupBox->setVisible(replaySpeedVisible);
}

void MainWindow::initControlUi() noexcept
{
    ui->positionSlider->setRange(::PositionSliderMin, ::PositionSliderMax);

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
    QList<QAction *> slowActions{new QAction("10 %", this),
                                 new QAction("25 %", this),
                                 new QAction("50 %", this),
                                 new QAction("75 %", this)};
    slowActions.at(0)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_1));
    slowActions.at(0)->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Slow10));
    slowActions.at(1)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_2));
    slowActions.at(1)->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Slow25));
    slowActions.at(2)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_3));
    slowActions.at(2)->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Slow50));
    slowActions.at(3)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_4));
    slowActions.at(3)->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Slow75));

    ui->normalSpeedAction->setCheckable(true);
    ui->normalSpeedAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_1));
    ui->normalSpeedAction->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Normal));

    QList<QAction *> fastActions{new QAction("2x", this),
                                 new QAction("4x", this),
                                 new QAction("8x", this),
                                 new QAction("16x", this)};
    fastActions.at(0)->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_2));
    fastActions.at(0)->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Fast2x));
    fastActions.at(1)->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_3));
    fastActions.at(1)->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Fast4x));
    fastActions.at(2)->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_4));
    fastActions.at(2)->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Fast8x));
    fastActions.at(3)->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_5));
    fastActions.at(3)->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Fast16x));

    ui->customSpeedAction->setCheckable(true);
    ui->customSpeedAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_6));
    ui->customSpeedAction->setProperty(::ReplaySpeedProperty, Enum::underly(ReplaySpeed::Custom));

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
    auto slow10RadioButton = new ActionRadioButton(this);
    slow10RadioButton->setAction(slowActions.at(0));
    replaySpeedLayout->addWidget(slow10RadioButton);

    auto slow25RadioButton = new ActionRadioButton(this);
    slow25RadioButton->setAction(slowActions.at(1));
    replaySpeedLayout->addWidget(slow25RadioButton);

    auto slow50RadioButton = new ActionRadioButton(this);
    slow50RadioButton->setAction(slowActions.at(2));
    replaySpeedLayout->addWidget(slow50RadioButton);

    auto slow75RadioButton = new ActionRadioButton(this);
    slow75RadioButton->setAction(slowActions.at(3));
    replaySpeedLayout->addWidget(slow75RadioButton);

    auto normalSpeedRadioButton = new ActionRadioButton(this);
    normalSpeedRadioButton->setAction(ui->normalSpeedAction);
    replaySpeedLayout->addWidget(normalSpeedRadioButton);

    auto fast2xRadioButton = new ActionRadioButton(this);
    fast2xRadioButton->setAction(fastActions.at(0));
    replaySpeedLayout->addWidget(fast2xRadioButton);

    auto fast4xRadioButton = new ActionRadioButton(this);
    fast4xRadioButton->setAction(fastActions.at(1));
    replaySpeedLayout->addWidget(fast4xRadioButton);

    auto fast8xRadioButton = new ActionRadioButton(this);
    fast8xRadioButton->setAction(fastActions.at(2));
    replaySpeedLayout->addWidget(fast8xRadioButton);

    auto fast16xRadioButton = new ActionRadioButton(this);
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
    auto &skyConnectManager = SkyConnectManager::getInstance();
    const float factor = skyConnectManager.getReplaySpeedFactor();
    const auto &settings = Settings::getInstance();
    if (settings.getReplaySpeeedUnit() == Replay::SpeedUnit::Absolute) {
        d->lastCustomReplaySpeedFactor = factor;
    } else {
        d->lastCustomReplaySpeedFactor = factor * 100.0f;
    }
    if (qFuzzyCompare(factor, 1.0f)) {
        normalSpeedRadioButton->setChecked(true);
    } else {
        d->customSpeedRadioButton ->setChecked(true);
        d->customSpeedLineEdit->setText(d->unit.formatNumber(d->lastCustomReplaySpeedFactor, ::ReplaySpeedDecimalPlaces));
    }

    d->replaySpeedUnitComboBox = new QComboBox(this);
    d->replaySpeedUnitComboBox->addItem(tr("Factor [f]"), Enum::underly(Replay::SpeedUnit::Absolute));
    d->replaySpeedUnitComboBox->addItem(tr("Percent [%]"), Enum::underly(Replay::SpeedUnit::Percent));
    switch (Settings::getInstance().getReplaySpeeedUnit()) {
    case Replay::SpeedUnit::Absolute:
        d->replaySpeedUnitComboBox->setCurrentIndex(0);
        break;
    case Replay::SpeedUnit::Percent:
        d->replaySpeedUnitComboBox->setCurrentIndex(1);
        break;
    }

    replaySpeedLayout->addWidget(d->replaySpeedUnitComboBox);

    d->simulationRateLabel = new QLabel(this);
    replaySpeedLayout->addWidget(d->simulationRateLabel);
}

void MainWindow::createTrayIcon() noexcept
{
    d->trayIconMenu = new QMenu(this);
    d->trayIconMenu->addAction(ui->recordAction);
    d->trayIconMenu->addAction(ui->playAction);
    d->trayIconMenu->addAction(ui->stopAction);
    d->trayIconMenu->addSeparator();
    d->trayIconMenu->addAction(ui->quitAction);

    d->trayIcon = new QSystemTrayIcon(this);
    d->trayIcon->setContextMenu(d->trayIconMenu);

    d->trayIcon->setIcon(QIcon(":/img/icons/application-icon.png"));
}

void MainWindow::initSkyConnectPlugin() noexcept
{
    auto &skyConnectManager = SkyConnectManager::getInstance();
    std::vector<SkyConnectManager::Handle> skyConnectPlugins = skyConnectManager.initialisePlugins();
    if (skyConnectPlugins.empty()) {
        QMessageBox::warning(this, tr("No valid connection plugin found"), tr("No valid connection plugin has been found in the plugin directory! Sky Dolly will launch with reduced functionality."));
    }
}

FlightDialog &MainWindow::getFlightDialog() noexcept
{
    if (d->flightDialog == nullptr) {
        d->flightDialog = new FlightDialog(this);
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
    auto &settings = Settings::getInstance();
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
    ui->timestampEdit->setMinimalUiEnabled(enable);

    // When hiding a widget it takes some time for the layout manager to
    // get notified, so we return to the Qt event queue first
    QTimer::singleShot(0, this, &MainWindow::updateWindowSize);
}

bool MainWindow::isMinimalUiEnabled() const noexcept
{
    const auto &settings = Settings::getInstance();
    const bool minimalUi = settings.isMinimalUiEnabled();
    const bool hasModules = d->moduleManager->getActiveModule().has_value();
    // Also enable minimal UI mode if no module plugins are available
    return minimalUi || !hasModules;
}

void MainWindow::updateReplaySpeedUi() noexcept
{
    if (d->customSpeedRadioButton->isChecked()) {
        d->customSpeedLineEdit->setEnabled(true);
        d->customSpeedLineEdit->setText(d->unit.formatNumber(d->lastCustomReplaySpeedFactor, ::ReplaySpeedDecimalPlaces));

        switch (Settings::getInstance().getReplaySpeeedUnit()) {
        case Replay::SpeedUnit::Absolute:
            d->customSpeedLineEdit->setToolTip(tr("Custom replay speed factor in [%L1, %L2].").arg(::ReplaySpeedAbsoluteMin).arg(::ReplaySpeedAbsoluteMax));
            d->customSpeedLineEdit->setValidator(d->customReplaySpeedFactorValidator);
            break;
        case Replay::SpeedUnit::Percent:
            d->customSpeedLineEdit->setToolTip(tr("Custom replay speed % in [%L1%, %L2%].").arg(::ReplaySpeedAbsoluteMin * 100.0).arg(::ReplaySpeedAbsoluteMax * 100.0));
            d->customSpeedLineEdit->setValidator(d->customReplaySpeedPercentValidator);
            break;
        }

    } else {
        d->customSpeedLineEdit->setEnabled(false);
        d->customSpeedLineEdit->clear();
        d->customSpeedLineEdit->setToolTip("");
    }

    updateSimulationRateLabel();
}

void MainWindow::updateRecordingDuration(std::int64_t timestamp) noexcept
{
    ui->timestampEdit->blockSignals(true);
    ui->timestampEdit->setEndTimestamp(timestamp);
    ui->timestampEdit->setTimestamp(timestamp);
    ui->timestampEdit->blockSignals(false);
}

void MainWindow::updatePositionSlider(std::int64_t timestamp) noexcept
{
    int sliderPosition {0};
    if (SkyConnectManager::getInstance().isInRecordingState()) {
        sliderPosition = ::PositionSliderMax;
        ui->positionSlider->setToolTip(tr("Recording"));
    } else {
        const auto totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
        const auto ts = std::min(timestamp, totalDuration);
        if (ts > 0) {
            sliderPosition = static_cast<int>(std::round(::PositionSliderMax * (static_cast<double>(ts) / static_cast<double>(totalDuration))));
        }
        const auto &flight = Logbook::getInstance().getCurrentFlight();
        const auto &aircraft = flight.getUserAircraft();
        const auto startDate = flight.getAircraftStartLocalTime(aircraft);
        ui->positionSlider->setToolTip(tr("%1 ms (%2)").arg(d->unit.formatNumber(timestamp), d->unit.formatTimestamp(timestamp, startDate)));
    }

    ui->positionSlider->blockSignals(true);
    ui->positionSlider->setValue(sliderPosition);
    ui->positionSlider->blockSignals(false);

    ui->timestampEdit->blockSignals(true);
    ui->timestampEdit->setTimestamp(timestamp);
    ui->timestampEdit->blockSignals(false);
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
    const auto &settings = Settings::getInstance();
    bool replaySpeedVisible {false};
    if (enterMinimalUi) {
        // When switching to minimal UI mode the default replay speed visibility takes precedence
        replaySpeedVisible = settings.getDefaultMinimalUiReplaySpeedVisibility() && settings.isReplaySpeedVisible();
    } else {
        // The current replay speed visibility setting decides (only)
        replaySpeedVisible = settings.isReplaySpeedVisible();
    }
    ui->showReplaySpeedAction->setChecked(replaySpeedVisible);
    ui->replaySpeedGroupBox->setVisible(replaySpeedVisible);
    d->simulationRateLabel->setHidden(enterMinimalUi);
}

void MainWindow::updatePositionSliderTickInterval() noexcept
{
    int tickInterval {10};
    if (isMinimalUiEnabled()) {
        if (!ui->showReplaySpeedAction->isChecked()) {
            const auto &settings = Settings::getInstance();
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

float MainWindow::getCustomSpeedFactor() const
{
    float customSpeedFactor {1.0};
    const QString text = d->customSpeedLineEdit->text();
    if (!text.isEmpty()) {
        switch (Settings::getInstance().getReplaySpeeedUnit()) {
        case Replay::SpeedUnit::Absolute:
            customSpeedFactor = d->unit.toNumber(text);
            break;
        case Replay::SpeedUnit::Percent:
            customSpeedFactor = d->unit.toNumber(text) / 100.0f;
            break;
        }
    }
    return customSpeedFactor;
}

void MainWindow::seek(int value, SkyConnectIntf::SeekMode seekMode) const noexcept
{
    auto &skyConnectManager = SkyConnectManager::getInstance();
    const double factor = static_cast<double>(value) / static_cast<double>(::PositionSliderMax);
    const std::int64_t totalDuration = Logbook::getInstance().getCurrentFlight().getTotalDurationMSec();
    auto timestamp = static_cast<std::int64_t>(std::round(factor * static_cast<double>(totalDuration)));

    // Prevent the timestampEdit field to set the replay position as well
    ui->timestampEdit->blockSignals(true);
    skyConnectManager.seek(timestamp, seekMode);
    ui->timestampEdit->blockSignals(false);
}

// PRIVATE SLOTS

void MainWindow::onPositionSliderPressed() noexcept
{
    d->continuousSeek = true;
    auto &skyConnectManager = SkyConnectManager::getInstance();
    d->previousState = skyConnectManager.getState();
    if (skyConnectManager.isInReplayState()) {
        // Pause the replay while sliding the position slider
        d->moduleManager->setPaused(true);
    }
}

void MainWindow::onPositionSliderValueChanged(int value) noexcept
{
    const SkyConnectIntf::SeekMode seekMode = d->continuousSeek ? SkyConnectIntf::SeekMode::Continuous : SkyConnectIntf::SeekMode::Discrete;
    seek(value, seekMode);
}

void MainWindow::onPositionSliderReleased() noexcept
{
    if (d->previousState == Connect::State::Replay) {
        d->moduleManager->setPaused(false);
    } else if (d->previousState == Connect::State::ReplayPaused) {
        d->moduleManager->setPaused(true);
    }
    // Unpausing the replay resets event states, so we must seek *after* unpausing
    seek(ui->positionSlider->value(), SkyConnectIntf::SeekMode::Discrete);
    d->continuousSeek = false;
}

void MainWindow::onTimestampEditChanged(std::int64_t timestamp) noexcept
{
    auto &skyConnectManager = SkyConnectManager::getInstance();
    if (skyConnectManager.isIdle() || skyConnectManager.getState() == Connect::State::ReplayPaused) {
        skyConnectManager.seek(timestamp, SkyConnectIntf::SeekMode::Discrete);
    }
}

void MainWindow::updateWindowSize() noexcept
{
    const bool minimalUi = isMinimalUiEnabled();
    if (minimalUi) {
        setMinimumSize(0, 0);
        // Let the layout manager realise that a widget has been hidden, which is an
        // asynchronous process
        QCoreApplication::processEvents();
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
    ReplaySpeed replaySpeed = static_cast<ReplaySpeed>(action->property(::ReplaySpeedProperty).toInt());
    float replaySpeedFactor {1.0f};
    switch (replaySpeed) {
    case ReplaySpeed::Slow10:
        replaySpeedFactor = 0.1f;
        break;
    case ReplaySpeed::Slow25:
        replaySpeedFactor = 0.25f;
        break;
    case ReplaySpeed::Slow50:
        replaySpeedFactor = 0.5f;
        break;
    case ReplaySpeed::Slow75:
        replaySpeedFactor = 0.75f;
        break;
    case ReplaySpeed::Normal:
        replaySpeedFactor = 1.0f;
        break;
    case ReplaySpeed::Fast2x:
        replaySpeedFactor = 2.0f;
        break;
    case ReplaySpeed::Fast4x:
        replaySpeedFactor = 4.0f;
        break;
    case ReplaySpeed::Fast8x:
        replaySpeedFactor = 8.0f;
        break;
    case ReplaySpeed::Fast16x:
        replaySpeedFactor = 16.0f;
        break;
    case ReplaySpeed::Custom:
        replaySpeedFactor = d->lastCustomReplaySpeedFactor;
        break;
    }

    auto &skyConnectManager = SkyConnectManager::getInstance();
    skyConnectManager.setReplaySpeedFactor(replaySpeedFactor);

    updateReplaySpeedUi();

    // Give the simulation rate a bit of time to settle in the flight simulator
    QTimer::singleShot(1s, this, &MainWindow::requestSimulationRate);
}

void MainWindow::onCustomSpeedChanged() noexcept
{
    auto &skyConnectManager = SkyConnectManager::getInstance();
    const double customReplaySpeedFactor = getCustomSpeedFactor();
    skyConnectManager.setReplaySpeedFactor(customReplaySpeedFactor);
    switch (Settings::getInstance().getReplaySpeeedUnit()) {
    case Replay::SpeedUnit::Absolute:
        d->lastCustomReplaySpeedFactor = customReplaySpeedFactor;
        break;
    case Replay::SpeedUnit::Percent:
        d->lastCustomReplaySpeedFactor = customReplaySpeedFactor * 100.0f;
        break;
    }

    // Give the simulation rate a bit of time to settle in the flight simulator
    QTimer::singleShot(1s, this, &MainWindow::requestSimulationRate);
}

void MainWindow::onReplaySpeedUnitSelected(int index) noexcept
{
    auto &settings = Settings::getInstance();
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
    updateTimeUi();
    updateControlIcons();
    updateReplaySpeedUi();
    updateFileMenu();
    updateModuleActions();
    updateWindowMenu();
    updateMainWindow();
    if (!SkyConnectManager::getInstance().isInRecordingState()) {
        onRecordingDurationChanged();
    }
}

void MainWindow::updateControlUi() noexcept
{
    const auto &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
    const bool hasRecording = aircraft.hasRecording();

    const auto &skyConnectManager = SkyConnectManager::getInstance();
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
        ui->clearFlightAction->setEnabled(hasRecording);
        // Transport
        ui->skipToBeginAction->setEnabled(hasRecording && hasSkyConnectPlugins);
        ui->backwardAction->setEnabled(hasRecording && hasSkyConnectPlugins);
        ui->forwardAction->setEnabled(hasRecording && hasSkyConnectPlugins);
        ui->skipToEndAction->setEnabled(hasRecording && hasSkyConnectPlugins);
        // Position
        ui->positionSlider->setEnabled(hasRecording && hasSkyConnectPlugins);
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
        ui->clearFlightAction->setEnabled(false);
        // Transport
        ui->skipToBeginAction->setEnabled(false);
        ui->backwardAction->setEnabled(false);
        ui->forwardAction->setEnabled(false);
        ui->skipToEndAction->setEnabled(false);
        // Position
        ui->positionSlider->setEnabled(false);
        ui->positionSlider->setValue(::PositionSliderMax);
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
        // Clearing a playing flight will stop it first
        ui->clearFlightAction->setEnabled(true);
        // Transport
        ui->skipToBeginAction->setEnabled(true);
        ui->backwardAction->setEnabled(true);
        ui->forwardAction->setEnabled(true);
        ui->skipToEndAction->setEnabled(true);
        // Position
        ui->positionSlider->setEnabled(true);
        break;
    case Connect::State::ReplayPaused:
        // Actions
        ui->pauseAction->setChecked(true);
        ui->playAction->setChecked(false);
        break;
    }

    const bool loopReplayEnabled = Settings::getInstance().isReplayLoopEnabled();
    ui->loopReplayAction->setChecked(loopReplayEnabled);

    updatePlayActionTooltip();
}

void MainWindow::updateTimeUi() noexcept
{
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    const auto &aircraft = flight.getUserAircraft();

    // TODO Perhaps add option to switch between simulation and real-world ("creation") time
    const auto startZuluDateTime = flight.getAircraftStartZuluTime(aircraft);
    ui->timestampEdit->setStartDateTime(startZuluDateTime);

    const auto &skyConnectManager = SkyConnectManager::getInstance();
    const bool hasRecording = aircraft.hasRecording();
    const bool hasSkyConnectPlugins = skyConnectManager.hasPlugins();
    bool enabled {false};
    switch (skyConnectManager.getState()) {
    case Connect::State::Disconnected:
        // Fall-thru intended: each time a control element is triggered a connection
        // attempt is made, so we enable the same elements as in connected state
        [[fallthrough]];
    case Connect::State::Connected:
        enabled = hasRecording && hasSkyConnectPlugins;
        ui->timestampEdit->setEnabled(enabled);
        break;
    case Connect::State::Recording:
        ui->timestampEdit->setEnabled(false);
        break;
    case Connect::State::RecordingPaused:
        break;
    case Connect::State::Replay:
        ui->timestampEdit->setEnabled(false);
        break;
    case Connect::State::ReplayPaused:
        ui->timestampEdit->setEnabled(true);
        break;
    }
}

void MainWindow::updatePlayActionTooltip() noexcept
{
    ui->playAction->setToolTip(tr("Simulation rate: %1").arg(d->simulationRate));
}

void MainWindow::updateControlIcons() noexcept
{
    const QIcon &recordIcon = d->moduleManager->getRecordIcon();
    ui->recordAction->setIcon(recordIcon);
}

void MainWindow::updateSimulationRateLabel() noexcept
{
    d->simulationRateLabel->setText(tr("Simulation rate: %1").arg(d->simulationRate));
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

void MainWindow::onRecordingStopped() noexcept
{
    onRecordingDurationChanged();
}

void MainWindow::onActionActivated(FlightSimulatorShortcuts::Action action) noexcept
{
    const auto &skyConnectManager = SkyConnectManager::getInstance();
    QString pushNotification;
    switch (action) {
    case FlightSimulatorShortcuts::Action::Record:
        if (ui->recordAction->isEnabled()) {
            if (skyConnectManager.isInRecordingState()) {
                pushNotification = tr("Recording stopped.");
            } else {
                pushNotification = tr("Recording started.");
            }
            ui->recordAction->trigger();
        }
        break;
    case FlightSimulatorShortcuts::Action::Replay:
        if (ui->playAction->isEnabled()) {
            ui->playAction->trigger();
        }
        break;
    case FlightSimulatorShortcuts::Action::Pause:
        if (ui->pauseAction->isEnabled()) {
            if (skyConnectManager.isRecordingPaused()) {
                pushNotification = tr("Recording resumed.");
            } else if (skyConnectManager.isRecording()) {
                pushNotification = tr("Recording paused.");
            }
            ui->pauseAction->trigger();
        }
        break;
    case FlightSimulatorShortcuts::Action::Stop:
        if (ui->stopAction->isEnabled()) {
            if (skyConnectManager.isInRecordingState()) {
                pushNotification = tr("Recording stopped.");
            }
            ui->stopAction->trigger();
        }
        break;
    case FlightSimulatorShortcuts::Action::Backward:
        if (ui->backwardAction->isEnabled()) {
            ui->backwardAction->trigger();
        }
        break;
    case FlightSimulatorShortcuts::Action::Forward:
        if (ui->forwardAction->isEnabled()) {
            ui->forwardAction->trigger();
        }
        break;
    case FlightSimulatorShortcuts::Action::Begin:
        if (ui->skipToBeginAction->isEnabled()) {
            ui->skipToBeginAction->trigger();
        }
        break;
    case FlightSimulatorShortcuts::Action::End:
        if (ui->skipToEndAction->isEnabled()) {
            ui->skipToEndAction->trigger();
        }
        break;
    case FlightSimulatorShortcuts::Action::None:
        break;
    }

    if (!pushNotification.isEmpty()) {
        d->trayIcon->showMessage(Version::getApplicationName(), pushNotification,
                                 QSystemTrayIcon::Information, 3000);
    }

#ifdef DEBUG
    qDebug() << "Main window: flight simulator shortcut activated" << Enum::underly(action);
#endif
}

void MainWindow::requestSimulationRate() noexcept
{
    SkyConnectManager::getInstance().requestSimulationRate();
}

void MainWindow::onSimulationRateReceived(float rate) noexcept
{
    d->simulationRate = rate;
    updatePlayActionTooltip();
    updateSimulationRateLabel();
}

void MainWindow::onRecordingDurationChanged() noexcept
{
    updateRecordingDuration();
    updatePositionSlider(SkyConnectManager::getInstance().getCurrentTimestamp());
}

void MainWindow::updateRecordingDuration() noexcept
{
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    const std::int64_t totalDuration = flight.getTotalDurationMSec();
    ui->timestampEdit->blockSignals(true);
    ui->timestampEdit->setEndTimestamp(totalDuration);
    ui->timestampEdit->blockSignals(false);
}

void MainWindow::updateFileMenu() noexcept
{
    const auto &aircraft = Logbook::getInstance().getCurrentFlight().getUserAircraft();
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
    const auto &settings = Settings::getInstance();
    const auto flags = windowFlags();
    if (settings.isWindowStaysOnTopEnabled() != flags.testFlag(Qt::WindowStaysOnTopHint)) {
        if (Settings::getInstance().isWindowStaysOnTopEnabled()) {
            this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
            this->show();
        } else {
            this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
            this->show();
        }
    }
    if (ui->recordAction->isChecked()) {
        ui->recordAction->setToolTip(tr("Stop the recording."));
    } else {
        ui->recordAction->setToolTip(tr("Start a new recording."));
    }

    if (settings.isAbsoluteSeekEnabled()) {
        const auto seekIntervalSeconds = settings.getSeekIntervalSeconds();
        ui->forwardAction->setToolTip(tr("Fast forward [%1 sec].").arg(seekIntervalSeconds));
        ui->backwardAction->setToolTip(tr("Rewind [%1 sec].").arg(seekIntervalSeconds));
    } else {
        const auto seekIntervalPercent = settings.getSeekIntervalPercent();
        ui->forwardAction->setToolTip(tr("Fast forward [%1 %].").arg(seekIntervalPercent));
        ui->backwardAction->setToolTip(tr("Rewind [%1 %].").arg(seekIntervalPercent));
    }

    if (settings.isModuleSelectorVisible()) {
        ui->showModulesAction->setToolTip(tr("Hide modules."));
    } else {
        ui->showModulesAction->setToolTip(tr("Show modules."));
    }
    if (d->connectedWithLogbook && !settings.isMinimalUiEnabled()) {
        setWindowTitle(Version::getApplicationName() % " - " % QFileInfo(settings.getLogbookPath()).fileName());
    } else {
        setWindowTitle(Version::getApplicationName());
    }

    d->trayIcon->show();
}

void MainWindow::onModuleActivated(const QString &title, [[maybe_unused]] QUuid uuid) noexcept
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
        if (ok) {
            RecentFile::getInstance().addRecentFile(logbookPath);
        } else {
            QMessageBox::critical(this, tr("Logbook Error"), tr("The logbook %1 could not be created.").arg(QDir::toNativeSeparators(logbookPath)));
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

void MainWindow::onRecentFileSelected(const QString &filePath, SecurityToken *securityToken) noexcept
{
    // Connecting with a non-existing SQLite database will actually succeed when calling
    // connectWithLogbook() (the non-existing database file is created first), so we
    // explicitly check the existence of the file
    const QFileInfo fileInfo {filePath};
    bool ok = fileInfo.exists();
    if (ok) {
        ok = connectWithLogbook(filePath);
    } else {
        QMessageBox::critical(this, tr("Logbook Not Found"), tr("The logbook %1 does not exist.").arg(QDir::toNativeSeparators(filePath)));
    }
    if (!ok) {
        RecentFile::getInstance().removeRecentFile(filePath);
    }
}

void MainWindow::updateRecentFileMenu() noexcept
{
    ui->recentFilesMenu->clear();
    for (QAction *recentFileAction : d->recentFileMenu->getRecentFileActionGroup().actions()) {
        ui->recentFilesMenu->addAction(recentFileAction);
    }
}

void MainWindow::optimiseLogbook() noexcept
{
    const auto &persistenceManager = PersistenceManager::getInstance();
    const QString logbookPath = persistenceManager.getLogbookPath();
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
            QMessageBox::critical(this, tr("Logbook Error"), tr("The logbook could not be optimised."));
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
    auto &settings = Settings::getInstance();
    settings.setModuleSelectorVisible(enable);
    ui->moduleSelectorWidget->setVisible(enable);
}

void MainWindow::onShowReplaySpeedChanged(bool enable) noexcept
{
    auto &settings = Settings::getInstance();
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

void MainWindow::clearFlight() noexcept
{
    stop();
    Logbook::getInstance().getCurrentFlight().clear(true, FlightData::CreationTimeMode::Reset);
}

// Service

void MainWindow::onFlightRestored() noexcept
{
    updateUi();
    d->moduleManager->setupInitialPosition();
}

void MainWindow::onLogbookConnectionChanged(bool connected) noexcept
{
    d->connectedWithLogbook = connected;
    updateUi();
}

void MainWindow::onFlightImport(QAction *action) noexcept
{
    auto &flight = Logbook::getInstance().getCurrentFlight();
    const QUuid pluginUuid = action->data().toUuid();
    const bool ok = PluginManager::getInstance().importFlights(pluginUuid, flight);
    if (ok) {
        updateUi();
        auto &skyConnectManager = SkyConnectManager::getInstance();
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
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    PluginManager::getInstance().exportFlight(flight, pluginUuid);
}

void MainWindow::onLocationImport(QAction *action) noexcept
{
    const QUuid pluginUuid = action->data().toUuid();
    PluginManager::getInstance().importLocations(pluginUuid);
}

void MainWindow::onLocationExport(QAction *action) noexcept
{
    const QUuid pluginUuid = action->data().toUuid();
    PluginManager::getInstance().exportLocations(pluginUuid);
}

void MainWindow::onReplayLoopChanged() noexcept
{
    updateControlUi();
}

void MainWindow::onStyleKeyChanged() noexcept
{
#ifdef DEBUG
    qDebug() << "MainWindow::onStyleKeyChanged: Application style changed (restart).";
#endif
    std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
    messageBox->setIcon(QMessageBox::Information);
    QPushButton *laterButton = messageBox->addButton(tr("&Later"), QMessageBox::RejectRole);
    QPushButton *restartButton = messageBox->addButton(tr("&Restart"), QMessageBox::AcceptRole);
    messageBox->setWindowTitle(tr("Restart Required"));
    messageBox->setText(tr("The changed style becomes visible after a restart."));
    messageBox->setDefaultButton(laterButton);

    messageBox->exec();
    const QAbstractButton *clickedButton = messageBox->clickedButton();
    if (clickedButton == restartButton) {
        QCoreApplication::quit();
        QProcess::startDetached(QCoreApplication::applicationFilePath(), {});
    }
}
