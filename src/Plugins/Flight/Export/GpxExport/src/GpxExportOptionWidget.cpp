/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#include <QCheckBox>
#include <QString>

#include <Kernel/Enum.h>
#include <Kernel/File.h>
#include "GpxExportOptionWidget.h"
#include "GpxExportSettings.h"
#include "ui_GpxExportOptionWidget.h"

struct GpxExportOptionWidgetPrivate
{
    GpxExportOptionWidgetPrivate(GpxExportSettings &pluginSettings) noexcept
        : pluginSettings {pluginSettings}
    {}

    GpxExportSettings &pluginSettings;
};

// PUBLIC

GpxExportOptionWidget::GpxExportOptionWidget(GpxExportSettings &pluginSettings, QWidget *parent) noexcept
    : QWidget {parent},
      ui {std::make_unique<Ui::GpxExportOptionWidget>()},
      d {std::make_unique<GpxExportOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

GpxExportOptionWidget::~GpxExportOptionWidget() = default;

// PRIVATE

void GpxExportOptionWidget::frenchConnection() noexcept
{
    connect(ui->timestampModeComboBox, &QComboBox::currentIndexChanged,
            this, &GpxExportOptionWidget::onTimestampModeChanged);
    connect(ui->exportGeoidHeightCheckBox, &QCheckBox::toggled,
            this, &GpxExportOptionWidget::onExportGeoidHeightChanged);
    connect(&d->pluginSettings, &GpxExportSettings::changed,
            this, &GpxExportOptionWidget::updateUi);
}

void GpxExportOptionWidget::initUi() noexcept
{
    ui->timestampModeComboBox->addItem(tr("Simulation time"), Enum::underly(GpxExportSettings::TimestampMode::Simulation));
    ui->timestampModeComboBox->addItem(tr("Recording (real world) time"), Enum::underly(GpxExportSettings::TimestampMode::Recording));
}

// PRIVATE SLOTS

void GpxExportOptionWidget::updateUi() noexcept
{
    const auto timestampMode = d->pluginSettings.getTimestampMode();
    int currentIndex = 0;
    while (currentIndex < ui->timestampModeComboBox->count() &&
           static_cast<GpxExportSettings::TimestampMode>(ui->timestampModeComboBox->itemData(currentIndex).toInt()) != timestampMode) {
        ++currentIndex;
    }
    ui->timestampModeComboBox->setCurrentIndex(currentIndex);
    switch (timestampMode) {
    case GpxExportSettings::TimestampMode::Simulation:
        ui->timestampModeComboBox->setToolTip(tr("Timestamps are based on the flight simulator time."));
        break;
    case GpxExportSettings::TimestampMode::Recording:
        ui->timestampModeComboBox->setToolTip(tr("Timestamps are based on the real world recording time. This may be useful for GPS-tagging screenshots taken during flight recording."));
        break;
    }

    if (File::hasEarthGravityModel()) {
        const auto enable = d->pluginSettings.isGeoidHeightExportEnabled();
        ui->exportGeoidHeightCheckBox->setChecked(enable);
        ui->exportGeoidHeightCheckBox->setEnabled(true);
        if (enable) {
            ui->exportGeoidHeightCheckBox->setToolTip(tr("Elevation data (tag <ele>) and the geoid height (tag <geoidheight>) are exported."));
        } else {
            ui->exportGeoidHeightCheckBox->setToolTip(tr("Only elevation data (tag <ele>) is exported."));
        }
    } else {
        ui->exportGeoidHeightCheckBox->setChecked(false);
        ui->exportGeoidHeightCheckBox->setEnabled(false);
        ui->exportGeoidHeightCheckBox->setToolTip(tr("No earth gravity model (EGM) is available."));
    }
}

void GpxExportOptionWidget::onTimestampModeChanged() noexcept
{
    const auto timestampMode = static_cast<GpxExportSettings::TimestampMode>(ui->timestampModeComboBox->currentData().toInt());
    d->pluginSettings.setTimestampMode(timestampMode);
}

void GpxExportOptionWidget::onExportGeoidHeightChanged(bool enable) noexcept
{
    d->pluginSettings.setGeoidHeightExportEnabled(enable);
}
