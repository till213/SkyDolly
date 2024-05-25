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

#include <QString>
#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextCursor>

#include <Kernel/Const.h>
#include <Kernel/Unit.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/SimVar.h>
#include <Persistence/Service/FlightService.h>
#include <PluginManager/SkyConnectManager.h>
#include <Widget/FocusPlainTextEdit.h>
#include "FlightDescriptionWidget.h"
#include "ui_FlightDescriptionWidget.h"

struct FlightDescriptionWidgetPrivate
{
    std::unique_ptr<FlightService> flightService {std::make_unique<FlightService>()};
    Unit unit;
};

// PUBLIC

FlightDescriptionWidget::FlightDescriptionWidget(QWidget *parent) :
    QWidget {parent},
    ui {std::make_unique<Ui::FlightDescriptionWidget>()},
    d {std::make_unique<FlightDescriptionWidgetPrivate>()}
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

FlightDescriptionWidget::~FlightDescriptionWidget() = default;

// PROTECTED

void FlightDescriptionWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);
    updateUi();

    // Connection
    auto &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &FlightDescriptionWidget::updateUi);

    // Flight
    const Logbook &logbook = Logbook::getInstance();
    const auto &flight = logbook.getCurrentFlight();
    connect(&flight, &Flight::cleared,
            this, &FlightDescriptionWidget::updateUi);
    connect(&flight, &Flight::titleChanged,
            this, &FlightDescriptionWidget::updateUi);
    connect(&flight, &Flight::flightNumberChanged,
            this, &FlightDescriptionWidget::updateUi);
    connect(&flight, &Flight::descriptionChanged,
            this, &FlightDescriptionWidget::updateUi);
    connect(&flight, &Flight::flightStored,
            this, &FlightDescriptionWidget::updateUi);
    connect(&flight, &Flight::flightRestored,
            this, &FlightDescriptionWidget::updateUi);
}

void FlightDescriptionWidget::hideEvent(QHideEvent *event) noexcept
{
    QWidget::hideEvent(event);

    // Connection
    auto &skyConnectManager = SkyConnectManager::getInstance();
    disconnect(&skyConnectManager, &SkyConnectManager::stateChanged,
               this, &FlightDescriptionWidget::updateUi);

    // Flight
    const Logbook &logbook = Logbook::getInstance();
    const auto &flight = logbook.getCurrentFlight();
    disconnect(&flight, &Flight::cleared,
               this, &FlightDescriptionWidget::updateUi);
    disconnect(&flight, &Flight::titleChanged,
               this, &FlightDescriptionWidget::updateUi);
    disconnect(&flight, &Flight::descriptionChanged,
               this, &FlightDescriptionWidget::updateUi);
    disconnect(&flight, &Flight::flightStored,
               this, &FlightDescriptionWidget::updateUi);
    disconnect(&flight, &Flight::flightRestored,
               this, &FlightDescriptionWidget::updateUi);
}

// PRIVATE

void FlightDescriptionWidget::initUi() noexcept
{
    ui->flightNumberLineEdit->setToolTip(QString::fromLatin1(SimVar::ATCId));
}

void FlightDescriptionWidget::frenchConnection() noexcept
{
    connect(ui->titleLineEdit, &QLineEdit::editingFinished,
            this, &FlightDescriptionWidget::onTitleEdited);
    connect(ui->flightNumberLineEdit, &QLineEdit::editingFinished,
            this, &FlightDescriptionWidget::onFlightNumberEdited);
    connect(ui->focusPlainTextEdit, &FocusPlainTextEdit::focusLost,
            this, &FlightDescriptionWidget::onDescriptionEdited);
}

// PRIVATE SLOTS

void FlightDescriptionWidget::updateUi() noexcept
{
    const auto &flight = Logbook::getInstance().getCurrentFlight();

    bool enabled = flight.getId() != Const::InvalidId;
    ui->titleLineEdit->blockSignals(true);
    ui->flightNumberLineEdit->blockSignals(true);
    ui->focusPlainTextEdit->blockSignals(true);

    ui->titleLineEdit->setText(flight.getTitle());
    ui->titleLineEdit->setEnabled(enabled);

    ui->flightNumberLineEdit->setText(flight.getFlightNumber());
    ui->flightNumberLineEdit->setEnabled(enabled);

    ui->focusPlainTextEdit->setPlainText(flight.getDescription());
    ui->focusPlainTextEdit->moveCursor(QTextCursor::MoveOperation::End);
    ui->focusPlainTextEdit->setEnabled(enabled);

    ui->titleLineEdit->blockSignals(false);
    ui->flightNumberLineEdit->blockSignals(false);
    ui->focusPlainTextEdit->blockSignals(false);

    ui->recordingTimeLineEdit->setText(d->unit.formatDateTime(flight.getCreationTime()));
    ui->recordingTimeLineEdit->setToolTip(flight.getCreationTime().toUTC().toString(Qt::ISODate));
}

void FlightDescriptionWidget::onTitleEdited() const noexcept
{
    auto &flight = Logbook::getInstance().getCurrentFlight();
    d->flightService->updateTitle(flight, ui->titleLineEdit->text());
}

void FlightDescriptionWidget::onFlightNumberEdited() const noexcept
{
    auto &flight = Logbook::getInstance().getCurrentFlight();
    d->flightService->updateFlightNumber(flight, ui->flightNumberLineEdit->text());
}

void FlightDescriptionWidget::onDescriptionEdited() const noexcept
{
    auto &flight = Logbook::getInstance().getCurrentFlight();
    d->flightService->updateDescription(flight, ui->focusPlainTextEdit->toPlainText());
}
