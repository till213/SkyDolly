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
        : importSettings(theImportSettings)
    {}

     CSVImportSettings &importSettings;
};

// PUBLIC

CSVImportOptionWidget::CSVImportOptionWidget(CSVImportSettings &importSettings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(new Ui::CSVImportOptionWidget),
      d(std::make_unique<CSVImportOptionWidgetPrivate>(importSettings))
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
    delete ui;
#ifdef DEBUG
    qDebug("CSVImportOptionWidget::~CSVImportOptionWidget: DELETED");
#endif
}

// PRIVATE

void CSVImportOptionWidget::frenchConnection() noexcept
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->formatComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CSVImportOptionWidget::onFormatComboBoxCurrentIndexChanged);
#else
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &CSVImportOptionWidget::onFormatComboBoxCurrentIndexChanged);
#endif
    connect(&d->importSettings, &CSVImportSettings::defaultsRestored,
            this, &CSVImportOptionWidget::updateUi);
}

void CSVImportOptionWidget::initUi() noexcept
{
    initOptionUi();
}

void CSVImportOptionWidget::initOptionUi() noexcept
{
    ui->formatComboBox->addItem(Version::getApplicationName(), Enum::toUnderlyingType(CSVImportSettings::Format::SkyDolly));
}

void CSVImportOptionWidget::updateOptionUi() noexcept
{
    int currentIndex = 0;
    while (currentIndex < ui->formatComboBox->count() &&
           static_cast<CSVImportSettings::Format>(ui->formatComboBox->itemData(currentIndex).toInt()) != d->importSettings.m_format) {
        ++currentIndex;
    }
    ui->formatComboBox->setCurrentIndex(currentIndex);
}

// PRIVATE SLOTS

void CSVImportOptionWidget::onFormatComboBoxCurrentIndexChanged([[maybe_unused]]int index) noexcept
{
    d->importSettings.m_format = static_cast<CSVImportSettings::Format>(ui->formatComboBox->currentData().toInt());
}

void CSVImportOptionWidget::updateUi() noexcept
{
    updateOptionUi();
}
