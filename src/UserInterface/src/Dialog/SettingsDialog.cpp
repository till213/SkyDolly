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
#include <memory>
#include <optional>
#include <cmath>
#include <unordered_map>

#include <QDialog>
#include <QWidget>
#include <QString>
#include <QDoubleSpinBox>
#include <QTimer>
#include <QStyleFactory>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/SampleRate.h>
#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Kernel/System.h>
#include <Model/SimVar.h>
#include <PluginManager/SkyConnectManager.h>
#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

namespace
{
    constexpr double MinSeekSeconds {0.001};
    constexpr double MaxSeekSeconds {999.0};

    constexpr double MinSeekPercent {0.001};
    constexpr double MaxSeekPercent {100.0};

    constexpr int UpdateIntervalMSec = 1000;

    constexpr int ReplayTab = 0;
    constexpr int FlightSimulatorTab = 1;

    constexpr const char *WindowsStyleKey {"windows"};
    constexpr const char *FusionStyleKey {"fusion"};
    constexpr const char *WindowsVistaStyleKey {"windowsvista"};
    constexpr const char *Windows11StyleKey {"windows11"};
    constexpr const char *MacOSStyleKey {"macos"};
}

struct SettingsDialogPrivate
{
    SettingsDialogPrivate()
    {
        updateTimer.setTimerType(Qt::TimerType::PreciseTimer);
        if (knownStyleNames.empty()) {
            knownStyleNames = {
                { Settings::DefaultStyleKey, QApplication::translate("SettingsDialogPrivate", "Default") },
                { ::WindowsStyleKey, "Windows" },
                { ::FusionStyleKey, "Fusion" },
                { ::WindowsVistaStyleKey, "Windows Vista" },
                { ::Windows11StyleKey, "Windows 11" },
                { ::MacOSStyleKey, "macOS" }
            };
        }
    }
    QTimer updateTimer;
    std::unique_ptr<OptionWidgetIntf> skyConnectOptionWidget {nullptr};

    // Key: style key (all lower case), value: style name
    static inline std::unordered_map<QString, QString> knownStyleNames;
};

// PUBLIC

SettingsDialog::SettingsDialog(QWidget *parent) noexcept :
    QDialog {parent},
    ui {std::make_unique<Ui::SettingsDialog>()},
    d {std::make_unique<SettingsDialogPrivate>()}
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

SettingsDialog::~SettingsDialog() = default;

// PROTECTED

void SettingsDialog::showEvent(QShowEvent *event) noexcept
{
    QDialog::showEvent(event);
    updateUi();
    connect(&Settings::getInstance(), &Settings::changed,
            this, &SettingsDialog::updateUi);
}

void SettingsDialog::hideEvent(QHideEvent *event) noexcept
{
    QDialog::hideEvent(event);
    disconnect(&Settings::getInstance(), &Settings::changed,
               this, &SettingsDialog::updateUi);
}

// PRIVATE

