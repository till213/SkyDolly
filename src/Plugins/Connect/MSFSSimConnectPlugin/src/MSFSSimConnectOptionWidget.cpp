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

#include <QCoreApplication>
#include <QString>
#include <QStringBuilder>
#include <QComboBox>
#include <QPushButton>
#include <QComboBox>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/File.h>
#include <Kernel/Version.h>
#include <PluginManager/Connect/ConnectPluginBaseSettings.h>
#include <PluginManager/SkyConnectManager.h>
#include "MSFSSimConnectSettings.h"
#include "MSFSSimConnectOptionWidget.h"
#include "ui_MSFSSimConnectOptionWidget.h"

struct MSFSSimConnectOptionWidgetPrivate
{
    MSFSSimConnectOptionWidgetPrivate(MSFSSimConnectSettings &pluginSettings)
        : pluginSettings {pluginSettings}
    {}

    MSFSSimConnectSettings &pluginSettings;
};

// PUBLIC

MSFSSimConnectOptionWidget::MSFSSimConnectOptionWidget(MSFSSimConnectSettings &pluginSettings, QWidget *parent)
    : OptionWidgetIntf {parent},
      ui {std::make_unique<Ui::MSFSSimConnectOptionWidget>()},
      d {std::make_unique<MSFSSimConnectOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

MSFSSimConnectOptionWidget::~MSFSSimConnectOptionWidget()
{
#ifdef DEBUG
    qDebug() << "MSFSSimConnectOptionWidget::~MSFSSimConnectOptionWidget: DELETED";
#endif
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
    const auto &skyConnectManager = SkyConnectManager::getInstance();
    // TODO FIXME: The option widget "survives" the plugin settings for a short moment, when the plugin is unloaded
    //             the connection then tries to access the settings that have already been deleted
    //             -> delete this option widget together with the plugin unloading, or get at least notified when the settings are deleted (disconnect this signal then)
    // connect(&skyConnectManager, &SkyConnectManager::stateChanged,
    //         this, &MSFSSimConnectOptionWidget::updateUi);

    connect(&d->pluginSettings, &MSFSSimConnectSettings::changed,
            this, &MSFSSimConnectOptionWidget::updateUi);
    connect(ui->restoreDefaultsPushButton, &QPushButton::clicked,
            this, &MSFSSimConnectOptionWidget::restoreDefaults);
    connect(ui->connectionComboBox, &QComboBox::currentIndexChanged,
            this, &MSFSSimConnectOptionWidget::updateInfoText);    
}

void MSFSSimConnectOptionWidget::initUi() noexcept
{
    ui->connectionComboBox->addItem(tr("Local (pipe)"));
    ui->connectionComboBox->addItem(tr("Network 1 (IPv4)"));
    ui->connectionComboBox->addItem(tr("Network 2 (IPv6)"));

    ui->infoLabel->setTextFormat(Qt::RichText);
    ui->infoLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->infoLabel->setOpenExternalLinks(true);
}

// PRIVATE SLOTS

void MSFSSimConnectOptionWidget::updateUi() noexcept
{
    using enum MSFSSimConnectSettings::ConnectionType;

    const bool simConnectConfiguration = File::hasSimConnectConfiguration();
    if (!simConnectConfiguration) {
        d->pluginSettings.setConnectionType(Pipe);
    }
    switch (d->pluginSettings.getConnectionType())
    {
    case Pipe:
        ui->connectionComboBox->setCurrentIndex(0);
        break;
    case IPv4:
        ui->connectionComboBox->setCurrentIndex(1);
        break;
    case IPv6:
        ui->connectionComboBox->setCurrentIndex(2);
        break;
    default:
        break;
    }
    const bool enabled = simConnectConfiguration && !SkyConnectManager::getInstance().isActive();
    ui->connectionComboBox->setEnabled(enabled);

    updateInfoText();
}

void MSFSSimConnectOptionWidget::updateInfoText() noexcept
{
    const bool hasConfiguration = File::hasSimConnectConfiguration();
    const bool isNetwork = ui->connectionComboBox->currentIndex() != 0;
    const QString url = QStringLiteral("file:///") % QCoreApplication::applicationDirPath() % "/SimConnect.cfg";
    const QString link = QStringLiteral("<a href=\"") % url % "\">SimConnect.cfg</a>";
    const QString infoText = hasConfiguration ? isNetwork ?
                                                    tr("Also refer to the %1 configuration file, located in the %2 application directory.").arg(link, Version::getApplicationName()) :
                                                    tr("This is the preferred connection type when running %1 on the same local machine as MSFS.").arg(Version::getApplicationName()) :
                                 tr("No SimConnect.cfg present in the %1 application directory: using local connection.").arg(Version::getApplicationName());
    ui->infoLabel->setText(infoText);
}

void MSFSSimConnectOptionWidget::restoreDefaults() noexcept
{
    d->pluginSettings.restoreDefaults();
}
