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
#include <Persistence/Service/EnumerationService.h>
#include <Widget/EnumerationComboBox.h>
#include "KmlLocationImportOptionWidget.h"
#include "KmlLocationImportSettings.h"
#include "ui_KmlLocationImportOptionWidget.h"

struct KmlLocationImportOptionWidgetPrivate
{
    KmlLocationImportOptionWidgetPrivate(KmlLocationImportSettings &pluginSettings) noexcept
        : pluginSettings {pluginSettings}
    {}

    KmlLocationImportSettings &pluginSettings;
};

// PUBLIC

KmlLocationImportOptionWidget::KmlLocationImportOptionWidget(KmlLocationImportSettings &pluginSettings, QWidget *parent) noexcept
    : QWidget {parent},
    ui {std::make_unique<Ui::KmlLocationImportOptionWidget>()},
    d {std::make_unique<KmlLocationImportOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

KmlLocationImportOptionWidget::~KmlLocationImportOptionWidget() = default;

// PRIVATE

void KmlLocationImportOptionWidget::frenchConnection() noexcept
{
    connect(ui->defaultCountryComboBox, &EnumerationComboBox::currentIndexChanged,
            this, &KmlLocationImportOptionWidget::onDefaultCountryChanged);
    connect(ui->defaultAltitudeSpinBox, &QSpinBox::valueChanged,
            this, &KmlLocationImportOptionWidget::onDefaultAltitudeChanged);
    connect(ui->defaultIndicatedAirspeedSpinBox, &QSpinBox::valueChanged,
            this, &KmlLocationImportOptionWidget::onDefaultIndicatedAirspeedChanged);
    connect(&d->pluginSettings, &KmlLocationImportSettings::changed,
            this, &KmlLocationImportOptionWidget::updateUi);
}

void KmlLocationImportOptionWidget::initUi() noexcept
{
    ui->defaultCountryComboBox->setEnumerationName(EnumerationService::Country);
    ui->defaultCountryComboBox->setEditable(true);

    ui->defaultAltitudeSpinBox->setMinimum(Const::MinimumAltitude);
    ui->defaultAltitudeSpinBox->setMaximum(Const::MaximumAltitude);
    ui->defaultAltitudeSpinBox->setSuffix(tr(" feet"));

    ui->defaultIndicatedAirspeedSpinBox->setMinimum(Const::MinimumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setMaximum(Const::MaximumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setSuffix(tr(" knots"));
}

// PRIVATE SLOTS

void KmlLocationImportOptionWidget::updateUi() noexcept
{
    ui->defaultCountryComboBox->setCurrentId(d->pluginSettings.getDefaultCountryId());
    ui->defaultCountryComboBox->setCurrentId(d->pluginSettings.getDefaultCountryId());
    ui->defaultAltitudeSpinBox->setValue(d->pluginSettings.getDefaultAltitude());
    ui->defaultIndicatedAirspeedSpinBox->setValue(d->pluginSettings.getDefaultIndicatedAirspeed());
}

void KmlLocationImportOptionWidget::onDefaultCountryChanged([[maybe_unused]]int index) noexcept
{
    d->pluginSettings.setDefaultCountryId(ui->defaultCountryComboBox->getCurrentId());
}

void KmlLocationImportOptionWidget::onDefaultAltitudeChanged(int value) noexcept
{
    d->pluginSettings.setDefaultAltitude(value);
}

void KmlLocationImportOptionWidget::onDefaultIndicatedAirspeedChanged(int value) noexcept
{
    d->pluginSettings.setDefaultIndicatedAirspeed(value);
}
