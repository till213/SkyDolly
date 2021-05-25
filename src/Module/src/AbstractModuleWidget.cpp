/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <QWidget>
#include <QMessageBox>

#include "../../Kernel/src/Version.h"
#include "../../Kernel/src/Settings.h"
#include "../../Model/src/Logbook.h"
#include "../../SkyConnect/src/SkyManager.h"
#include "../../SkyConnect/src/SkyConnectIntf.h"
#include "../../Persistence/src/Service/FlightService.h"
#include "Module.h"
#include "AbstractModuleWidget.h"

class AbstractModuleWidgetPrivate
{
public:
    AbstractModuleWidgetPrivate(FlightService &theFlightService) noexcept
        : flightService(theFlightService)
    {}

    FlightService &flightService;
};

// PUBLIC

AbstractModuleWidget::AbstractModuleWidget(FlightService &flightService, QWidget *parent) noexcept
    : QWidget(parent),
      d(std::make_unique<AbstractModuleWidgetPrivate>(flightService))
{}

AbstractModuleWidget::~AbstractModuleWidget() noexcept
{}

QWidget &AbstractModuleWidget::getWidget() noexcept
{
    return *this;
}

// PROTECTED

FlightService &AbstractModuleWidget::getFlightService() const noexcept
{
    return d->flightService;
}

void AbstractModuleWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);
    updateUi();

    SkyConnectIntf &skyConnect = SkyManager::getInstance().getCurrentSkyConnect();
    connect(&skyConnect, &SkyConnectIntf::recordingStopped,
            this, &AbstractModuleWidget::handleRecordingStopped);
}

void AbstractModuleWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);

    SkyConnectIntf &skyConnect = SkyManager::getInstance().getCurrentSkyConnect();
    disconnect(&skyConnect, &SkyConnectIntf::recordingStopped,
               this, &AbstractModuleWidget::handleRecordingStopped);
}

// PROTECTED SLOTS

void AbstractModuleWidget::handleRecordingStopped() noexcept
{
    Settings &settings = Settings::getInstance();
    int previewInfoCount = settings.getPreviewInfoDialogCount();
    if (previewInfoCount > 0) {
        --previewInfoCount;
        QMessageBox::information(this, "Preview",
            QString("%1 %2 stores flights automatically into a database (the logbook). As new features are being added and developed the database format will change.\n\n"
                    "During the preview phase older databases will automatically be migrated to the current data format (as \"proof of concept\").\n\n"
                    "However take note that the first release version 1.0.0 will consolidate all migration steps into the final database format, making logbooks generated with preview "
                    "versions (such as this one) unreadable!\n\n"
                    "(From that point onwards databases (logbooks) will of course be migrated to the format of the next release version.)\n\n"
                    "This dialog will be shown %3 more times.").arg(Version::getApplicationName(), Version::getApplicationVersion()).arg(previewInfoCount),
            QMessageBox::StandardButton::Ok);
        settings.setPreviewInfoDialogCount(previewInfoCount);
    }
    d->flightService.store(Logbook::getInstance().getCurrentFlight());
}
