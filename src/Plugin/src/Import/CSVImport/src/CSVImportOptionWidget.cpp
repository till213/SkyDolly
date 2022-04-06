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
#include "../../../../../Kernel/src/Version.h"
#include "CSVImportOptionWidget.h"
#include "CSVImportSettings.h"
#include "ui_CSVImportOptionWidget.h"

class CSVImportOptionWidgetPrivate
{
public:
    CSVImportOptionWidgetPrivate(CSVImportSettings &theImportSettings) noexcept
        : settings(theImportSettings)
    {}

     CSVImportSettings &settings;
};

// PUBLIC

CSVImportOptionWidget::CSVImportOptionWidget(CSVImportSettings &settings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::CSVImportOptionWidget>()),
      d(std::make_unique<CSVImportOptionWidgetPrivate>(settings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("CSVImportOptionWidget::CSVImportOptionWidget: CREATED");
#endif
}

CSVImportOptionWidget::~CSVImportOptionWidget() noexcept
{
#ifdef DEBUG
    qDebug("CSVImportOptionWidget::~CSVImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void CSVImportOptionWidget::frenchConnection() noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CSVImportOptionWidget::onFormatChanged);
#else
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &CSVImportOptionWidget::onFormatChanged);
#endif
    connect(&d->settings, &CSVImportSettings::extendedSettingsChanged,
            this, &CSVImportOptionWidget::updateUi);
}

void CSVImportOptionWidget::initUi() noexcept
{
    ui->formatComboBox->addItem(Version::getApplicationName(), Enum::toUnderlyingType(CSVImportSettings::Format::SkyDolly));
    ui->formatComboBox->addItem("FlightRadar24", Enum::toUnderlyingType(CSVImportSettings::Format::FlightRadar24));
    ui->formatComboBox->addItem("Flight Recorder", Enum::toUnderlyingType(CSVImportSettings::Format::FlightRecorder));
}

// PRIVATE SLOTS

void CSVImportOptionWidget::onFormatChanged([[maybe_unused]]int index) noexcept
{
    const CSVImportSettings::Format format = static_cast<CSVImportSettings::Format>(ui->formatComboBox->currentData().toInt());
    d->settings.setFormat(format);
}

void CSVImportOptionWidget::updateUi() noexcept
{
    const CSVImportSettings::Format format = d->settings.getFormat();
    int currentIndex = 0;
    while (currentIndex < ui->formatComboBox->count() &&
           static_cast<CSVImportSettings::Format>(ui->formatComboBox->itemData(currentIndex).toInt()) != format) {
        ++currentIndex;
    }
    ui->formatComboBox->setCurrentIndex(currentIndex);
}