void SettingsDialog::initUi() noexcept
{
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    // Replay
    ui->seekInSecondsSpinBox->setMinimum(::MinSeekSeconds);
    ui->seekInSecondsSpinBox->setMaximum(::MaxSeekSeconds);
    ui->seekInPercentSpinBox->setMinimum(::MinSeekPercent);
    ui->seekInPercentSpinBox->setMaximum(::MaxSeekPercent);

    ui->repeatCanopyOpenCheckBox->setToolTip(tr("When enabled this option will repeatedly send the same value for simulation variable \"%1\", even when its value does not actually change.\n"
                                                "This may help enforcing a consistent canopy animation with certain aircraft.")
                                                .arg(SimVar::CanopyOpen));
    ui->maximumSimulationRateSpinBox->setToolTip(tr("This option limits the simulation rate in the flight simulator. Note that the actual replay speed may still be set to higher values."));

    // TODO For now we only support "none" and "simulation time"
    ui->timeModeComboBox->addItem(tr("None"), Enum::underly(Replay::TimeMode::None));
    ui->timeModeComboBox->addItem(tr("Simulation time"), Enum::underly(Replay::TimeMode::SimulationTime));

    // Flight simulator
    auto &skyConnectManager = SkyConnectManager::getInstance();
    std::vector<SkyConnectManager::Handle> plugins = skyConnectManager.availablePlugins();
    for (const auto &plugin : plugins) {
        ui->connectionComboBox->addItem(plugin.second.name, plugin.first);
    }
    initFlightSimulatorOptionWidget();

    // User interface
    const auto defaultStyleName = d->knownStyleNames[Settings::DefaultStyleKey];
    ui->styleComboBox->addItem(defaultStyleName, Settings::DefaultStyleKey);
    auto styleKeys = QStyleFactory::keys();
    styleKeys.sort();
    for (const auto &key : styleKeys) {
        const auto lowerKey = key.toLower();
        const auto &styleName = d->knownStyleNames.contains(lowerKey) ? d->knownStyleNames[lowerKey] : key;
        if (!(System::isWindows10() && lowerKey == ::Windows11StyleKey) && !(System::isWindows11() && lowerKey == ::WindowsVistaStyleKey)) {
            ui->styleComboBox->addItem(styleName, lowerKey);
        }
    }

    ui->settingsTabWidget->setCurrentIndex(::ReplayTab);
    onTabChanged(ui->settingsTabWidget->currentIndex());
}

void SettingsDialog::frenchConnection() noexcept
{
    const auto &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::connectionChanged,
            this, &SettingsDialog::onSkyConnectPluginChanged);
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &SettingsDialog::updateUi);

    connect(this, &SettingsDialog::accepted,
            this, &SettingsDialog::onAccepted);
    connect(ui->settingsTabWidget, &QTabWidget::currentChanged,
            this, &SettingsDialog::onTabChanged);
    connect(ui->connectionComboBox, &QComboBox::currentIndexChanged,
            this, &SettingsDialog::onFlightSimulatorConnectionSelectionChanged);
    connect(ui->styleComboBox, &QComboBox::currentIndexChanged,
            this, &SettingsDialog::onStyleChanged);
}

void SettingsDialog::updateReplayTab() noexcept
{
    const auto &settings = Settings::getInstance();

    // Replay
    ui->absoluteSeekEnabledCheckBox->setChecked(settings.isAbsoluteSeekEnabled());
    ui->seekInSecondsSpinBox->setValue(settings.getSeekIntervalSeconds());
    ui->seekInPercentSpinBox->setValue(settings.getSeekIntervalPercent());
    ui->repeatCanopyOpenCheckBox->setChecked(settings.isRepeatCanopyOpenEnabled());
    ui->maximumSimulationRateSpinBox->setValue(settings.getMaximumSimulationRate());

    const auto replayTimeMode = settings.getReplayTimeMode();
    int currentIndex {0};
    auto indexCount = ui->timeModeComboBox->count();
    while (currentIndex < indexCount &&
           static_cast<Replay::TimeMode>(ui->timeModeComboBox->itemData(currentIndex).toInt()) != replayTimeMode) {
        ++currentIndex;
    }
    if (currentIndex < indexCount) {
        ui->timeModeComboBox->setCurrentIndex(currentIndex);
    } else if (indexCount > 0) {
        // Option not supported -> select the first available option
        ui->timeModeComboBox->setCurrentIndex(0);
    }
    ui->timeModeComboBox->setToolTip(tr("Defines how the time in the flight simulator is synchronised during replay."));
}

void SettingsDialog::updateFlightSimulatorTab() noexcept
{
    const auto &settings = Settings::getInstance();

    auto &skyConnectManager = SkyConnectManager::getInstance();
    std::optional<QString> pluginName = skyConnectManager.getCurrentSkyConnectPluginName();
    if (pluginName) {
        ui->connectionComboBox->setCurrentText(pluginName.value());
    }
    const bool enabled = !skyConnectManager.isActive();
    ui->connectionComboBox->setEnabled(enabled);
    updateConnectionStatus();
}

