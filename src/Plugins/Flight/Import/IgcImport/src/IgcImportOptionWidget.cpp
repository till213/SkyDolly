/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

#include <Kernel/Enum.h>
#include <Kernel/Version.h>
#include <Kernel/File.h>
#include "IgcImportOptionWidget.h"
#include "IgcImportSettings.h"
#include "ui_IgcImportOptionWidget.h"

struct IgcImportOptionWidgetPrivate
{
    IgcImportOptionWidgetPrivate(IgcImportSettings &pluginSettings) noexcept
        : pluginSettings {pluginSettings}
    {}

     IgcImportSettings &pluginSettings;
};

// PUBLIC

IgcImportOptionWidget::IgcImportOptionWidget(IgcImportSettings &pluginSettings, QWidget *parent) noexcept
    : QWidget {parent},
      ui {std::make_unique<Ui::IgcImportOptionWidget>()},
      d {std::make_unique<IgcImportOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

IgcImportOptionWidget::~IgcImportOptionWidget() = default;

// PRIVATE

void IgcImportOptionWidget::frenchConnection() noexcept
{
    connect(ui->altitudeComboBox, &QComboBox::currentIndexChanged,
            this, &IgcImportOptionWidget::onAltitudeChanged);
    connect(ui->enlThresholdSpinBox, &QSpinBox::valueChanged,
            this, &IgcImportOptionWidget::onEnlThresholdChanged);
    connect(ui->convertAltitudeCheckBox, &QCheckBox::clicked,
            this, &IgcImportOptionWidget::onConvertAltitudeChanged);
    connect(&d->pluginSettings, &IgcImportSettings::changed,
            this, &IgcImportOptionWidget::updateUi);
}

void IgcImportOptionWidget::initUi() noexcept
{
    ui->altitudeComboBox->addItem(tr("GNSS altitude"), Enum::underly(IgcImportSettings::AltitudeMode::Gnss));
    ui->altitudeComboBox->addItem(tr("Pressure altitude"), Enum::underly(IgcImportSettings::AltitudeMode::Pressure));

    // Percent [0, 100]
    ui->enlThresholdSpinBox->setRange(0, 100);
    ui->enlThresholdSpinBox->setSuffix("%");
    ui->enlThresholdSpinBox->setSingleStep(5);
}

// PRIVATE SLOTS

void IgcImportOptionWidget::updateUi() noexcept
{
    const IgcImportSettings::AltitudeMode altitudeMode = d->pluginSettings.getAltitudeMode();
    int currentIndex = 0;
    while (currentIndex < ui->altitudeComboBox->count() &&
           static_cast<IgcImportSettings::AltitudeMode>(ui->altitudeComboBox->itemData(currentIndex).toInt()) != altitudeMode) {
        ++currentIndex;
    }
    ui->altitudeComboBox->setCurrentIndex(currentIndex);
    ui->enlThresholdSpinBox->setValue(d->pluginSettings.getEnlThresholdPercent());
    switch (altitudeMode) {
    case IgcImportSettings::AltitudeMode::Gnss:
        if (File::hasEarthGravityModel()) {
            ui->convertAltitudeCheckBox->setEnabled(true);
            ui->convertAltitudeCheckBox->setChecked(d->pluginSettings.isConvertAltitudeEnabled());
            ui->convertAltitudeCheckBox->setToolTip(tr("Converts imported height above WGS84 ellipsoid to height above the EGM2008 geoid."));
        } else {
            ui->convertAltitudeCheckBox->setEnabled(false);
            ui->convertAltitudeCheckBox->setChecked(false);
            ui->convertAltitudeCheckBox->setToolTip(tr("No earth gravity model (EGM) is available."));
        }
        break;
    case IgcImportSettings::AltitudeMode::Pressure:
        ui->convertAltitudeCheckBox->setEnabled(false);
        ui->convertAltitudeCheckBox->setChecked(false);
        break;
    }
}

void IgcImportOptionWidget::onAltitudeChanged() noexcept
{
    const IgcImportSettings::AltitudeMode altitudeMode = static_cast<IgcImportSettings::AltitudeMode>(ui->altitudeComboBox->currentData().toInt());
    d->pluginSettings.setAltitudeMode(altitudeMode);
}

void IgcImportOptionWidget::onEnlThresholdChanged(int value) noexcept
{
    d->pluginSettings.setEnlThresholdPercent(value);
}

void IgcImportOptionWidget::onConvertAltitudeChanged(bool enabled) noexcept
{
    d->pluginSettings.setConvertAltitudeEnabled(enabled);
}
