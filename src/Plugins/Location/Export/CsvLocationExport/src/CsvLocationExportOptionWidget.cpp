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
#include <QString>

#include <Kernel/Enum.h>
#include "CsvLocationExportOptionWidget.h"
#include "CsvLocationExportSettings.h"
#include "ui_CsvLocationExportOptionWidget.h"

struct CsvLocationExportOptionWidgetPrivate
{
    CsvLocationExportOptionWidgetPrivate(CsvLocationExportSettings &pluginSettings) noexcept
        : pluginSettings(pluginSettings)
    {}

    CsvLocationExportSettings &pluginSettings;
};

// PUBLIC

CsvLocationExportOptionWidget::CsvLocationExportOptionWidget(CsvLocationExportSettings &pluginSettings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::CsvLocationExportOptionWidget>()),
      d(std::make_unique<CsvLocationExportOptionWidgetPrivate>(pluginSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

CsvLocationExportOptionWidget::~CsvLocationExportOptionWidget() = default;

// PRIVATE

void CsvLocationExportOptionWidget::frenchConnection() noexcept
{
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &CsvLocationExportOptionWidget::onFormatChanged);
    connect(&d->pluginSettings, &CsvLocationExportSettings::changed,
            this, &CsvLocationExportOptionWidget::updateUi);
}

void CsvLocationExportOptionWidget::initUi() noexcept
{
    ui->formatComboBox->addItem(QStringLiteral("Sky Dolly"), Enum::underly(CsvLocationExportSettings::Format::SkyDolly));
    ui->formatComboBox->addItem(QStringLiteral("Little Navmap"), Enum::underly(CsvLocationExportSettings::Format::LittleNavmap));
}

// PRIVATE SLOTS

void CsvLocationExportOptionWidget::updateUi() noexcept
{
    const CsvLocationExportSettings::Format format = d->pluginSettings.getFormat();
    int currentIndex = 0;
    while (currentIndex < ui->formatComboBox->count() &&
           static_cast<CsvLocationExportSettings::Format>(ui->formatComboBox->itemData(currentIndex).toInt()) != format) {
        ++currentIndex;
    }
    ui->formatComboBox->setCurrentIndex(currentIndex);
}

void CsvLocationExportOptionWidget::onFormatChanged() noexcept
{
    const CsvLocationExportSettings::Format format = static_cast<CsvLocationExportSettings::Format>(ui->formatComboBox->currentData().toInt());
    d->pluginSettings.setFormat(format);
}
