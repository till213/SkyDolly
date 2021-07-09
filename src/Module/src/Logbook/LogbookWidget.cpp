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
#include <QCheckBox>
#include <QAction>

#include "../../../Kernel/src/Version.h"
#include "../../../Kernel/src/Unit.h"
#include "../../../Kernel/src/Settings.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/FlightSummary.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Persistence/src/Service/DatabaseService.h"
#include "../../../Persistence/src/Service/LogbookService.h"
#include "../../../Persistence/src/Service/FlightService.h"
#include "../../../SkyConnect/src/SkyManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../AbstractModuleWidget.h"
#include "../Module.h"
#include "LogbookWidget.h"
#include "ui_LogbookWidget.h"

namespace
{
    constexpr int MinimumTableWidth = 600;
    constexpr int InvalidSelection = -1;
    constexpr int InvalidColumn = -1;
    constexpr int FlightIdColumn = 0;
}

class LogbookWidgetPrivate
{
public:
    LogbookWidgetPrivate(DatabaseService &theDatabaseService, FlightService &theFlightService) noexcept
        : titleColumnIndex(InvalidColumn),
          databaseService(theDatabaseService),
          flightService(theFlightService),
          logbookService(std::make_unique<LogbookService>()),
          selectedRow(InvalidSelection),
          selectedFlightId(Flight::InvalidId),
          moduleAction(nullptr)
    {}

    int titleColumnIndex;
    DatabaseService &databaseService;
    FlightService &flightService;
    std::unique_ptr<LogbookService> logbookService;
    int selectedRow;
    qint64 selectedFlightId;
    Unit unit;
    std::unique_ptr<QAction> moduleAction;
};

// PUBLIC

LogbookWidget::LogbookWidget(DatabaseService &databaseService, FlightService &flightService, QWidget *parent) noexcept
    : AbstractModuleWidget(flightService, parent),
      ui(std::make_unique<Ui::LogbookWidget>()),
      d(std::make_unique<LogbookWidgetPrivate>(databaseService, flightService))
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

LogbookWidget::~LogbookWidget() noexcept
{
#ifdef DEBUG
    qDebug("LogbookWidget::~LogbookWidget: DELETED.");
#endif
}

qint64 LogbookWidget::getSelectedFlightId() const noexcept
{
    return d->selectedFlightId;
}

Module::Module LogbookWidget::getModuleId() const noexcept
{
    return Module::Module::Logbook;
}

const QString LogbookWidget::getModuleName() const noexcept
{
    return getName();
}

QAction &LogbookWidget::getAction() noexcept
{
    return *d->moduleAction;
}

// PROTECTED

void LogbookWidget::showEvent(QShowEvent *event) noexcept
{
    AbstractModuleWidget::showEvent(event);

    // Service
    connect(&d->databaseService, &DatabaseService::logbookConnectionChanged,
            this, &LogbookWidget::updateUi);
    connect(&d->flightService, &FlightService::flightStored,
            this, &LogbookWidget::updateUi);
    connect(&d->flightService, &FlightService::flightRestored,
            this, &LogbookWidget::updateAircraftIcon);
    connect(&d->flightService, &FlightService::flightUpdated,
            this, &LogbookWidget::updateUi);
    SkyConnectIntf &skyConnect = SkyManager::getInstance().getCurrentSkyConnect();
    connect(&skyConnect, &SkyConnectIntf::stateChanged,
            this, &LogbookWidget::updateEditUi);

    updateUi();
    handleSelectionChanged();
}

void LogbookWidget::hideEvent(QHideEvent *event) noexcept
{
    AbstractModuleWidget::hideEvent(event);

    disconnect(&d->databaseService, &DatabaseService::logbookConnectionChanged,
               this, &LogbookWidget::updateUi);
    disconnect(&d->flightService, &FlightService::flightStored,
               this, &LogbookWidget::updateUi);
    disconnect(&d->flightService, &FlightService::flightRestored,
               this, &LogbookWidget::updateAircraftIcon);
    disconnect(&d->flightService, &FlightService::flightUpdated,
               this, &LogbookWidget::updateUi);
    SkyConnectIntf &skyConnect = SkyManager::getInstance().getCurrentSkyConnect();
    disconnect(&skyConnect, &SkyConnectIntf::stateChanged,
               this, &LogbookWidget::updateEditUi);
}

