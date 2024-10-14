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
#include <QLineEdit>
#include <QString>

#include "IgcExportOptionWidget.h"
#include "IgcExportSettings.h"
#include "ui_IgcExportOptionWidget.h"

struct IgcExportOptionWidgetPrivate
{
    IgcExportOptionWidgetPrivate(IgcExportSettings &pluginSettings) noexcept
        : pluginSettings {pluginSettings}
    {}

    IgcExportSettings &pluginSettings;
};

// PUBLIC

IgcExportOptionWidget::IgcExportOptionWidget(IgcExportSettings &pluginSettings, QWidget *parent) noexcept
    : QWidget {parent},
      ui {std::make_unique<Ui::IgcExportOptionWidget>()},
      d {std::make_unique<IgcExportOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

IgcExportOptionWidget::~IgcExportOptionWidget() = default;

// PRIVATE

void IgcExportOptionWidget::frenchConnection() noexcept
{
    connect(&d->pluginSettings, &IgcExportSettings::changed,
            this, &IgcExportOptionWidget::updateUi);
    connect(ui->pilotNameLineEdit, &QLineEdit::textChanged,
            this, &IgcExportOptionWidget::onPilotNameChanged);
    connect(ui->coPilotNameLineEdit, &QLineEdit::textChanged,
            this, &IgcExportOptionWidget::onCoPilotNameChanged);
}

void IgcExportOptionWidget::initUi() noexcept
{}

// PRIVATE SLOTS

void IgcExportOptionWidget::updateUi() noexcept
{
    ui->pilotNameLineEdit->setText(d->pluginSettings.getPilotName());
    ui->coPilotNameLineEdit->setText(d->pluginSettings.getCoPilotName());
}

void IgcExportOptionWidget::onPilotNameChanged(const QString &name) noexcept
{
    d->pluginSettings.setPilotName(name);
}

void IgcExportOptionWidget::onCoPilotNameChanged(const QString &name) noexcept
{
    d->pluginSettings.setCoPilotName(name);
}
