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
#include "KMLExportOptionWidget.h"
#include "KMLExportSettings.h"
#include "ui_KMLExportOptionWidget.h"

class KMLExportOptionWidgetPrivate
{
public:
    KMLExportOptionWidgetPrivate(KMLExportSettings &theExportSettings) noexcept
        : exportSettings(theExportSettings)
    {}

     KMLExportSettings &exportSettings;
};

// PUBLIC

KMLExportOptionWidget::KMLExportOptionWidget(KMLExportSettings &exportSettings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(new Ui::KMLExportOptionWidget),
      d(std::make_unique<KMLExportOptionWidgetPrivate>(exportSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug("KMLExportOptionWidget::KMLExportOptionWidget: CREATED");
#endif
}

KMLExportOptionWidget::~KMLExportOptionWidget() noexcept
{
    delete ui;
#ifdef DEBUG
    qDebug("KMLExportOptionWidget::~KMLExportOptionWidget: DELETED");
#endif
}

// PRIVATE

void KMLExportOptionWidget::frenchConnection() noexcept
{

    connect(&d->exportSettings, &KMLExportSettings::defaultsRestored,
            this, &KMLExportOptionWidget::updateUi);
}

void KMLExportOptionWidget::initUi() noexcept
{

}

// PRIVATE SLOTS

void KMLExportOptionWidget::onFormatChanged([[maybe_unused]]int index) noexcept
{

}

void KMLExportOptionWidget::updateUi() noexcept
{

}