// PRIVATE

void LogbookWidget::initUi() noexcept
{
    d->moduleAction = std::make_unique<QAction>(getName());
    d->moduleAction->setCheckable(true);

    ui->logTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    const QStringList headers {tr("Flight"), tr("Date"), tr("Aircraft"), tr("Number of Aircrafts"), tr("Departure Time"), tr("Departure"), tr("Arrival Time"), tr("Arrival"), tr("Total Time of Flight"), tr("Title")};
    ui->logTableWidget->setColumnCount(headers.count());
    ui->logTableWidget->setHorizontalHeaderLabels(headers);
    ui->logTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->logTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->logTableWidget->verticalHeader()->hide();
    ui->logTableWidget->setMinimumWidth(MinimumTableWidth);
    ui->logTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->logTableWidget->sortByColumn(FlightIdColumn, Qt::SortOrder::DescendingOrder);
}

void LogbookWidget::frenchConnection() noexcept
{
    connect(ui->logTableWidget, &QTableWidget::itemSelectionChanged,
            this, &LogbookWidget::handleSelectionChanged);
    connect(ui->loadPushButton, &QPushButton::clicked,
            this, &LogbookWidget::loadFlight);
    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &LogbookWidget::deleteFlight);
    connect(ui->logTableWidget, &QTableWidget::cellDoubleClicked,
            this, &LogbookWidget::handleCellSelected);
    connect(ui->logTableWidget, &QTableWidget::cellChanged,
            this, &LogbookWidget::handleCellChanged);
}

inline void LogbookWidget::insertYear(QTreeWidget *parent, std::forward_list<FlightDate> &flightDatesByYear) noexcept
{
    QTreeWidgetItem *yearItem = new QTreeWidgetItem(parent, QStringList(QString::number(flightDatesByYear.cbegin()->year)));
    while (!flightDatesByYear.empty()) {
        std::forward_list<FlightDate>::const_iterator first = flightDatesByYear.cbegin();
        std::forward_list<FlightDate>::const_iterator last = first;

        // Group by year
        int currentMonth = first->month;
        while (last != flightDatesByYear.end() && last->month == currentMonth) {
            ++last;
        }
        std::forward_list<FlightDate> flightDatesByMonth = {};
        flightDatesByMonth.splice_after(flightDatesByMonth.cbefore_begin(), flightDatesByYear, flightDatesByYear.cbefore_begin(), last);
        insertMonth(yearItem, flightDatesByMonth);
    }
    // The parent takes ownership over yearItem
    // -> suppress the clang code analysis warning about "potential leak of memory"
    // * Is there a better way (annotation) to suppress this warning?
    // * https://stackoverflow.com/questions/65667955/clang-static-analyzer-complains-about-memory-leak-when-using-protobufs-set-allo
#ifdef __clang_analyzer__
    delete yearItem;
#endif
}

inline void LogbookWidget::insertMonth(QTreeWidgetItem *parent, std::forward_list<FlightDate> &flightDatesByMonth) noexcept
{
    QTreeWidgetItem *monthItem = new QTreeWidgetItem(parent, QStringList(d->unit.formatMonth(flightDatesByMonth.cbegin()->month)));
    // The days are already unique
    insertDay(monthItem, flightDatesByMonth);
    // The parent takes ownership over monthItem
    // -> suppress the clang code analysis warning about "potential leak of memory"
#ifdef __clang_analyzer__
    delete monthItem;
#endif
}

inline void LogbookWidget::insertDay(QTreeWidgetItem *parent, std::forward_list<FlightDate> &flightDatesByDayOfMonth) noexcept
{
    for (auto &it: flightDatesByDayOfMonth) {
        new QTreeWidgetItem(parent, QStringList(QString::number(it.dayOfMonth)));
    }
}

const QString LogbookWidget::getName() noexcept
{
    return QString(QT_TRANSLATE_NOOP("LogbookWidget", "Logbook"));
}

