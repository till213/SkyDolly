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

#include "../../../../../Kernel/src/Enum.h"
#include "KMLImportOptionWidget.h"
#include "KMLImportSettings.h"
#include "ui_KMLImportOptionWidget.h"

class KMLImportOptionWidgetPrivate
{
public:
    KMLImportOptionWidgetPrivate(KMLImportSettings &theImportSettings) noexcept
        : importSettings(theImportSettings)
    {}

     KMLImportSettings &importSettings;
};

// PUBLIC

KMLImportOptionWidget::KMLImportOptionWidget(KMLImportSettings &importSettings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(new Ui::KMLImportOptionWidget),
      d(std::make_unique<KMLImportOptionWidgetPrivate>(importSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("KMLImportOptionWidget::KMLImportOptionWidget: CREATED");
#endif
}

KMLImportOptionWidget::~KMLImportOptionWidget() noexcept
{
    delete ui;
#ifdef DEBUG
    qDebug("KMLImportOptionWidget::~KMLImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void KMLImportOptionWidget::frenchConnection() noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &KMLImportOptionWidget::onFormatComboBoxCurrentIndexChanged);
#else
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &KMLImportOptionWidget::onFormatComboBoxCurrentIndexChanged);
#endif
    connect(&d->importSettings, &KMLImportSettings::defaultsRestored,
            this, &KMLImportOptionWidget::updateUi);
}

void KMLImportOptionWidget::initUi() noexcept
{
    initOptionUi();
}

void KMLImportOptionWidget::initOptionUi() noexcept
{
    ui->formatComboBox->addItem("FlightAware", Enum::toUnderlyingType(KMLImportSettings::Format::FlightAware));
    ui->formatComboBox->addItem("FlightRadar24", Enum::toUnderlyingType(KMLImportSettings::Format::FlightRadar24));
    ui->formatComboBox->addItem(tr("Generic KML with track data"), Enum::toUnderlyingType(KMLImportSettings::Format::Generic));
}

void KMLImportOptionWidget::updateOptionUi() noexcept
{
    int currentIndex = 0;
    while (currentIndex < ui->formatComboBox->count() &&
           static_cast<KMLImportSettings::Format>(ui->formatComboBox->itemData(currentIndex).toInt()) != d->importSettings.m_format) {
        ++currentIndex;
    }
    ui->formatComboBox->setCurrentIndex(currentIndex);
}

// PRIVATE SLOTS

void KMLImportOptionWidget::onFormatComboBoxCurrentIndexChanged([[maybe_unused]]int index) noexcept
{
    d->importSettings.m_format = static_cast<KMLImportSettings::Format>(ui->formatComboBox->currentData().toInt());
}

void KMLImportOptionWidget::updateUi() noexcept
{
    updateOptionUi();
}
