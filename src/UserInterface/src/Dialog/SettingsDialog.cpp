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
#include <memory>
#include <optional>

#include <QDialog>
#include <QWidget>
#include <QString>
#include <QDoubleSpinBox>

#include <Kernel/SampleRate.h>
#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
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
}

SettingsDialog::SettingsDialog(QWidget *parent) noexcept :
    QDialog(parent),
    ui(std::make_unique<Ui::SettingsDialog>())
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
    ui->seekInSecondsSpinBox->setMinimum(MinSeekSeconds);
    ui->seekInSecondsSpinBox->setMaximum(MaxSeekSeconds);

    ui->seekInPercentSpinBox->setMinimum(MinSeekPercent);
    ui->seekInPercentSpinBox->setMaximum(MaxSeekPercent);

    ui->repeatCanopyOpenCheckBox->setToolTip(SimVar::CanopyOpen);

    // Recording
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Auto), tr("Auto"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz1), tr("1 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz2), tr("2 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz5), tr("5 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz10), tr("10 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz15), tr("15 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz20), tr("20 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz24), tr("24 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz25), tr("25 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz30), tr("30 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz45), tr("45 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz50), tr("50 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::underly(SampleRate::SampleRate::Hz60), tr("60 Hz"));

    // Flight simulator
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    std::vector<SkyConnectManager::Handle> plugins = skyConnectManager.availablePlugins();
    for (auto &plugin : plugins) {
        ui->connectionComboBox->addItem(plugin.second.name, plugin.first);
    }

    ui->recordSequenceEdit->setMaximumSequenceLength(1);
    ui->replaySequenceEdit->setMaximumSequenceLength(1);
    ui->pauseSequenceEdit->setMaximumSequenceLength(1);
    ui->stopSequenceEdit->setMaximumSequenceLength(1);
    ui->backwardSequenceEdit->setMaximumSequenceLength(1);
    ui->forwardSequenceEdit->setMaximumSequenceLength(1);
    ui->beginSequenceEdit->setMaximumSequenceLength(1);
    ui->endSequenceEdit->setMaximumSequenceLength(1);

    ui->settingsTabWidget->setCurrentIndex(0);
}

void SettingsDialog::frenchConnection() noexcept
{
    connect(this, &SettingsDialog::accepted,
            this, &SettingsDialog::handleAccepted);
}

// PRIVATE SLOTS

void SettingsDialog::updateUi() noexcept
{
    Settings &settings = Settings::getInstance();

    // Replay
    ui->absoluteSeekEnabledCheckBox->setChecked(settings.isAbsoluteSeekEnabled());
    ui->seekInSecondsSpinBox->setValue(settings.getSeekIntervalSeconds());
    ui->seekInPercentSpinBox->setValue(settings.getSeekIntervalPercent());
    ui->repeatCanopyOpenCheckBox->setChecked(settings.isRepeatCanopyOpenEnabled());

    // Recording
    ui->recordFrequencyComboBox->setCurrentIndex(Enum::underly(settings.getRecordingSampleRate()));

    // Flight simulator
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    std::optional<QString> pluginName = skyConnectManager.getCurrentSkyConnectPluginName();
    if (pluginName) {
        ui->connectionComboBox->setCurrentText(pluginName.value());
    }
    const FlightSimulatorShortcuts &shortcuts = settings.getFlightSimulatorShortcuts();
    ui->recordSequenceEdit->setKeySequence(shortcuts.record);
    ui->replaySequenceEdit->setKeySequence(shortcuts.replay);
    ui->pauseSequenceEdit->setKeySequence(shortcuts.pause);
    ui->stopSequenceEdit->setKeySequence(shortcuts.stop);
    ui->backwardSequenceEdit->setKeySequence(shortcuts.backward);
    ui->forwardSequenceEdit->setKeySequence(shortcuts.forward);
    ui->beginSequenceEdit->setKeySequence(shortcuts.begin);
    ui->endSequenceEdit->setKeySequence(shortcuts.end);

    // User interface
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

void SettingsDialog::handleAccepted() noexcept
{
    Settings &settings = Settings::getInstance();

    // Replay
    settings.setAbsoluteSeekEnabled(ui->absoluteSeekEnabledCheckBox->isChecked());
    settings.setSeekIntervalSeconds(ui->seekInSecondsSpinBox->value());
    settings.setSeekIntervalPercent(ui->seekInPercentSpinBox->value());
    settings.setRepeatCanopyOpenEnabled(ui->repeatCanopyOpenCheckBox->isChecked());

    // Recording
    settings.setRecordingSampleRate(static_cast<SampleRate::SampleRate>(ui->recordFrequencyComboBox->currentIndex()));

    // Flight simulator
    const QUuid uuid = ui->connectionComboBox->currentData().toUuid();
    settings.setSkyConnectPluginUuid(uuid);

    FlightSimulatorShortcuts shortcuts;
    shortcuts.record = ui->recordSequenceEdit->keySequence();
    shortcuts.replay = ui->replaySequenceEdit->keySequence();
    shortcuts.pause = ui->pauseSequenceEdit->keySequence();
    shortcuts.stop = ui->stopSequenceEdit->keySequence();
    shortcuts.backward = ui->backwardSequenceEdit->keySequence();
    shortcuts.forward = ui->forwardSequenceEdit->keySequence();
    shortcuts.begin = ui->beginSequenceEdit->keySequence();
    shortcuts.end = ui->endSequenceEdit->keySequence();
    settings.setFlightSimulatorShortcuts(shortcuts);

    // User interface
    settings.setDeleteFlightConfirmationEnabled(ui->confirmDeleteFlightCheckBox->isChecked());
    settings.setDeleteAircraftConfirmationEnabled(ui->confirmDeleteAircraftCheckBox->isChecked());
    settings.setDeleteLocationConfirmationEnabled(ui->confirmDeleteLocationCheckBox->isChecked());
    settings.setResetTimeOffsetConfirmationEnabled(ui->confirmResetAllTimeOffsetCheckBox->isChecked());

    // See note above about the boolean inversion
    settings.setDefaultMinimalUiButtonTextVisibility(!ui->hideButtonTextCheckBox->isChecked());
    settings.setDefaultMinimalUiNonEssentialButtonVisibility(!ui->hideNonEssentialButtonsCheckBox->isChecked());
    settings.setDefaultMinimalUiReplaySpeedVisibility(!ui->hideReplaySpeedCheckBox->isChecked());
}
