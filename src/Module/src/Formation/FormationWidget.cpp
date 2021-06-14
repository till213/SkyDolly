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
#include <QIcon>

#include "../../../Kernel/src/Unit.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/AircraftInfo.h"
#include "../../../Persistence/src/Service/FlightService.h"
#include "../../../Persistence/src/Service/AircraftService.h"
#include "../../../SkyConnect/src/SkyManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../AbstractModuleWidget.h"
#include "FormationWidget.h"
#include "ui_FormationWidget.h"

namespace
{
    constexpr int MinimumTableWidth = 600;
    constexpr int InvalidSelection = -1;
    constexpr int SequenceNumberColumn = 0;
}

class FormationWidgetPrivate
{
public:
    FormationWidgetPrivate() noexcept
        : moduleAction(nullptr),
          aircraftService(std::make_unique<AircraftService>()),
          selectedRow(InvalidSelection),
          selectedAircraftIndex(Flight::InvalidId)
    {}

    QMetaObject::Connection aircraftInfoChangedConnection;
    std::unique_ptr<QAction> moduleAction;
    std::unique_ptr<AircraftService> aircraftService;
    int selectedRow;
    qint64 selectedAircraftIndex;
    Unit unit;
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
    connect(&flight, &Flight::aircraftRemoved,
            this, &FormationWidget::updateUi);
    SkyConnectIntf &skyConnect = SkyManager::getInstance().getCurrentSkyConnect();
    connect(&skyConnect, &SkyConnectIntf::stateChanged,
            this, &FormationWidget::updateUi);

    // Also updates the UI
    handleUserAircraftChanged(flight.getUserAircraft());
    handleSelectionChanged();    
}

void FormationWidget::hideEvent(QHideEvent *event) noexcept
{
    AbstractModuleWidget::hideEvent(event);

    Flight &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::userAircraftChanged,
               this, &FormationWidget::handleUserAircraftChanged);
    disconnect(&flight, &Flight::aircraftRemoved,
               this, &FormationWidget::updateUi);
    SkyConnectIntf &skyConnect = SkyManager::getInstance().getCurrentSkyConnect();
    disconnect(&skyConnect, &SkyConnectIntf::stateChanged,
               this, &FormationWidget::updateUi);
    QObject::disconnect(d->aircraftInfoChangedConnection);
}

// PROTECTED SLOTS

void FormationWidget::handleRecordingStopped() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    int count = flight.getAircraftCount();
    if (count > 1) {
        // Sequence starts at 1
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

    const QStringList headers {tr("Sequence"), tr("Type"), tr("Category"), tr("Wing Span"), tr("Initial Airspeed"), tr("Initial Altitude Above Ground")};
    ui->aircraftTableWidget->setColumnCount(headers.count());
    ui->aircraftTableWidget->setHorizontalHeaderLabels(headers);
    ui->aircraftTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->aircraftTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->aircraftTableWidget->verticalHeader()->hide();
    ui->aircraftTableWidget->setMinimumWidth(MinimumTableWidth);
    ui->aircraftTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->aircraftTableWidget->sortByColumn(SequenceNumberColumn, Qt::SortOrder::AscendingOrder);
}

void FormationWidget::frenchConnection() noexcept
{
    connect(ui->aircraftTableWidget, &QTableWidget::itemSelectionChanged,
            this, &FormationWidget::handleSelectionChanged);
    connect(ui->aircraftTableWidget, &QTableWidget::cellDoubleClicked,
            this, &FormationWidget::handleCellSelected);
    connect(ui->userAircraftPushButton, &QPushButton::clicked,
            this, &FormationWidget::updateUserAircraftIndex);
    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &FormationWidget::deleteAircraft);
}

const QString FormationWidget::getName()
{
    return QString(QT_TRANSLATE_NOOP("LogbookWidget", "Formation"));
}

// PRIVATE SLOTS

