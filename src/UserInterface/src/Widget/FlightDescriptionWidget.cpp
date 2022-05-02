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
#include <memory>

#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextCursor>

#include <Kernel/Unit.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include "../../../Persistence/src/Service/FlightService.h"
#include <Widget/FocusPlainTextEdit.h>
#include "FlightDescriptionWidget.h"
#include "ui_FlightDescriptionWidget.h"

class FlightDescriptionWidgetPrivate
{
public:
    FlightDescriptionWidgetPrivate(FlightService &theFlightService) noexcept
        : flightService(theFlightService)
    {}

    ~FlightDescriptionWidgetPrivate() noexcept
    {}

    FlightService &flightService;
    Unit unit;
};

// PUBLIC

FlightDescriptionWidget::FlightDescriptionWidget(FlightService &flightService, QWidget *parent) :
    QWidget(parent),
    d(std::make_unique<FlightDescriptionWidgetPrivate>(flightService)),
    ui(std::make_unique<Ui::FlightDescriptionWidget>())
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

FlightDescriptionWidget::~FlightDescriptionWidget()
{}

// PROTECTED

void FlightDescriptionWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);
    updateUi();

    Logbook &logbook = Logbook::getInstance();
    connect(&logbook.getCurrentFlight(), &Flight::flightChanged,
            this, &FlightDescriptionWidget::updateUi);
    connect(&logbook, &Logbook::flightStored,
            this, &FlightDescriptionWidget::updateUi);
}

void FlightDescriptionWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);

    Logbook &logbook = Logbook::getInstance();
    disconnect(&logbook.getCurrentFlight(), &Flight::flightChanged,
               this, &FlightDescriptionWidget::updateUi);
    disconnect(&logbook, &Logbook::flightStored,
               this, &FlightDescriptionWidget::updateUi);
}

// PRIVATE

void FlightDescriptionWidget::initUi() noexcept
{}

void FlightDescriptionWidget::frenchConnection() noexcept
{
    connect(ui->titleLineEdit, &QLineEdit::editingFinished,
            this, &FlightDescriptionWidget::onTitleEdited);
    connect(ui->focusPlainTextEdit, &FocusPlainTextEdit::focusLost,
            this, &FlightDescriptionWidget::onDescriptionEdited);
}

// PRIVATE SLOTS

void FlightDescriptionWidget::updateUi() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();

    bool enabled = flight.getId() != Flight::InvalidId;
    ui->titleLineEdit->blockSignals(true);
    ui->focusPlainTextEdit->blockSignals(true);
    ui->titleLineEdit->setText(flight.getTitle());
    ui->titleLineEdit->setEnabled(enabled);
    ui->focusPlainTextEdit->setPlainText(flight.getDescription());
    ui->focusPlainTextEdit->moveCursor(QTextCursor::MoveOperation::End);
    ui->focusPlainTextEdit->setEnabled(enabled);
    ui->titleLineEdit->blockSignals(false);
    ui->focusPlainTextEdit->blockSignals(false);

    ui->recordingTimeLineEdit->setText(d->unit.formatDateTime(flight.getCreationTime()));
}

void FlightDescriptionWidget::onTitleEdited() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    d->flightService.updateTitleAndDescription(flight, ui->titleLineEdit->text(), ui->focusPlainTextEdit->toPlainText());
}

void FlightDescriptionWidget::onDescriptionEdited() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    d->flightService.updateTitleAndDescription(flight, ui->titleLineEdit->text(), ui->focusPlainTextEdit->toPlainText());
}
