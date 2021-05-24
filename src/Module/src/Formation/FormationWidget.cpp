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
#include <memory>

#include <QWidget>
#include <QAction>

#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Persistence/src/Service/FlightService.h"
#include "../../../Persistence/src/Service/AircraftService.h"
#include "../AbstractModuleWidget.h"
#include "FormationWidget.h"
#include "ui_FormationWidget.h"

class FormationWidgetPrivate
{
public:
    FormationWidgetPrivate() noexcept
        : moduleAction(nullptr),
          aircraftService(std::make_unique<AircraftService>())
    {}

    std::unique_ptr<QAction> moduleAction;
    std::unique_ptr<AircraftService> aircraftService;
};

// PUBLIC

FormationWidget::FormationWidget(SkyConnectIntf &skyConnect, FlightService &flightService, QWidget *parent) noexcept
    : AbstractModuleWidget(skyConnect, flightService, parent),
      ui(std::make_unique<Ui::FormationWidget>()),
      d(std::make_unique<FormationWidgetPrivate>())
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

FormationWidget::~FormationWidget() noexcept
{
#ifdef DEBUG
    qDebug("FormationWidget::~FormationWidget: DELETED.");
#endif
}

Module::Module FormationWidget::getModuleId() const noexcept
{
    return Module::Module::Formation;
}

const QString FormationWidget::getModuleName() const noexcept
{
    return getName();
}

QAction &FormationWidget::getAction() noexcept
{
    return *d->moduleAction;
}

// PROTECTED

void FormationWidget::showEvent(QShowEvent *event) noexcept
{
    AbstractModuleWidget::showEvent(event);
    updateUi();
}

void FormationWidget::hideEvent(QHideEvent *event) noexcept
{
    AbstractModuleWidget::hideEvent(event);
}

void FormationWidget::updateUi() noexcept
{

}

// PROTECTED SLOTS

void FormationWidget::handleRecordingStopped() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    int count = flight.getAircraftCount();
    if (count > 1) {
        d->aircraftService->store(flight.getId(), count, *flight.getAircrafts().at(count - 1));
    } else {
        AbstractModuleWidget::handleRecordingStopped();
    }
}

// PRIVATE

void FormationWidget::initUi() noexcept
{
    d->moduleAction = std::make_unique<QAction>(getName());
}

void FormationWidget::frenchConnection() noexcept
{

}

const QString FormationWidget::getName()
{
    return QString(QT_TRANSLATE_NOOP("LogbookWidget", "Formation"));
}