// PRIVATE SLOTS

void LogbookWidget::updateUi() noexcept
{
    if (d->databaseService.isConnected()) {

        const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
        const qint64 flightInMemoryId = flight.getId();
        QVector<FlightSummary> summaries = d->logbookService->getFlightSummaries();
        ui->logTableWidget->blockSignals(true);
        ui->logTableWidget->setSortingEnabled(false);
        ui->logTableWidget->clearContents();
        ui->logTableWidget->setRowCount(summaries.count());
        const QIcon aircraftIcon(":/img/icons/aircraft-normal.png");
        int rowIndex = 0;
        for (const FlightSummary &summary : summaries) {

            int columnIndex = 0;

            // ID
            QTableWidgetItem *newItem = new QTableWidgetItem();
            if (summary.id == flightInMemoryId) {
                newItem->setIcon(aircraftIcon);
            }
            newItem->setData(Qt::DisplayRole, summary.id);
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            newItem->setToolTip(tr("Double-click to load flight"));
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // Creation date
            newItem = new QTableWidgetItem(d->unit.formatDate(summary.creationDate));
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // Aircraft type
            newItem = new QTableWidgetItem(summary.aircraftType);
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // Aircraft count
            newItem = new QTableWidgetItem();
            newItem->setData(Qt::DisplayRole, summary.aircraftCount);
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // Start date
            newItem = new QTableWidgetItem(d->unit.formatTime(summary.startDate));
            newItem->setToolTip(tr("Simulation time: %1 (%2Z)").arg(d->unit.formatTime(summary.startSimulationLocalTime), d->unit.formatTime(summary.startSimulationZuluTime)));
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // Start location
            newItem = new QTableWidgetItem(summary.startLocation);
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // End date
            newItem = new QTableWidgetItem(d->unit.formatTime(summary.endDate));
            newItem->setToolTip(tr("Simulation time: %1 (%2Z)").arg(d->unit.formatTime(summary.endSimulationLocalTime), d->unit.formatTime(summary.endSimulationZuluTime)));
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // End location
            newItem = new QTableWidgetItem(summary.endLocation);
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // Duration
            qint64 durationMSec = summary.startDate.msecsTo(summary.endDate);
            QTime time = QTime(0, 0).addMSecs(durationMSec);
            newItem = new QTableWidgetItem(d->unit.formatDuration(time));
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            durationMSec = summary.startSimulationLocalTime.msecsTo(summary.endSimulationLocalTime);
            time = QTime(0, 0).addMSecs(durationMSec);
            newItem->setToolTip(tr("Simulation duration: %1").arg(d->unit.formatDuration(time)));
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // Title
            newItem = new QTableWidgetItem(summary.title);
            newItem->setToolTip(tr("Double-click to edit title"));
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            d->titleColumnIndex = columnIndex;
            ++columnIndex;
            ++rowIndex;
        }

        ui->logTableWidget->setSortingEnabled(true);
        ui->logTableWidget->resizeColumnsToContents();
        ui->logTableWidget->blockSignals(false);

    } else {
        // Clear existing entries
        ui->logTableWidget->setRowCount(0);
    }

    updateDateSelectorUi();
    updateEditUi();
}

void LogbookWidget::updateEditUi() noexcept
{
    const bool active = SkyManager::getInstance().getCurrentSkyConnect().isActive();
    ui->loadPushButton->setEnabled(!active && d->selectedFlightId != Flight::InvalidId);
    ui->deletePushButton->setEnabled(!active && d->selectedFlightId != Flight::InvalidId);
}

void LogbookWidget::updateAircraftIcon() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
    const qint64 flightInMemoryId = flight.getId();
    const QIcon aircraftIcon(":/img/icons/aircraft-normal.png");
    const QIcon emptyIcon;
    for (int row = 0; row < ui->logTableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = ui->logTableWidget->item(row, FlightIdColumn);
        if (item->data(Qt::DisplayRole).toLongLong() == flightInMemoryId) {
            item->setIcon(aircraftIcon);
        } else {
            item->setIcon(emptyIcon);
        }
    }
}

