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
#include <QComboBox>

#include <Kernel/Enum.h>
#include <Kernel/Version.h>
#include "CsvImportOptionWidget.h"
#include "CsvImportSettings.h"
#include "ui_CsvImportOptionWidget.h"

struct CsvImportOptionWidgetPrivate
{
    CsvImportOptionWidgetPrivate(CsvImportSettings &pluginSettings) noexcept
        : pluginSettings(pluginSettings)
    {}

     CsvImportSettings &pluginSettings;
};

// PUBLIC

CsvImportOptionWidget::CsvImportOptionWidget(CsvImportSettings &pluginSettings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::CsvImportOptionWidget>()),
      d(std::make_unique<CsvImportOptionWidgetPrivate>(pluginSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

CsvImportOptionWidget::~CsvImportOptionWidget() = default;

// PRIVATE

void CsvImportOptionWidget::frenchConnection() noexcept
{
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &CsvImportOptionWidget::onFormatChanged);
    connect(&d->pluginSettings, &CsvImportSettings::changed,
            this, &CsvImportOptionWidget::updateUi);
}

void CsvImportOptionWidget::initUi() noexcept
{
    ui->formatComboBox->addItem("Flightradar24", Enum::underly(CsvImportSettings::Format::Flightradar24));
    ui->formatComboBox->addItem("Flight Recorder", Enum::underly(CsvImportSettings::Format::FlightRecorder));
}

// PRIVATE SLOTS

void CsvImportOptionWidget::updateUi() noexcept
{
    const CsvImportSettings::Format format = d->pluginSettings.getFormat();
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
    d->pluginSettings.setFormat(format);
}
