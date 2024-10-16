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
#include <QWidget>
#include <QComboBox>

#include <Kernel/Enum.h>
#include "KmlImportOptionWidget.h"
#include "KmlImportSettings.h"
#include "ui_KmlImportOptionWidget.h"

struct KmlImportOptionWidgetPrivate
{
    KmlImportOptionWidgetPrivate(KmlImportSettings &pluginSettings) noexcept
        : pluginSettings {pluginSettings}
    {}

    KmlImportSettings &pluginSettings;
};

// PUBLIC

KmlImportOptionWidget::KmlImportOptionWidget(KmlImportSettings &pluginSettings, QWidget *parent) noexcept
   : QWidget {parent},
     ui {std::make_unique<Ui::KmlImportOptionWidget>()},
     d {std::make_unique<KmlImportOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

KmlImportOptionWidget::~KmlImportOptionWidget() = default;

// PRIVATE

void KmlImportOptionWidget::frenchConnection() noexcept
{
    connect(ui->formatComboBox, &QComboBox::currentIndexChanged,
            this, &KmlImportOptionWidget::onFormatChanged);
    connect(&d->pluginSettings, &KmlImportSettings::changed,
            this, &KmlImportOptionWidget::updateUi);
}

void KmlImportOptionWidget::initUi() noexcept
{
    ui->formatComboBox->addItem("FlightAware", Enum::underly(KmlImportSettings::Format::FlightAware));
    ui->formatComboBox->addItem("FlightRadar24", Enum::underly(KmlImportSettings::Format::FlightRadar24));
    ui->formatComboBox->addItem(tr("Generic KML with track data"), Enum::underly(KmlImportSettings::Format::Generic));
}

// PRIVATE SLOTS

void KmlImportOptionWidget::updateUi() noexcept
{
    const KmlImportSettings::Format format = d->pluginSettings.getFormat();
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
    d->pluginSettings.setFormat(format);
}