void LogbookWidget::updateDateSelectorUi() noexcept
{
    // Sorted by year, month, day
    std::forward_list<FlightDate> flightDates = d->logbookService->getFlightDates();
    ui->logTreeWidget->blockSignals(true);
    ui->logTreeWidget->reset();

    while (!flightDates.empty()) {
        std::forward_list<FlightDate>::const_iterator first = flightDates.cbegin();
        std::forward_list<FlightDate>::const_iterator last = first;

        // Group by year
        int currentYear = first->year;
        while (last != flightDates.end() && last->year == currentYear) {
            ++last;
        }
        std::forward_list<FlightDate> flightDatesByYear = {};
        flightDatesByYear.splice_after(flightDatesByYear.cbefore_begin(), flightDates, flightDates.cbefore_begin(), last);
        insertYear(ui->logTreeWidget, flightDatesByYear);
    }
}

void LogbookWidget::handleSelectionChanged() noexcept
{
    QItemSelectionModel *select = ui->logTableWidget->selectionModel();
    QModelIndexList modelIndices = select->selectedRows(FlightIdColumn);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        d->selectedRow = modelIndex.row();
        d->selectedFlightId = ui->logTableWidget->model()->data(modelIndex).toLongLong();
    } else {
        d->selectedRow = InvalidSelection;
        d->selectedFlightId = Flight::InvalidId;
    }
    updateEditUi();
}

void LogbookWidget::loadFlight() noexcept
{
    qint64 selectedFlightId = d->selectedFlightId;
    if (selectedFlightId != Flight::InvalidId) {
        const bool ok = d->flightService.restore(selectedFlightId, Logbook::getInstance().getCurrentFlight());
        if (!ok) {
            QMessageBox::critical(this, tr("Database error"), tr("The flight %1 could not be read from the library.").arg(selectedFlightId));
        }
    }
}

void LogbookWidget::deleteFlight() noexcept
{
    if (d->selectedFlightId != Flight::InvalidId) {

        Settings &settings = Settings::getInstance();
        bool doDelete;
        if (settings.isDeleteFlightConfirmationEnabled()) {
            QMessageBox messageBox(this);
            QCheckBox *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), &messageBox);

            messageBox.setText(tr("The flight %1 is about to be deleted. Deletion cannot be undone.").arg(d->selectedFlightId));
            messageBox.setInformativeText(tr("Do you want to delete the flight?"));
            QPushButton *deleteButton = messageBox.addButton(tr("Delete"), QMessageBox::AcceptRole);
            QPushButton *keepButton = messageBox.addButton(tr("Keep"), QMessageBox::RejectRole);
            messageBox.setDefaultButton(keepButton);
            messageBox.setCheckBox(dontAskAgainCheckBox);
            messageBox.setIcon(QMessageBox::Icon::Question);

            messageBox.exec();
            doDelete = messageBox.clickedButton() == deleteButton;
            settings.setDeleteFlightConfirmationEnabled(!dontAskAgainCheckBox->isChecked());
        } else {
            doDelete = true;
        }
        if (doDelete) {
            d->flightService.deleteById(d->selectedFlightId);
            int lastSelectedRow = d->selectedRow;
            updateUi();
            int selectedRow = qMin(lastSelectedRow, ui->logTableWidget->rowCount() - 1);
            ui->logTableWidget->selectRow(selectedRow);
        }
    }
}

void LogbookWidget::handleCellSelected(int row, int column) noexcept
{
    if (column == ui->logTableWidget->columnCount() - 1) {
        QTableWidgetItem *item = ui->logTableWidget->item(row, column);
        ui->logTableWidget->editItem(item);
    } else {
        loadFlight();
    }
}

void LogbookWidget::handleCellChanged(int row, int column) noexcept
{
    if (column == d->titleColumnIndex) {
        QTableWidgetItem *item = ui->logTableWidget->item(row, column);
        const QString title = item->data(Qt::EditRole).toString();

        // Also update the current flight, if in memory
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        if (flight.getId() == d->selectedFlightId) {
            flight.setTitle(title);
        }

        d->flightService.updateTitle(d->selectedFlightId, title);
    }
}
