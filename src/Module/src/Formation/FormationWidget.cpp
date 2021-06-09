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
#include <QTableWidget>
#include <QPalette>
#include <QColor>

#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/AircraftInfo.h"
#include "../../../SkyConnect/src/SkyManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../Persistence/src/Service/FlightService.h"
#include "../../../Persistence/src/Service/AircraftService.h"
#include "../AbstractModuleWidget.h"
#include "FormationWidget.h"
#include "ui_FormationWidget.h"

namespace
{
    constexpr int MinimumTableWidth = 600;
    constexpr int InvalidSelection = -1;
    constexpr int InvalidColumn = -1;
}

class FormationWidgetPrivate
{
public:
    FormationWidgetPrivate() noexcept
        : moduleAction(nullptr),
          aircraftService(std::make_unique<AircraftService>())
    {}

    QMetaObject::Connection aircraftIdAssignedConnection;
    QMetaObject::Connection aircraftInfoChangedConnection;
    std::unique_ptr<QAction> moduleAction;
    std::unique_ptr<AircraftService> aircraftService;
};

// PUBLIC

FormationWidget::FormationWidget(FlightService &flightService, QWidget *parent) noexcept
    : AbstractModuleWidget(flightService, parent),
      ui(std::make_unique<Ui::FormationWidget>()),
      d(std::make_unique<FormationWidgetPrivate>())
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    handleUserAircraftChanged(flight.getUserAircraft());
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

    Flight &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::userAircraftChanged,
            this, &FormationWidget::handleUserAircraftChanged);
}

void FormationWidget::hideEvent(QHideEvent *event) noexcept
{
    AbstractModuleWidget::hideEvent(event);

    Flight &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::userAircraftChanged,
               this, &FormationWidget::handleUserAircraftChanged);
}

void FormationWidget::updateUi() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();

    std::vector<std::unique_ptr<Aircraft>> &aircrafts = flight.getAircrafts();
    ui->aircraftTableWidget->blockSignals(true);
    ui->aircraftTableWidget->setSortingEnabled(false);
    ui->aircraftTableWidget->clearContents();
    ui->aircraftTableWidget->setRowCount(aircrafts.size());
    int rowIndex = 0;
    for (const auto &aircraft : aircrafts) {

        const AircraftInfo &aircraftInfo = aircraft->getAircraftInfoConst();

        QBrush backgroundColor = QGuiApplication::palette().mid();
        int columnIndex = 0;

        // ID
        QTableWidgetItem *newItem = new QTableWidgetItem();
        if (aircraftInfo.aircraftId != Aircraft::InvalidId) {
            newItem->setData(Qt::DisplayRole, aircraftInfo.aircraftId);
        } else if (SkyManager::getInstance().getCurrentSkyConnect().isRecording()) {
            backgroundColor = Qt::red;
        }
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Sequence
        newItem = new QTableWidgetItem();
        // Sequence numbers start at 1
        newItem->setData(Qt::DisplayRole, rowIndex + 1);
        newItem->setBackground(backgroundColor);
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Type
        newItem = new QTableWidgetItem(aircraftInfo.type);
        newItem->setBackground(backgroundColor);
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        ++rowIndex;
    }
    // Don't show internal aircraft IDs
    ui->aircraftTableWidget->hideColumn(0);
    ui->aircraftTableWidget->setSortingEnabled(true);
    ui->aircraftTableWidget->resizeColumnsToContents();
    ui->aircraftTableWidget->blockSignals(false);

    updateEditUi();
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
    d->moduleAction->setCheckable(true);

    ui->aircraftTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    const QStringList headers {tr("ID"), tr("Sequence"), tr("Type")};
    ui->aircraftTableWidget->setColumnCount(headers.count());
    ui->aircraftTableWidget->setHorizontalHeaderLabels(headers);
    ui->aircraftTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->aircraftTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->aircraftTableWidget->verticalHeader()->hide();
    ui->aircraftTableWidget->setMinimumWidth(MinimumTableWidth);
    ui->aircraftTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->aircraftTableWidget->sortByColumn(1, Qt::SortOrder::AscendingOrder);
}

void FormationWidget::updateEditUi() noexcept
{

}

void FormationWidget::frenchConnection() noexcept
{

}

const QString FormationWidget::getName()
{
    return QString(QT_TRANSLATE_NOOP("LogbookWidget", "Formation"));
}

// PRIVATE SLOTS

void FormationWidget::handleUserAircraftChanged(Aircraft &aircraft) noexcept
{
    QObject::disconnect(d->aircraftIdAssignedConnection);
    QObject::disconnect(d->aircraftIdAssignedConnection);
    d->aircraftInfoChangedConnection = connect(&aircraft, &Aircraft::idAssigned,
                                               this, &FormationWidget::updateUi);
    d->aircraftInfoChangedConnection = connect(&aircraft, &Aircraft::infoChanged,
                                               this, &FormationWidget::updateUi);
}

void FormationWidget::handleAircraftIdAssigned(qint64 id) noexcept
{
    int row = 0;
    QTableWidgetItem *item = nullptr;
    while (row < ui->aircraftTableWidget->rowCount() && item == nullptr) {
        item = ui->aircraftTableWidget->item(row, 0);
        bool ok;
        item->data(Qt::DisplayRole).toLongLong(&ok);
        if (ok) {
            item = nullptr;
            ++row;
        }
    }
    if (item != nullptr) {
        item->setData(Qt::DisplayRole, id);
    }
}
