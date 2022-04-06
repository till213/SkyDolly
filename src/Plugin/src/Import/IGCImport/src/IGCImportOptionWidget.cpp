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
#include "IGCImportOptionWidget.h"
#include "IGCImportSettings.h"
#include "ui_IGCImportOptionWidget.h"

class IGCImportOptionWidgetPrivate
{
public:
    IGCImportOptionWidgetPrivate(IGCImportSettings &theImportSettings) noexcept
        : settings(theImportSettings)
    {}

     IGCImportSettings &settings;
};

// PUBLIC

IGCImportOptionWidget::IGCImportOptionWidget(IGCImportSettings &settings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::IGCImportOptionWidget>()),
      d(std::make_unique<IGCImportOptionWidgetPrivate>(settings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("IGCImportOptionWidget::IGCImportOptionWidget: CREATED");
#endif
}

IGCImportOptionWidget::~IGCImportOptionWidget() noexcept
{
#ifdef DEBUG
    qDebug("IGCImportOptionWidget::~IGCImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void IGCImportOptionWidget::frenchConnection() noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->altitudeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &IGCImportOptionWidget::onAltitudeChanged);
    connect(ui->enlThresholdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &IGCImportOptionWidget::onENLThresholdChanged);
#else
    connect(ui->altitudeComboBox, &QComboBox::currentIndexChanged,
            this, &IGCImportOptionWidget::onAltitudeChanged);
    connect(ui->enlThresholdSpinBox, &QSpinBox::valueChanged,
            this, &IGCImportOptionWidget::onENLThresholdChanged);
#endif

    connect(&d->settings, &IGCImportSettings::extendedSettingsChanged,
            this, &IGCImportOptionWidget::updateUi);
}

void IGCImportOptionWidget::initUi() noexcept
{
    ui->altitudeComboBox->addItem(tr("GNSS altitude"), Enum::toUnderlyingType(IGCImportSettings::AltitudeMode::Gnss));
    ui->altitudeComboBox->addItem(tr("Pressure altitude"), Enum::toUnderlyingType(IGCImportSettings::AltitudeMode::Pressure));

    // Percent [0, 100]
    ui->enlThresholdSpinBox->setRange(0, 100);
    ui->enlThresholdSpinBox->setSuffix("%");
    ui->enlThresholdSpinBox->setSingleStep(5);
}

// PRIVATE SLOTS

void IGCImportOptionWidget::onAltitudeChanged() noexcept
{
    const IGCImportSettings::AltitudeMode altitudeMode = static_cast<IGCImportSettings::AltitudeMode>(ui->altitudeComboBox->currentData().toInt());
    d->settings.setAltitudeMode(altitudeMode);
}

void IGCImportOptionWidget::onENLThresholdChanged(int value) noexcept
{
    d->settings.setEnlThresholdPercent(value);
}

void IGCImportOptionWidget::updateUi() noexcept
{
    const IGCImportSettings::AltitudeMode altitudeMode = d->settings.getAltitudeMode();
    int currentIndex = 0;
    while (currentIndex < ui->altitudeComboBox->count() &&
           static_cast<IGCImportSettings::AltitudeMode>(ui->altitudeComboBox->itemData(currentIndex).toInt()) != altitudeMode) {
        ++currentIndex;
    }
    ui->altitudeComboBox->setCurrentIndex(currentIndex);

    ui->enlThresholdSpinBox->setValue(d->settings.getEnlThresholdPercent());
}
