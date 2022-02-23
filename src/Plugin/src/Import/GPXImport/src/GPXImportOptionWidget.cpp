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
#include <QSpinBox>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Version.h"
#include "GPXImportOptionWidget.h"
#include "GPXImportSettings.h"
#include "ui_GPXImportOptionWidget.h"

namespace
{
    constexpr int MaximumAltitude = 99000;
    constexpr int MaximumVelocity = 999;
}

class GPXImportOptionWidgetPrivate
{
public:
    GPXImportOptionWidgetPrivate(GPXImportSettings &theImportSettings) noexcept
        : importSettings(theImportSettings)
    {}

     GPXImportSettings &importSettings;
};

// PUBLIC

GPXImportOptionWidget::GPXImportOptionWidget(GPXImportSettings &importSettings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(new Ui::GPXImportOptionWidget),
      d(std::make_unique<GPXImportOptionWidgetPrivate>(importSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("GPXImportOptionWidget::GPXImportOptionWidget: CREATED");
#endif
}

GPXImportOptionWidget::~GPXImportOptionWidget() noexcept
{
    delete ui;
#ifdef DEBUG
    qDebug("GPXImportOptionWidget::~GPXImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void GPXImportOptionWidget::frenchConnection() noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->defaultAltitudeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GPXImportOptionWidget::onDefaultAltitudeSpinBoxValueChanged);
    connect(ui->defaultVelocitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GPXImportOptionWidget::onDefaultVelocitySpinBoxValueChanged);
#else
    connect(ui->defaultAltitudeSpinBox, &QSpinBox::valueChanged,
            this, &GPXImportOptionWidget::onDefaultAltitudeSpinBoxValueChanged);
    connect(ui->defaultVelocitySpinBox, &QSpinBox::valueChanged,
            this, &GPXImportOptionWidget::onDefaultVelocitySpinBoxValueChanged);
#endif

    connect(&d->importSettings, &GPXImportSettings::defaultsRestored,
            this, &GPXImportOptionWidget::updateUi);
}

void GPXImportOptionWidget::initUi() noexcept
{
    initOptionUi();
}

void GPXImportOptionWidget::initOptionUi() noexcept
{
    ui->defaultAltitudeSpinBox->setRange(0, ::MaximumAltitude);
    ui->defaultVelocitySpinBox->setRange(0, ::MaximumVelocity);
}

void GPXImportOptionWidget::updateOptionUi() noexcept
{
    ui->defaultAltitudeSpinBox->setValue(d->importSettings.m_defaultAltitude);
    ui->defaultVelocitySpinBox->setValue(d->importSettings.m_defaultVelocity);
}

// PRIVATE SLOTS

void GPXImportOptionWidget::onDefaultAltitudeSpinBoxValueChanged(int value) noexcept
{
    d->importSettings.m_defaultAltitude = value;
}

void GPXImportOptionWidget::onDefaultVelocitySpinBoxValueChanged(int value) noexcept
{
    d->importSettings.m_defaultVelocity = value;
}

void GPXImportOptionWidget::updateUi() noexcept
{
    updateOptionUi();
}
