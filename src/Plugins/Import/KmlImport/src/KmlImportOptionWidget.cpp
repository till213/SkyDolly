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

#include <Kernel/Enum.h>
#include "KmlImportOptionWidget.h"
#include "KmlImportSettings.h"
#include "ui_KmlImportOptionWidget.h"

class KmlImportOptionWidgetPrivate
{
public:
    KmlImportOptionWidgetPrivate(KmlImportSettings &theImportSettings) noexcept
        : settings(theImportSettings)
    {}

    KmlImportSettings &settings;
};

// PUBLIC

KmlImportOptionWidget::KmlImportOptionWidget(KmlImportSettings &settings, QWidget *parent) noexcept
   : QWidget(parent),
     ui(std::make_unique<Ui::KmlImportOptionWidget>()),
      d(std::make_unique<KmlImportOptionWidgetPrivate>(settings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("KmlImportOptionWidget::KmlImportOptionWidget: CREATED");
#endif
}

KmlImportOptionWidget::~KmlImportOptionWidget() noexcept
{
#ifdef DEBUG
    qDebug("KmlImportOptionWidget::~KmlImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void KmlImportOptionWidget::frenchConnection() noexcept
{
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &KmlImportOptionWidget::onFormatChanged);
    connect(&d->settings, &KmlImportSettings::extendedSettingsChanged,
            this, &KmlImportOptionWidget::updateUi);
}

void KmlImportOptionWidget::initUi() noexcept
{
    ui->formatComboBox->addItem("FlightAware", Enum::toUnderlyingType(KmlImportSettings::Format::FlightAware));
    ui->formatComboBox->addItem("FlightRadar24", Enum::toUnderlyingType(KmlImportSettings::Format::FlightRadar24));
    ui->formatComboBox->addItem(tr("Generic KML with track data"), Enum::toUnderlyingType(KmlImportSettings::Format::Generic));
}

// PRIVATE SLOTS

void KmlImportOptionWidget::updateUi() noexcept
{
    const KmlImportSettings::Format format = d->settings.getFormat();
    int currentIndex = 0;
    while (currentIndex < ui->formatComboBox->count() &&
           static_cast<KmlImportSettings::Format>(ui->formatComboBox->itemData(currentIndex).toInt()) != format) {
        ++currentIndex;
    }
    ui->formatComboBox->setCurrentIndex(currentIndex);
}

void KmlImportOptionWidget::onFormatChanged() noexcept
{
    const KmlImportSettings::Format format = static_cast<KmlImportSettings::Format>(ui->formatComboBox->currentData().toInt());
    d->settings.setFormat(format);
}
