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
#include <QComboBox>
#include <QString>

#include "../../../../../Kernel/src/Enum.h"
#include "GPXExportOptionWidget.h"
#include "GPXExportSettings.h"
#include "ui_GPXExportOptionWidget.h"

class GPXExportOptionWidgetPrivate
{
public:
    GPXExportOptionWidgetPrivate(GPXExportSettings &theSettings) noexcept
        : settings(theSettings)
    {}

    GPXExportSettings &settings;
};

// PUBLIC

GPXExportOptionWidget::GPXExportOptionWidget(GPXExportSettings &settings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::GPXExportOptionWidget>()),
      d(std::make_unique<GPXExportOptionWidgetPrivate>(settings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("GPXExportOptionWidget::GPXExportOptionWidget: CREATED");
#endif
}

GPXExportOptionWidget::~GPXExportOptionWidget() noexcept
{
#ifdef DEBUG
    qDebug("GPXExportOptionWidget::~GPXExportOptionWidget: DELETED");
#endif
}

// PRIVATE

void GPXExportOptionWidget::frenchConnection() noexcept
{
    connect(&d->settings, &GPXExportSettings::extendedSettingsChanged,
            this, &GPXExportOptionWidget::updateUi);
    connect(ui->timestampModeComboBox, &QComboBox::currentIndexChanged,
            this, &GPXExportOptionWidget::onTimestampModeChanged);
}

void GPXExportOptionWidget::initUi() noexcept
{
    ui->timestampModeComboBox->addItem(tr("Simulation time"), Enum::toUnderlyingType(GPXExportSettings::TimestampMode::Simulation));
    ui->timestampModeComboBox->addItem(tr("Recording (real world) time"), Enum::toUnderlyingType(GPXExportSettings::TimestampMode::Recording));
}

// PRIVATE SLOTS

void GPXExportOptionWidget::updateUi() noexcept
{
    const GPXExportSettings::TimestampMode timestampMode = d->settings.getTimestampMode();
    int currentIndex = 0;
    while (currentIndex < ui->timestampModeComboBox->count() &&
           static_cast<GPXExportSettings::TimestampMode>(ui->timestampModeComboBox->itemData(currentIndex).toInt()) != timestampMode) {
        ++currentIndex;
    }

    ui->timestampModeComboBox->setCurrentIndex(currentIndex);
    switch (timestampMode) {
    case GPXExportSettings::TimestampMode::Simulation:
        ui->timestampModeComboBox->setToolTip(tr("Timestamps are based on the time set in the flight simulator."));
        break;
    case GPXExportSettings::TimestampMode::Recording:
        ui->timestampModeComboBox->setToolTip(tr("Timestamps are based on the (real world) recording time. This may be useful for GPS-tagging screenshots taken during flight recording."));
        break;
    default:
        break;
    }
}

void GPXExportOptionWidget::onTimestampModeChanged() noexcept
{
    const GPXExportSettings::TimestampMode timestampMode = static_cast<GPXExportSettings::TimestampMode>(ui->timestampModeComboBox->currentData().toInt());
    d->settings.setTimestampMode(timestampMode);
}
