/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#include <QString>

#include <Kernel/Enum.h>
#include "CsvExportOptionWidget.h"
#include "CsvExportSettings.h"
#include "ui_CsvExportOptionWidget.h"

struct CsvExportOptionWidgetPrivate
{
    CsvExportOptionWidgetPrivate(CsvExportSettings &pluginSettings) noexcept
        : pluginSettings {pluginSettings}
    {}

    CsvExportSettings &pluginSettings;
};

// PUBLIC

CsvExportOptionWidget::CsvExportOptionWidget(CsvExportSettings &pluginSettings, QWidget *parent) noexcept
    : QWidget {parent},
      ui {std::make_unique<Ui::CsvExportOptionWidget>()},
      d {std::make_unique<CsvExportOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

CsvExportOptionWidget::~CsvExportOptionWidget() = default;

// PRIVATE

void CsvExportOptionWidget::frenchConnection() noexcept
{
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &CsvExportOptionWidget::onFormatChanged);
    connect(&d->pluginSettings, &CsvExportSettings::changed,
            this, &CsvExportOptionWidget::updateUi);
}

void CsvExportOptionWidget::initUi() noexcept
{
    ui->formatComboBox->addItem("Flightradar24", Enum::underly(CsvExportSettings::Format::Flightradar24));
    ui->formatComboBox->addItem("Position and attitude", Enum::underly(CsvExportSettings::Format::PositionAndAttitude));
}

// PRIVATE SLOTS

void CsvExportOptionWidget::updateUi() noexcept
{
    const CsvExportSettings::Format format = d->pluginSettings.getFormat();
    int currentIndex = 0;
    while (currentIndex < ui->formatComboBox->count() &&
           static_cast<CsvExportSettings::Format>(ui->formatComboBox->itemData(currentIndex).toInt()) != format) {
        ++currentIndex;
    }
    ui->formatComboBox->setCurrentIndex(currentIndex);
}

void CsvExportOptionWidget::onFormatChanged() noexcept
{
    const CsvExportSettings::Format format = static_cast<CsvExportSettings::Format>(ui->formatComboBox->currentData().toInt());
    d->pluginSettings.setFormat(format);
}