void SettingsDialog::updateUserInterfaceTab() noexcept
{
    const auto &settings = Settings::getInstance();

    const auto styleKey = settings.getStyleKey();
    bool found {false};
    int index = 0;
    while (!found && index < ui->styleComboBox->count()) {
        const auto key = ui->styleComboBox->itemData(index);
        if (key == styleKey) {
            found = true;
        } else {
            ++index;
        }
    }
    if (found) {
        ui->styleComboBox->setCurrentIndex(index);
    }
    ui->confirmDeleteFlightCheckBox->setChecked(settings.isDeleteFlightConfirmationEnabled());
    ui->confirmDeleteAircraftCheckBox->setChecked(settings.isDeleteAircraftConfirmationEnabled());
    ui->confirmDeleteLocationCheckBox->setChecked(settings.isDeleteLocationConfirmationEnabled());
    ui->confirmResetAllTimeOffsetCheckBox->setChecked(settings.isResetTimeOffsetConfirmationEnabled());

    // Note: from a user's perspective the "hiding aspect" is more important ("which UI elements to I want
    //       to hide in minimal UI mode" - hence the "hide" in the checkbox names), but from a logic perspective
    //       it makes more sense to talk about "visibility" (true = visible, false = hidden)
    ui->hideButtonTextCheckBox->setChecked(!settings.getDefaultMinimalUiButtonTextVisibility());
    ui->hideNonEssentialButtonsCheckBox->setChecked(!settings.getDefaultMinimalUiNonEssentialButtonVisibility());
    ui->hideReplaySpeedCheckBox->setChecked(!settings.getDefaultMinimalUiReplaySpeedVisibility());
}

// PRIVATE SLOTS

void SettingsDialog::updateUi() noexcept
{
    updateReplayTab();
    updateFlightSimulatorTab();
    updateUserInterfaceTab();
}

void SettingsDialog::updateConnectionStatus() const noexcept
{
    double time {0.0};

    ui->connectionStatusLabel->setToolTip("");

    auto &skyConnectManager = SkyConnectManager::getInstance();
    switch (skyConnectManager.getState()) {
    case Connect::State::Disconnected:
        ui->connectionStatusLabel->setText(tr("Disconnected"));
        time = skyConnectManager.getRemainingReconnectTime() / 1000.0;
        if (time != -1) {
            ui->connectionStatusLabel->setToolTip(tr("Next reconnect attempt in %1 seconds").arg(std::round(time)));
        }
        break;
    case Connect::State::Connected:
        ui->connectionStatusLabel->setText(tr("Connected"));
        break;
    case Connect::State::Recording:
        ui->connectionStatusLabel->setText(tr("Recording"));
        break;
    case Connect::State::RecordingPaused:
        ui->connectionStatusLabel->setText(tr("Recording paused"));
        break;
    case Connect::State::Replay:
        ui->connectionStatusLabel->setText(tr("Replaying"));
        break;
    case Connect::State::ReplayPaused:
        ui->connectionStatusLabel->setText(tr("Replay paused"));
        break;
    default:
        ui->connectionStatusLabel->setText(tr("Disconnected"));
        break;
    }
}

void SettingsDialog::onFlightSimulatorConnectionSelectionChanged() const noexcept
{
    auto &skyConnectManager = SkyConnectManager::getInstance();
    const auto uuid = ui->connectionComboBox->currentData().toUuid();
    skyConnectManager.tryAndSetCurrentSkyConnect(uuid);
}

void SettingsDialog::onStyleChanged() noexcept
{
    if (ui->styleComboBox->currentData().toString() != Settings::getInstance().getStyleKey()) {
        ui->styleInfoLabel->setText(tr("Restart required."));
    } else {
        ui->styleInfoLabel->clear();
    }
}

