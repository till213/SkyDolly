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

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Kernel/Version.h>
#include "GpxImportOptionWidget.h"
#include "GpxImportSettings.h"
#include "ui_GpxImportOptionWidget.h"

namespace
{
    constexpr int MaximumAltitude = 99999;
    constexpr int MaximumSpeed = 999;
}

class GpxImportOptionWidgetPrivate
{
public:
    GpxImportOptionWidgetPrivate(GpxImportSettings &theImportSettings) noexcept
        : settings(theImportSettings)
    {}

     GpxImportSettings &settings;
};

// PUBLIC

GpxImportOptionWidget::GpxImportOptionWidget(GpxImportSettings &settings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::GpxImportOptionWidget>()),
      d(std::make_unique<GpxImportOptionWidgetPrivate>(settings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("GpxImportOptionWidget::GpxImportOptionWidget: CREATED");
#endif
}

GpxImportOptionWidget::~GpxImportOptionWidget() noexcept
{
#ifdef DEBUG
    qDebug("GpxImportOptionWidget::~GpxImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void GpxImportOptionWidget::frenchConnection() noexcept
{
    connect(ui->waypointSelectionComboBox, &QComboBox::currentIndexChanged,
            this, &GpxImportOptionWidget::onWaypointSelelectionChanged);
    connect(ui->positionSelectionComboBox, &QComboBox::currentIndexChanged,
            this, &GpxImportOptionWidget::onPositionSelelectionChanged);
    connect(ui->defaultAltitudeSpinBox, &QSpinBox::valueChanged,
            this, &GpxImportOptionWidget::onDefaultAltitudeChanged);
    connect(ui->defaultSpeedSpinBox, &QSpinBox::valueChanged,
            this, &GpxImportOptionWidget::onDefaultSpeedChanged);
    connect(ui->convertAltitudeCheckBox, &QCheckBox::stateChanged,
            this, &GpxImportOptionWidget::onConvertAltitudeChanged);
    connect(&d->settings, &GpxImportSettings::extendedSettingsChanged,
            this, &GpxImportOptionWidget::updateUi);
}

void GpxImportOptionWidget::initUi() noexcept
{
    ui->waypointSelectionComboBox->addItem(tr("Waypoint (<wpt>)"), Enum::toUnderlyingType(GpxImportSettings::GPXElement::Waypoint));
    ui->waypointSelectionComboBox->addItem(tr("Route (<rte>)"), Enum::toUnderlyingType(GpxImportSettings::GPXElement::Route));
    ui->waypointSelectionComboBox->addItem(tr("Track (<trk>)"), Enum::toUnderlyingType(GpxImportSettings::GPXElement::Track));

    ui->positionSelectionComboBox->addItem(tr("Waypoint (<wpt>)"), Enum::toUnderlyingType(GpxImportSettings::GPXElement::Waypoint));
    ui->positionSelectionComboBox->addItem(tr("Route (<rte>)"), Enum::toUnderlyingType(GpxImportSettings::GPXElement::Route));
    ui->positionSelectionComboBox->addItem(tr("Track (<trk>)"), Enum::toUnderlyingType(GpxImportSettings::GPXElement::Track));

    ui->defaultAltitudeSpinBox->setRange(0, ::MaximumAltitude);
    ui->defaultAltitudeSpinBox->setSuffix(tr(" feet"));
    ui->defaultAltitudeSpinBox->setSingleStep(100);
    ui->defaultAltitudeSpinBox->setGroupSeparatorShown(true);
    ui->defaultSpeedSpinBox->setRange(0, ::MaximumSpeed);
    ui->defaultSpeedSpinBox->setSuffix(tr(" knots"));
    ui->defaultSpeedSpinBox->setSingleStep(5);
    ui->defaultSpeedSpinBox->setGroupSeparatorShown(true);
}

// PRIVATE SLOTS

void GpxImportOptionWidget::updateUi() noexcept
{
    const GpxImportSettings::GPXElement waypointSelection = d->settings.getWaypointSelection();
    int currentIndex = 0;
    while (currentIndex < ui->waypointSelectionComboBox->count() &&
           static_cast<GpxImportSettings::GPXElement>(ui->waypointSelectionComboBox->itemData(currentIndex).toInt()) != waypointSelection) {
        ++currentIndex;
    }
    ui->waypointSelectionComboBox->setCurrentIndex(currentIndex);

    const GpxImportSettings::GPXElement positionSelection = d->settings.getPositionSelection();
    currentIndex = 0;
    while (currentIndex < ui->positionSelectionComboBox->count() &&
           static_cast<GpxImportSettings::GPXElement>(ui->positionSelectionComboBox->itemData(currentIndex).toInt()) != positionSelection) {
        ++currentIndex;
    }
    ui->positionSelectionComboBox->setCurrentIndex(currentIndex);

    ui->defaultAltitudeSpinBox->setValue(d->settings.getDefaultAltitude());
    ui->defaultSpeedSpinBox->setValue(d->settings.getDefaultSpeed());

    if (Settings::getInstance().hasEarthGravityModel()) {
        ui->convertAltitudeCheckBox->setEnabled(true);
        ui->convertAltitudeCheckBox->setChecked(d->settings.isConvertAltitudeEnabled());
        ui->convertAltitudeCheckBox->setToolTip(tr("Converts imported height above WGS84 ellipsoid to height above the EGM2008 geoid."));
    } else {
        ui->convertAltitudeCheckBox->setEnabled(false);
        ui->convertAltitudeCheckBox->setChecked(false);
        ui->convertAltitudeCheckBox->setToolTip(tr("No earth gravity model (EGM) is available."));
    }
}

void GpxImportOptionWidget::onWaypointSelelectionChanged() noexcept
{
    const GpxImportSettings::GPXElement waypointSelection = static_cast<GpxImportSettings::GPXElement>(ui->waypointSelectionComboBox->currentData().toInt());
    d->settings.setWaypointSelection(waypointSelection);
}

void GpxImportOptionWidget::onPositionSelelectionChanged() noexcept
{
    const GpxImportSettings::GPXElement positionSelection = static_cast<GpxImportSettings::GPXElement>(ui->positionSelectionComboBox->currentData().toInt());
    d->settings.setPositionSelection(positionSelection);
}

void GpxImportOptionWidget::onDefaultAltitudeChanged(int value) noexcept
{
    d->settings.setDefaultAltitude(value);
}

void GpxImportOptionWidget::onDefaultSpeedChanged(int value) noexcept
{
    d->settings.setDefaultSpeed(value);
}

void GpxImportOptionWidget::onConvertAltitudeChanged(int state) noexcept
{
    d->settings.setConvertAltitudeEnabled(state == Qt::Checked);
}