void FormationWidget::updateUi() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();

    std::vector<std::unique_ptr<Aircraft>> &aircrafts = flight.getAircrafts();
    ui->aircraftTableWidget->blockSignals(true);
    ui->aircraftTableWidget->setSortingEnabled(false);
    ui->aircraftTableWidget->clearContents();
    ui->aircraftTableWidget->setRowCount(aircrafts.size());
    int rowIndex = 0;
    const int userAircraftIndex = flight.getUserAircraftIndex();
    const bool recording = SkyManager::getInstance().getCurrentSkyConnect().isRecording();
    const QString tooltip = tr("Double-click to change user aircraft");
    for (const auto &aircraft : aircrafts) {

        const AircraftInfo &aircraftInfo = aircraft->getAircraftInfoConst();
        int columnIndex = 0;

        // Sequence
        QTableWidgetItem *newItem = new QTableWidgetItem();
        if (rowIndex == userAircraftIndex) {
            QIcon icon;
            if (recording) {
                icon = QIcon(":/img/icons/aircraft-record-normal.png");
            } else {
                icon = QIcon(":/img/icons/aircraft-normal.png");
            }
            newItem->setIcon(icon);
        }
        // Sequence numbers start at 1
        newItem->setData(Qt::DisplayRole, rowIndex + 1);
        newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        newItem->setToolTip(tooltip);
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Type
        newItem = new QTableWidgetItem(aircraftInfo.type);
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Category
        newItem = new QTableWidgetItem(aircraftInfo.category);
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Wing span
        newItem = new QTableWidgetItem(d->unit.formatFeet(aircraftInfo.wingSpan));
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ++columnIndex;

        // Initial airspeed
        newItem = new QTableWidgetItem(d->unit.formatKnots(aircraftInfo.initialAirspeed));
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ++columnIndex;

        // Initial altitude above ground
        newItem = new QTableWidgetItem(d->unit.formatFeet(aircraftInfo.altitudeAboveGround));
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ++columnIndex;

        ++rowIndex;
    }

    ui->aircraftTableWidget->setSortingEnabled(true);
    ui->aircraftTableWidget->resizeColumnsToContents();
    ui->aircraftTableWidget->blockSignals(false);

    updateEditUi();
}

void FormationWidget::updateEditUi() noexcept
{
    const bool active = SkyManager::getInstance().getCurrentSkyConnect().isActive();
    const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
    bool userAircraftIndex = d->selectedAircraftIndex == flight.getUserAircraftIndex();
    ui->userAircraftPushButton->setEnabled(!active &&  d->selectedAircraftIndex != Flight::InvalidId && !userAircraftIndex);
    const bool formation = flight.getAircraftCount() > 1;
    ui->deletePushButton->setEnabled(!active && d->selectedAircraftIndex != Flight::InvalidId && formation);
}

void FormationWidget::handleUserAircraftChanged(Aircraft &aircraft) noexcept
{
    QObject::disconnect(d->aircraftInfoChangedConnection);
    d->aircraftInfoChangedConnection = connect(&aircraft, &Aircraft::infoChanged,
                                               this, &FormationWidget::updateUi);
    updateUi();
}

void FormationWidget::handleCellSelected(int row, int column) noexcept
{
    Q_UNUSED(column)
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    getFlightService().updateUserAircraftIndex(flight, row);
}

void FormationWidget::handleSelectionChanged() noexcept
{
    QItemSelectionModel *select = ui->aircraftTableWidget->selectionModel();
    QModelIndexList modelIndices = select->selectedRows(SequenceNumberColumn);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        d->selectedRow = modelIndex.row();
        // Index starts at 0
        d->selectedAircraftIndex = ui->aircraftTableWidget->model()->data(modelIndex).toInt() - 1;
    } else {
        d->selectedRow = InvalidSelection;
        d->selectedAircraftIndex = Flight::InvalidId;
    }
    updateEditUi();
}

void FormationWidget::updateUserAircraftIndex() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    getFlightService().updateUserAircraftIndex(flight, d->selectedRow);
}

void FormationWidget::deleteAircraft() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    d->aircraftService->removeByIndex(flight, d->selectedRow);
}
