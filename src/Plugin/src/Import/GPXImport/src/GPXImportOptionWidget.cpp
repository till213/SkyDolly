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
    constexpr int MaximumAltitude = 99999;
    constexpr int MaximumVelocity = 999;
}

class GPXImportOptionWidgetPrivate
{
public:
    GPXImportOptionWidgetPrivate(GPXImportSettings &theImportSettings) noexcept
        : settings(theImportSettings)
    {}

     GPXImportSettings &settings;
};

// PUBLIC

GPXImportOptionWidget::GPXImportOptionWidget(GPXImportSettings &settings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::GPXImportOptionWidget>()),
      d(std::make_unique<GPXImportOptionWidgetPrivate>(settings))
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
#ifdef DEBUG
    qDebug("GPXImportOptionWidget::~GPXImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void GPXImportOptionWidget::frenchConnection() noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->waypointSelectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GPXImportOptionWidget::onWaypointSelelectionChanged);
    connect(ui->positionSelectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GPXImportOptionWidget::onPositionSelelectionChanged);
    connect(ui->defaultAltitudeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GPXImportOptionWidget::onDefaultAltitudeChanged);
    connect(ui->defaultVelocitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &GPXImportOptionWidget::onDefaultVelocityChanged);
#else
    connect(ui->waypointSelectionComboBox, &QComboBox::currentIndexChanged,
            this, &GPXImportOptionWidget::onWaypointSelelectionChanged);
    connect(ui->positionSelectionComboBox, &QComboBox::currentIndexChanged,
            this, &GPXImportOptionWidget::onPositionSelelectionChanged);
    connect(ui->defaultAltitudeSpinBox, &QSpinBox::valueChanged,
            this, &GPXImportOptionWidget::onDefaultAltitudeChanged);
    connect(ui->defaultVelocitySpinBox, &QSpinBox::valueChanged,
            this, &GPXImportOptionWidget::onDefaultVelocityChanged);
#endif

    connect(&d->settings, &GPXImportSettings::extendedSettingsChanged,
            this, &GPXImportOptionWidget::updateUi);
}

void GPXImportOptionWidget::initUi() noexcept
{
    ui->waypointSelectionComboBox->addItem(tr("Waypoint (<wpt>)"), Enum::toUnderlyingType(GPXImportSettings::GPXElement::Waypoint));
    ui->waypointSelectionComboBox->addItem(tr("Route (<rte>)"), Enum::toUnderlyingType(GPXImportSettings::GPXElement::Route));
    ui->waypointSelectionComboBox->addItem(tr("Track (<trk>)"), Enum::toUnderlyingType(GPXImportSettings::GPXElement::Track));

    ui->positionSelectionComboBox->addItem(tr("Waypoint (<wpt>)"), Enum::toUnderlyingType(GPXImportSettings::GPXElement::Waypoint));
    ui->positionSelectionComboBox->addItem(tr("Route (<rte>)"), Enum::toUnderlyingType(GPXImportSettings::GPXElement::Route));
    ui->positionSelectionComboBox->addItem(tr("Track (<trk>)"), Enum::toUnderlyingType(GPXImportSettings::GPXElement::Track));

    ui->defaultAltitudeSpinBox->setRange(0, ::MaximumAltitude);
    ui->defaultAltitudeSpinBox->setSuffix(tr(" feet"));
    ui->defaultAltitudeSpinBox->setSingleStep(100);
    ui->defaultAltitudeSpinBox->setGroupSeparatorShown(true);
    ui->defaultVelocitySpinBox->setRange(0, ::MaximumVelocity);
    ui->defaultVelocitySpinBox->setSuffix(tr(" knots"));
    ui->defaultVelocitySpinBox->setSingleStep(5);
    ui->defaultVelocitySpinBox->setGroupSeparatorShown(true);
}

// PRIVATE SLOTS

void GPXImportOptionWidget::onWaypointSelelectionChanged() noexcept
{
    const GPXImportSettings::GPXElement waypointSelection = static_cast<GPXImportSettings::GPXElement>(ui->waypointSelectionComboBox->currentData().toInt());
    d->settings.setWaypointSelection(waypointSelection);
}

void GPXImportOptionWidget::onPositionSelelectionChanged() noexcept
{
    const GPXImportSettings::GPXElement positionSelection = static_cast<GPXImportSettings::GPXElement>(ui->positionSelectionComboBox->currentData().toInt());
    d->settings.setPositionSelection(positionSelection);
}

void GPXImportOptionWidget::onDefaultAltitudeChanged(int value) noexcept
{
    d->settings.setDefaultAltitude(value);
}

void GPXImportOptionWidget::onDefaultVelocityChanged(int value) noexcept
{
    d->settings.setDefaultVelocity(value);
}

void GPXImportOptionWidget::updateUi() noexcept
{
    const GPXImportSettings::GPXElement waypointSelection = d->settings.getWaypointSelection();
    int currentIndex = 0;
    while (currentIndex < ui->waypointSelectionComboBox->count() &&
           static_cast<GPXImportSettings::GPXElement>(ui->waypointSelectionComboBox->itemData(currentIndex).toInt()) != waypointSelection) {
        ++currentIndex;
    }
    ui->waypointSelectionComboBox->setCurrentIndex(currentIndex);

    const GPXImportSettings::GPXElement positionSelection = d->settings.getPositionSelection();
    currentIndex = 0;
    while (currentIndex < ui->positionSelectionComboBox->count() &&
           static_cast<GPXImportSettings::GPXElement>(ui->positionSelectionComboBox->itemData(currentIndex).toInt()) != positionSelection) {
        ++currentIndex;
    }
    ui->positionSelectionComboBox->setCurrentIndex(currentIndex);

    ui->defaultAltitudeSpinBox->setValue(d->settings.getDefaultAltitude());
    ui->defaultVelocitySpinBox->setValue(d->settings.getDefaultVelocity());
}
