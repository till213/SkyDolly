/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <Kernel/File.h>
#include <Kernel/Version.h>
#include "GpxImportOptionWidget.h"
#include "GpxImportSettings.h"
#include "ui_GpxImportOptionWidget.h"

namespace
{
    constexpr int MaximumAltitude {99999};
    constexpr int MaximumSpeed {999};
}

struct GpxImportOptionWidgetPrivate
{
    GpxImportOptionWidgetPrivate(GpxImportSettings &pluginSettings) noexcept
        : pluginSettings {pluginSettings}
    {}

     GpxImportSettings &pluginSettings;
};

// PUBLIC

GpxImportOptionWidget::GpxImportOptionWidget(GpxImportSettings &pluginSettings, QWidget *parent) noexcept
    : QWidget {parent},
      ui {std::make_unique<Ui::GpxImportOptionWidget>()},
      d {std::make_unique<GpxImportOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

GpxImportOptionWidget::~GpxImportOptionWidget() = default;

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
    connect(ui->convertAltitudeCheckBox, &QCheckBox::checkStateChanged,
            this, &GpxImportOptionWidget::onConvertAltitudeChanged);
    connect(&d->pluginSettings, &GpxImportSettings::changed,
            this, &GpxImportOptionWidget::updateUi);
}

void GpxImportOptionWidget::initUi() noexcept
{
    ui->waypointSelectionComboBox->addItem(tr("Waypoint (<wpt>)"), Enum::underly(GpxImportSettings::GPXElement::Waypoint));
    ui->waypointSelectionComboBox->addItem(tr("Route (<rte>)"), Enum::underly(GpxImportSettings::GPXElement::Route));
    ui->waypointSelectionComboBox->addItem(tr("Track (<trk>)"), Enum::underly(GpxImportSettings::GPXElement::Track));

    ui->positionSelectionComboBox->addItem(tr("Waypoint (<wpt>)"), Enum::underly(GpxImportSettings::GPXElement::Waypoint));
    ui->positionSelectionComboBox->addItem(tr("Route (<rte>)"), Enum::underly(GpxImportSettings::GPXElement::Route));
    ui->positionSelectionComboBox->addItem(tr("Track (<trk>)"), Enum::underly(GpxImportSettings::GPXElement::Track));

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
    const GpxImportSettings::GPXElement waypointSelection = d->pluginSettings.getWaypointSelection();
    int currentIndex = 0;
    while (currentIndex < ui->waypointSelectionComboBox->count() &&
           static_cast<GpxImportSettings::GPXElement>(ui->waypointSelectionComboBox->itemData(currentIndex).toInt()) != waypointSelection) {
        ++currentIndex;
    }
    ui->waypointSelectionComboBox->setCurrentIndex(currentIndex);

    const GpxImportSettings::GPXElement positionSelection = d->pluginSettings.getPositionSelection();
    currentIndex = 0;
    while (currentIndex < ui->positionSelectionComboBox->count() &&
           static_cast<GpxImportSettings::GPXElement>(ui->positionSelectionComboBox->itemData(currentIndex).toInt()) != positionSelection) {
        ++currentIndex;
    }
    ui->positionSelectionComboBox->setCurrentIndex(currentIndex);

    ui->defaultAltitudeSpinBox->setValue(d->pluginSettings.getDefaultAltitude());
    ui->defaultSpeedSpinBox->setValue(d->pluginSettings.getDefaultSpeed());

    if (File::hasEarthGravityModel()) {
        ui->convertAltitudeCheckBox->setEnabled(true);
        ui->convertAltitudeCheckBox->setChecked(d->pluginSettings.isConvertAltitudeEnabled());
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
    d->pluginSettings.setWaypointSelection(waypointSelection);
}

void GpxImportOptionWidget::onPositionSelelectionChanged() noexcept
{
    const GpxImportSettings::GPXElement positionSelection = static_cast<GpxImportSettings::GPXElement>(ui->positionSelectionComboBox->currentData().toInt());
    d->pluginSettings.setPositionSelection(positionSelection);
}

void GpxImportOptionWidget::onDefaultAltitudeChanged(int value) noexcept
{
    d->pluginSettings.setDefaultAltitude(value);
}

void GpxImportOptionWidget::onDefaultSpeedChanged(int value) noexcept
{
    d->pluginSettings.setDefaultSpeed(value);
}

void GpxImportOptionWidget::onConvertAltitudeChanged(Qt::CheckState state) noexcept
{
    d->pluginSettings.setConvertAltitudeEnabled(state == Qt::CheckState::Checked);
}
