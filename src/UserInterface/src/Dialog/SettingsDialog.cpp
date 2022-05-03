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
    constexpr double MinSeekSeconds = 0.001;
    constexpr double MaxSeekSeconds = 999.0;

    constexpr double MinSeekPercent = 0.001;
    constexpr double MaxSeekPercent = 100.0;
}

SettingsDialog::SettingsDialog(QWidget *parent) noexcept :
    QDialog(parent),
    ui(std::make_unique<Ui::SettingsDialog>())
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

SettingsDialog::~SettingsDialog() noexcept
{}

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

    // SkyConnect plugin (flight simulator)
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    std::vector<SkyConnectManager::Handle> plugins = skyConnectManager.availablePlugins();
    for (auto &plugin : plugins) {
        ui->connectionComboBox->addItem(plugin.second.name, plugin.first);
    }

    // Record
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Auto), tr("Auto"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz1), tr("1 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz2), tr("2 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz5), tr("5 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz10), tr("10 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz15), tr("15 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz20), tr("20 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz24), tr("24 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz25), tr("25 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz30), tr("30 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz45), tr("45 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz50), tr("50 Hz"));
    ui->recordFrequencyComboBox->insertItem(Enum::toUnderlyingType(SampleRate::SampleRate::Hz60), tr("60 Hz"));

    // Replay
    ui->seekInSecondsSpinBox->setMinimum(MinSeekSeconds);
    ui->seekInSecondsSpinBox->setMaximum(MaxSeekSeconds);

    ui->seekInPercentSpinBox->setMinimum(MinSeekPercent);
    ui->seekInPercentSpinBox->setMaximum(MaxSeekPercent);

    ui->repeatFlapsCheckBox->setToolTip(SimVar::FlapsHandleIndex);
    ui->repeatCanopyOpenCheckBox->setToolTip(SimVar::CanopyOpen);

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

    // SkyConnect plugin
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    std::optional<QString> pluginName = skyConnectManager.getCurrentSkyConnectPluginName();
    if (pluginName) {
        ui->connectionComboBox->setCurrentText(pluginName.value());
    }

    // Recording
    ui->recordFrequencyComboBox->setCurrentIndex(Enum::toUnderlyingType(settings.getRecordingSampleRate()));

    // Replay
    ui->absoluteSeekEnabledCheckBox->setChecked(settings.isAbsoluteSeekEnabled());
    ui->seekInSecondsSpinBox->setValue(settings.getSeekIntervalSeconds());
    ui->seekInPercentSpinBox->setValue(settings.getSeekIntervalPercent());

    ui->repeatFlapsCheckBox->setChecked(settings.isRepeatFlapsHandleIndexEnabled());
    ui->repeatCanopyOpenCheckBox->setChecked(settings.isRepeatCanopyOpenEnabled());

    // User interface
    ui->confirmDeleteFlightCheckBox->setChecked(settings.isDeleteFlightConfirmationEnabled());
    ui->confirmDeleteAircraftCheckBox->setChecked(settings.isDeleteAircraftConfirmationEnabled());
    ui->confirmResetAllTimeOffsetCheckBox->setChecked(settings.isResetTimeOffsetConfirmationEnabled());
}

void SettingsDialog::handleAccepted() noexcept
{
    Settings &settings = Settings::getInstance();

    // Flight simulator
    const QUuid uuid = ui->connectionComboBox->currentData().toUuid();
    settings.setSkyConnectPluginUuid(uuid);

    // Recording
    settings.setRecordingSampleRate(static_cast<SampleRate::SampleRate>(ui->recordFrequencyComboBox->currentIndex()));

    // Replay
    settings.setAbsoluteSeekEnabled(ui->absoluteSeekEnabledCheckBox->isChecked());
    settings.setSeekIntervalSeconds(ui->seekInSecondsSpinBox->value());
    settings.setSeekIntervalPercent(ui->seekInPercentSpinBox->value());

    settings.setRepeatFlapsHandleIndexEnabled(ui->repeatFlapsCheckBox->isChecked());
    settings.setRepeatCanopyOpenEnabled(ui->repeatCanopyOpenCheckBox->isChecked());

    // User interface
    settings.setDeleteFlightConfirmationEnabled(ui->confirmDeleteFlightCheckBox->isChecked());
    settings.setDeleteAircraftConfirmationEnabled(ui->confirmDeleteAircraftCheckBox->isChecked());
    settings.setResetTimeOffsetConfirmationEnabled(ui->confirmResetAllTimeOffsetCheckBox->isChecked());
}
