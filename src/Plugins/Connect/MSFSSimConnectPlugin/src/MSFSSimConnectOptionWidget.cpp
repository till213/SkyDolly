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
#include <memory>

#include <QComboBox>
#ifdef DEBUG
#include <QDebug>
#endif

#include <PluginManager/Connect/ConnectPluginBaseSettings.h>
#include "MSFSSimConnectSettings.h"
#include "MSFSSimConnectOptionWidget.h"
#include "ui_MSFSSimConnectOptionWidget.h"

struct MSFSSimConnectOptionWidgetPrivate
{
    MSFSSimConnectOptionWidgetPrivate(MSFSSimConnectSettings &pluginSettings)
        : pluginSettings(pluginSettings)
    {}

    MSFSSimConnectSettings &pluginSettings;
};

// PUBLIC

MSFSSimConnectOptionWidget::MSFSSimConnectOptionWidget(MSFSSimConnectSettings &pluginSettings, QWidget *parent)
    : OptionWidgetIntf(parent),
    ui(new Ui::MSFSSimConnectOptionWidget),
    d(std::make_unique<MSFSSimConnectOptionWidgetPrivate>(pluginSettings))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

MSFSSimConnectOptionWidget::~MSFSSimConnectOptionWidget()
{
    delete ui;
}

void MSFSSimConnectOptionWidget::accept() noexcept
{
    switch (ui->connectionComboBox->currentIndex())
    {
    case 0:
        d->pluginSettings.setConnectionType(MSFSSimConnectSettings::ConnectionType::Pipe);
        break;
    case 1:
        d->pluginSettings.setConnectionType(MSFSSimConnectSettings::ConnectionType::IPv4);
        break;
    case 2:
        d->pluginSettings.setConnectionType(MSFSSimConnectSettings::ConnectionType::IPv6);
        break;
    default:
        break;
    }
}

// PRIVATE

void MSFSSimConnectOptionWidget::frenchConnection() noexcept
{
    connect(&d->pluginSettings, &MSFSSimConnectSettings::changed,
            this, &MSFSSimConnectOptionWidget::updateUi);
}

void MSFSSimConnectOptionWidget::initUi() noexcept
{
    ui->connectionComboBox->addItem(tr("Local connection (pipe)"));
    ui->connectionComboBox->addItem(tr("Network 1 (IPv4)"));
    ui->connectionComboBox->addItem(tr("Network 2 (IPv6)"));
}

// PRIVATE SLOTS

void MSFSSimConnectOptionWidget::updateUi() noexcept
{
    switch (d->pluginSettings.getConnectionType())
    {
    case MSFSSimConnectSettings::ConnectionType::Pipe:
        ui->connectionComboBox->setCurrentIndex(0);
        break;
    case MSFSSimConnectSettings::ConnectionType::IPv4:
        ui->connectionComboBox->setCurrentIndex(1);
        break;
    case MSFSSimConnectSettings::ConnectionType::IPv6:
        ui->connectionComboBox->setCurrentIndex(2);
        break;
    default:
        break;
    }
}