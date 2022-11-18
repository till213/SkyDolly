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
#include <QWidget>
#include <QComboBox>

#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include "CsvLocationImportOptionWidget.h"
#include "CsvLocationImportSettings.h"
#include "ui_CsvLocationImportOptionWidget.h"

struct CsvLocationImportOptionWidgetPrivate
{
    CsvLocationImportOptionWidgetPrivate(CsvLocationImportSettings &theImportSettings) noexcept
        : settings(theImportSettings)
    {}

     CsvLocationImportSettings &settings;
};

// PUBLIC

CsvLocationImportOptionWidget::CsvLocationImportOptionWidget(CsvLocationImportSettings &settings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::CsvLocationImportOptionWidget>()),
      d(std::make_unique<CsvLocationImportOptionWidgetPrivate>(settings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

CsvLocationImportOptionWidget::~CsvLocationImportOptionWidget() = default;

// PRIVATE

void CsvLocationImportOptionWidget::frenchConnection() noexcept
{
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &CsvLocationImportOptionWidget::onFormatChanged);
    connect(ui->defaultAltitudeSpinBox, &QSpinBox::valueChanged,
            this, &CsvLocationImportOptionWidget::onDefaultAltitudeChanged);
    connect(ui->defaultIndicatedAirspeedSpinBox, &QSpinBox::valueChanged,
            this, &CsvLocationImportOptionWidget::onDefaultIndicatedAirspeedChanged);
    connect(&d->settings, &CsvLocationImportSettings::extendedSettingsChanged,
            this, &CsvLocationImportOptionWidget::updateUi);
}

void CsvLocationImportOptionWidget::initUi() noexcept
{
    ui->formatComboBox->addItem("Sky Dolly", Enum::toUnderlyingType(CsvLocationImportSettings::Format::SkyDolly));
    ui->formatComboBox->addItem("Little Navmap", Enum::toUnderlyingType(CsvLocationImportSettings::Format::LittleNavmap));

    ui->defaultAltitudeSpinBox->setMinimum(Const::MinimumAltitude);
    ui->defaultAltitudeSpinBox->setMaximum(Const::MaximumAltitude);
    ui->defaultAltitudeSpinBox->setSuffix(tr(" feet"));

    ui->defaultIndicatedAirspeedSpinBox->setMinimum(Const::MinimumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setMaximum(Const::MaximumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setSuffix(tr(" knots"));
}

// PRIVATE SLOTS

void CsvLocationImportOptionWidget::updateUi() noexcept
{
    const CsvLocationImportSettings::Format format = d->settings.getFormat();
    int currentIndex = 0;
    while (currentIndex < ui->formatComboBox->count() &&
           static_cast<CsvLocationImportSettings::Format>(ui->formatComboBox->itemData(currentIndex).toInt()) != format) {
        ++currentIndex;
    }
    ui->formatComboBox->setCurrentIndex(currentIndex);
    if (format != CsvLocationImportSettings::Format::SkyDolly) {
        ui->defaultAltitudeSpinBox->setValue(d->settings.getDefaultAltitude());
        ui->defaultIndicatedAirspeedSpinBox->setValue(d->settings.getDefaultIndicatedAirspeed());
        ui->defaultAltitudeSpinBox->setEnabled(true);
        ui->defaultIndicatedAirspeedSpinBox->setEnabled(true);
    } else {
        ui->defaultAltitudeSpinBox->setEnabled(false);
        ui->defaultIndicatedAirspeedSpinBox->setEnabled(false);
    }
}

void CsvLocationImportOptionWidget::onFormatChanged([[maybe_unused]]int index) noexcept
{
    const CsvLocationImportSettings::Format format = static_cast<CsvLocationImportSettings::Format>(ui->formatComboBox->currentData().toInt());
    d->settings.setFormat(format);
}

void CsvLocationImportOptionWidget::onDefaultAltitudeChanged(int value) noexcept
{
    d->settings.setDefaultAltitdue(value);
}

void CsvLocationImportOptionWidget::onDefaultIndicatedAirspeedChanged(int value) noexcept
{
    d->settings.setDefaultIndicatedAirspeed(value);
}
