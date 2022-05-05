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
#include <Kernel/Version.h>
#include "CsvImportOptionWidget.h"
#include "CsvImportSettings.h"
#include "ui_CsvImportOptionWidget.h"

class CsvImportOptionWidgetPrivate
{
public:
    CsvImportOptionWidgetPrivate(CsvImportSettings &theImportSettings) noexcept
        : settings(theImportSettings)
    {}

     CsvImportSettings &settings;
};

// PUBLIC

CsvImportOptionWidget::CsvImportOptionWidget(CsvImportSettings &settings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::CsvImportOptionWidget>()),
      d(std::make_unique<CsvImportOptionWidgetPrivate>(settings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("CsvImportOptionWidget::CsvImportOptionWidget: CREATED");
#endif
}

CsvImportOptionWidget::~CsvImportOptionWidget() noexcept
{
#ifdef DEBUG
    qDebug("CsvImportOptionWidget::~CsvImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void CsvImportOptionWidget::frenchConnection() noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CsvImportOptionWidget::onFormatChanged);
#else
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &CsvImportOptionWidget::onFormatChanged);
#endif
    connect(&d->settings, &CsvImportSettings::extendedSettingsChanged,
            this, &CsvImportOptionWidget::updateUi);
}

void CsvImportOptionWidget::initUi() noexcept
{
    ui->formatComboBox->addItem(Version::getApplicationName(), Enum::toUnderlyingType(CsvImportSettings::Format::SkyDolly));
    ui->formatComboBox->addItem("FlightRadar24", Enum::toUnderlyingType(CsvImportSettings::Format::FlightRadar24));
    ui->formatComboBox->addItem("Flight Recorder", Enum::toUnderlyingType(CsvImportSettings::Format::FlightRecorder));
}

// PRIVATE SLOTS

void CsvImportOptionWidget::updateUi() noexcept
{
    const CsvImportSettings::Format format = d->settings.getFormat();
    int currentIndex = 0;
    while (currentIndex < ui->formatComboBox->count() &&
           static_cast<CsvImportSettings::Format>(ui->formatComboBox->itemData(currentIndex).toInt()) != format) {
        ++currentIndex;
    }
    ui->formatComboBox->setCurrentIndex(currentIndex);
}

void CsvImportOptionWidget::onFormatChanged([[maybe_unused]]int index) noexcept
{
    const CsvImportSettings::Format format = static_cast<CsvImportSettings::Format>(ui->formatComboBox->currentData().toInt());
    d->settings.setFormat(format);
}
