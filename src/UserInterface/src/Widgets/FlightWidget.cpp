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

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVector>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QDateTime>
#include <QTime>
#include <QPushButton>
#include <QMessageBox>

#include "../../../Model/src/Flight.h"
#include "../../../Model/src/FlightDescription.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Persistence/src/Service/FlightService.h"
#include "../Unit.h"

#include "FlightWidget.h"
#include "ui_FlightWidget.h"

namespace
{
    constexpr int MinimumTableWidth = 600;
    constexpr int InvalidSelection = -1;
}

class FlightWidgetPrivate
{
public:
    FlightWidgetPrivate(FlightService &theFlightService)
        : flightService(theFlightService),
          selectedRow(InvalidSelection),
          selectedFlightId(Flight::InvalidId)

    {}

    FlightService &flightService;
    int selectedRow;
    qint64 selectedFlightId;
    Unit unit;
};

// PUBLIC

FlightWidget::FlightWidget(FlightService &flightService, QWidget *parent) noexcept
    : QWidget(parent),
      ui(new Ui::FlightWidget),
      d(std::make_unique<FlightWidgetPrivate>(flightService))
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

FlightWidget::~FlightWidget() noexcept
{
    delete ui;
}

qint64 FlightWidget::getSelectedFlightId() const noexcept
{
    return d->selectedFlightId;
}

// PROTECTED

void FlightWidget::showEvent(QShowEvent *event) noexcept
{
    Q_UNUSED(event)
    updateUi();

    // Service
    connect(&d->flightService, &FlightService::flightStored,
            this, &FlightWidget::updateUi);
}

void FlightWidget::hideEvent(QHideEvent *event) noexcept
{
    Q_UNUSED(event)

    disconnect(&d->flightService, &FlightService::flightStored,
               this, &FlightWidget::updateUi);
}

// PRIVATE

void FlightWidget::initUi() noexcept
{
    ui->flightTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    const QStringList headers {tr("Flight"), tr("Date"), tr("Aircraft"), tr("Departure Time"), tr("Departure"), tr("Arrival Time"), tr("Arrival"), tr("Total Time of Flight"), tr("Description")};
    ui->flightTableWidget->setColumnCount(headers.count());
    ui->flightTableWidget->setHorizontalHeaderLabels(headers);
    ui->flightTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->flightTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->flightTableWidget->setMinimumWidth(MinimumTableWidth);
}

void FlightWidget::updateEditUi() noexcept
{
    ui->loadPushButton->setEnabled(d->selectedFlightId != Flight::InvalidId);
    ui->deletePushButton->setEnabled(d->selectedFlightId != Flight::InvalidId);
}

void FlightWidget::frenchConnection() noexcept
{
    connect(ui->flightTableWidget, &QTableWidget::itemSelectionChanged,
            this, &FlightWidget::handleSelectionChanged);
    connect(ui->loadPushButton, &QPushButton::clicked,
            this, &FlightWidget::handleLoad);
    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &FlightWidget::handleDelete);
}

// PRIVATE SLOTS

void FlightWidget::updateUi() noexcept
{
    QVector<FlightDescription> descriptions = d->flightService.getFlightDescriptions();
    ui->flightTableWidget->setSortingEnabled(false);
    ui->flightTableWidget->clearContents();
    ui->flightTableWidget->setRowCount(descriptions.count());
    int rowIndex = 0;
    for (const FlightDescription &desc : descriptions) {
        QTableWidgetItem *newItem = new QTableWidgetItem();

        newItem->setData(Qt::DisplayRole, desc.id);
        ui->flightTableWidget->setItem(rowIndex, 0, newItem);

        newItem = new QTableWidgetItem(d->unit.formatDate(desc.creationDate));
        ui->flightTableWidget->setItem(rowIndex, 1, newItem);

        newItem = new QTableWidgetItem(desc.aircraftType);
        ui->flightTableWidget->setItem(rowIndex, 2, newItem);

        newItem = new QTableWidgetItem(d->unit.formatTime(desc.startDate));
        ui->flightTableWidget->setItem(rowIndex, 3, newItem);

        newItem = new QTableWidgetItem(desc.startLocation);
        ui->flightTableWidget->setItem(rowIndex, 4, newItem);

        newItem = new QTableWidgetItem(d->unit.formatTime(desc.endDate));
        ui->flightTableWidget->setItem(rowIndex, 5, newItem);

        newItem = new QTableWidgetItem(desc.endLocation);
        ui->flightTableWidget->setItem(rowIndex, 6, newItem);

        const qint64 durationMSec = desc.startDate.msecsTo(desc.endDate);
        const QTime time = QTime(0, 0).addMSecs(durationMSec);
        newItem = new QTableWidgetItem(d->unit.formatDuration(time));
        ui->flightTableWidget->setItem(rowIndex, 7, newItem);

        newItem = new QTableWidgetItem(desc.description);
        ui->flightTableWidget->setItem(rowIndex, 8, newItem);

        ++rowIndex;
    }
    ui->flightTableWidget->resizeColumnsToContents();
    ui->flightTableWidget->setSortingEnabled(true);

    updateEditUi();
}

void FlightWidget::handleSelectionChanged() noexcept
{
    QItemSelectionModel *select = ui->flightTableWidget->selectionModel();
    QModelIndexList selectedRows = select->selectedRows(0);
    if (selectedRows.count() > 0) {
        QModelIndex modelIndex = selectedRows.at(0);
        d->selectedRow = modelIndex.row();
        d->selectedFlightId = ui->flightTableWidget->model()->data(modelIndex).toLongLong();
    } else {
        d->selectedRow = InvalidSelection;
        d->selectedFlightId = Flight::InvalidId;
    }
    updateEditUi();
}

void FlightWidget::handleLoad() noexcept
{
    qint64 selectedFlightId = d->selectedFlightId;
    if (selectedFlightId != Flight::InvalidId) {
        bool ok = d->flightService.restore(selectedFlightId, Logbook::getInstance().getCurrentFlight());
        if (!ok) {
            QMessageBox::critical(this, tr("Database error"), tr("The flight %1 could not be read from the library.").arg(selectedFlightId));
        }
    }
}

void FlightWidget::handleDelete() noexcept
{
    if (d->selectedFlightId != Flight::InvalidId) {
        d->flightService.deleteById(d->selectedFlightId);
        int lastSelectedRow = d->selectedRow;
        updateUi();
        int selectedRow = qMin(lastSelectedRow, ui->flightTableWidget->rowCount() - 1);
        ui->flightTableWidget->selectRow(selectedRow);
    }
}
