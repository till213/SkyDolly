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
#include <QCoreApplication>
#include <QString>
#include <QFile>
#include <QDir>
#include <QComboBox>

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Version.h"
#include "IGCImportOptionWidget.h"
#include "IGCImportSettings.h"
#include "ui_IGCImportOptionWidget.h"

class IGCImportOptionWidgetPrivate
{
public:
    IGCImportOptionWidgetPrivate(IGCImportSettings &theImportSettings) noexcept
        : importSettings(theImportSettings)
    {}

     IGCImportSettings &importSettings;
};

// PUBLIC

IGCImportOptionWidget::IGCImportOptionWidget(IGCImportSettings &importSettings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(new Ui::IGCImportOptionWidget),
      d(std::make_unique<IGCImportOptionWidgetPrivate>(importSettings))
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
    delete ui;
#ifdef DEBUG
    qDebug("IGCImportOptionWidget::~IGCImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void IGCImportOptionWidget::frenchConnection() noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->altitudeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &IGCImportOptionWidget::onaltitudeComboBoxCurrentIndexChanged);
#else
    connect(ui->altitudeComboBox, &QComboBox::currentIndexChanged,
            this, &IGCImportOptionWidget::onAltitudeComboBoxCurrentIndexChanged);
#endif
    connect(&d->importSettings, &IGCImportSettings::defaultsRestored,
            this, &IGCImportOptionWidget::updateUi);
}

void IGCImportOptionWidget::initUi() noexcept
{
    initOptionUi();
}

void IGCImportOptionWidget::initOptionUi() noexcept
{
    ui->altitudeComboBox->addItem(tr("GNSS altitdue"), Enum::toUnderlyingType(IGCImportSettings::Altitude::GnssAltitude));
    ui->altitudeComboBox->addItem(tr("Pressure altitdue"), Enum::toUnderlyingType(IGCImportSettings::Altitude::PressureAltitude));
}

void IGCImportOptionWidget::updateOptionUi() noexcept
{
    int currentIndex = 0;
    while (currentIndex < ui->altitudeComboBox->count() &&
           static_cast<IGCImportSettings::Altitude>(ui->altitudeComboBox->itemData(currentIndex).toInt()) != d->importSettings.m_altitude) {
        ++currentIndex;
    }
    ui->altitudeComboBox->setCurrentIndex(currentIndex);
}

// PRIVATE SLOTS

void IGCImportOptionWidget::onAltitudeComboBoxCurrentIndexChanged([[maybe_unused]]int index) noexcept
{
    d->importSettings.m_altitude = static_cast<IGCImportSettings::Altitude>(ui->altitudeComboBox->currentData().toInt());
}

void IGCImportOptionWidget::updateUi() noexcept
{
    updateOptionUi();
}
