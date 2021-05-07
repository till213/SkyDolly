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

#include <QDoubleValidator>
#include <QDialog>
#include <QWidget>

#include "../../../Kernel/src/SampleRate.h"
#include "../../../Kernel/src/Enum.h"
#include "../../../Kernel/src/Settings.h"
#include "../../../Model/src/SimVar.h"
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
    Q_UNUSED(event)

    updateUi();
    connect(&Settings::getInstance(), &Settings::changed,
            this, &SettingsDialog::updateUi);
}

void SettingsDialog::hideEvent(QHideEvent *event) noexcept
{
    Q_UNUSED(event)
    disconnect(&Settings::getInstance(), &Settings::changed,
               this, &SettingsDialog::updateUi);
}

// PRIVATE

void SettingsDialog::initUi() noexcept
{
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

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
    QDoubleValidator *secondsValidator = new QDoubleValidator(ui->seekInSecondsLineEdit);
    ui->seekInSecondsLineEdit->setValidator(secondsValidator);
    secondsValidator->setBottom(MinSeekSeconds);
    secondsValidator->setTop(MaxSeekSeconds);

    QDoubleValidator *percentValidator = new QDoubleValidator(ui->seekInPercentLineEdit);
    ui->seekInPercentLineEdit->setValidator(percentValidator);
    percentValidator->setBottom(MinSeekPercent);
    percentValidator->setTop(MaxSeekPercent);

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
    // Recording
    ui->recordFrequencyComboBox->setCurrentIndex(Enum::toUnderlyingType(settings.getRecordSampleRate()));

    // Replay
    ui->absoluteSeekEnabledCheckBox->setChecked(settings.isAbsoluteSeekEnabled());
    ui->seekInSecondsLineEdit->setText(QString::number(settings.getSeekIntervalSeconds()));
    ui->seekInPercentLineEdit->setText(QString::number(settings.getSeekIntervalPercent()));

    ui->repeatFlapsCheckBox->setChecked(settings.isRepeatFlapsHandleIndexEnabled());
    ui->repeatCanopyOpenCheckBox->setChecked(settings.isRepeatCanopyOpenEnabled());
}

void SettingsDialog::handleAccepted() noexcept
{
    Settings &settings = Settings::getInstance();

    // Recording
    settings.setRecordSampleRate(static_cast<SampleRate::SampleRate>(ui->recordFrequencyComboBox->currentIndex()));

    // Replay
    settings.setAbsoluteSeekEnabled(ui->absoluteSeekEnabledCheckBox->isChecked());
    settings.setSeekIntervalSeconds(ui->seekInSecondsLineEdit->text().toDouble());
    settings.setSeekIntervalPercent(ui->seekInPercentLineEdit->text().toDouble());

    settings.setRepeatFlapsHandleIndexEnabled(ui->repeatFlapsCheckBox->isChecked());
    settings.setRepeatCanopyOpenEnabled(ui->repeatCanopyOpenCheckBox->isChecked());
}