void SettingsDialog::onSkyConnectPluginChanged() noexcept
{
    if (d->skyConnectOptionWidget != nullptr) {
        d->skyConnectOptionWidget.reset();
    }
    initFlightSimulatorOptionWidget();
}

void SettingsDialog::onAccepted() noexcept
{
    auto &settings = Settings::getInstance();

    // Replay
    settings.setAbsoluteSeekEnabled(ui->absoluteSeekEnabledCheckBox->isChecked());
    settings.setSeekIntervalSeconds(ui->seekInSecondsSpinBox->value());
    settings.setSeekIntervalPercent(ui->seekInPercentSpinBox->value());
    settings.setRepeatCanopyOpenEnabled(ui->repeatCanopyOpenCheckBox->isChecked());
    settings.setMaximumSimulationRate(ui->maximumSimulationRateSpinBox->value());
    settings.setReplayTimeMode(static_cast<Replay::TimeMode>(ui->timeModeComboBox->currentData().toInt()));

    // Flight simulator
    if (d->skyConnectOptionWidget != nullptr) {
        d->skyConnectOptionWidget->accept();
    }

    // User interface
    settings.setStyleKey(ui->styleComboBox->currentData().toString());
    settings.setDeleteFlightConfirmationEnabled(ui->confirmDeleteFlightCheckBox->isChecked());
    settings.setDeleteAircraftConfirmationEnabled(ui->confirmDeleteAircraftCheckBox->isChecked());
    settings.setDeleteLocationConfirmationEnabled(ui->confirmDeleteLocationCheckBox->isChecked());
    settings.setResetTimeOffsetConfirmationEnabled(ui->confirmResetAllTimeOffsetCheckBox->isChecked());

    // See note above about the boolean inversion
    settings.setDefaultMinimalUiButtonTextVisibility(!ui->hideButtonTextCheckBox->isChecked());
    settings.setDefaultMinimalUiNonEssentialButtonVisibility(!ui->hideNonEssentialButtonsCheckBox->isChecked());
    settings.setDefaultMinimalUiReplaySpeedVisibility(!ui->hideReplaySpeedCheckBox->isChecked());
}

void SettingsDialog::onTabChanged(int index) noexcept
{
    switch (index)
    {
    case ::FlightSimulatorTab:
        connect(&d->updateTimer, &QTimer::timeout,
                this, &SettingsDialog::updateConnectionStatus);
        d->updateTimer.start(::UpdateIntervalMSec);
#ifdef DEBUG
        qDebug() << "SettingsDialog::onTabChanged: index:" << index << "started update timer";
#endif
        break;
    default:
        d->updateTimer.stop();
        disconnect(&d->updateTimer, &QTimer::timeout,
                   this, &SettingsDialog::updateConnectionStatus);
#ifdef DEBUG
        qDebug() << "SettingsDialog::onTabChanged: index:" << index << "stopped update timer";
#endif
        break;
    }
}

void SettingsDialog::initFlightSimulatorOptionWidget() noexcept
{
    const auto &skyConnectManager = SkyConnectManager::getInstance();
    auto optionWidget = skyConnectManager.createOptionWidget();
    if (optionWidget) {
        // Transfer ownership to this settings dialog (the layout manager below specifically)
        d->skyConnectOptionWidget = std::move(optionWidget.value());
        ui->optionGroupBox->setHidden(false);
        std::unique_ptr<QLayout> layout {ui->optionGroupBox->layout()};
        // Any previously existing layout is deleted first, which is what we want
        layout = std::make_unique<QVBoxLayout>();
        layout->addWidget(d->skyConnectOptionWidget.get());
        // Transfer ownership of the layout to the optionGroupBox
        ui->optionGroupBox->setLayout(layout.release());
    } else {
        d->skyConnectOptionWidget.reset();
        ui->optionGroupBox->setHidden(true);
    }
}
