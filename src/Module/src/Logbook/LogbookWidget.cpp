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
#include <forward_list>
#include <vector>
#include <cstdint>

#include <QCoreApplication>
#include <QVariant>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QDateTime>
#include <QTime>
#include <QTimer>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QAction>
#include <QShowEvent>
#include <QHideEvent>

#include "../../../Kernel/src/Version.h"
#include "../../../Kernel/src/Const.h"
#include "../../../Kernel/src/Enum.h"
#include "../../../Kernel/src/Unit.h"
#include "../../../Kernel/src/Settings.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/FlightSummary.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/SimType.h"
#include "../../../Persistence/src/Dao/FlightSelector.h"
#include "../../../Persistence/src/Service/DatabaseService.h"
#include "../../../Persistence/src/Service/LogbookService.h"
#include "../../../Persistence/src/Service/FlightService.h"
#include "../../../Persistence/src/ConnectionManager.h"
#include "../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../Widget/src/Platform.h"
#include "../../../Widget/src/TableDateItem.h"
#include "../../../Widget/src/TableTimeItem.h"
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

    constexpr int DateColumn = 0;
    constexpr int NofFlightsColumn = 1;

    constexpr int SearchTimeoutMSec = 200;

    enum struct Duration {
        All = 0,
        TwoMinutes = 2,
        FiveMinutes = 5,
        TenMinutes = 10,
        Fifteen = 15,
        ThirtyMinutes = 30,
        OneHour = 60,
        TwoHours = 120,
        ThreeHours = 180,
        FourHours = 240
    };
}

class LogbookWidgetPrivate
{
public:
    LogbookWidgetPrivate(QObject *parent, DatabaseService &theDatabaseService, FlightService &theFlightService) noexcept
        : titleColumnIndex(InvalidColumn),
          databaseService(theDatabaseService),
          flightService(theFlightService),
          logbookService(std::make_unique<LogbookService>()),
          selectedRow(InvalidSelection),
          selectedFlightId(Flight::InvalidId),
          moduleAction(nullptr),
          searchTimer(new QTimer(parent))
    {
        searchTimer->setSingleShot(true);
        searchTimer->setInterval(SearchTimeoutMSec);
    }

    int titleColumnIndex;
    DatabaseService &databaseService;
    FlightService &flightService;
    std::unique_ptr<LogbookService> logbookService;
    int selectedRow;
    std::int64_t selectedFlightId;
    Unit unit;
    std::unique_ptr<QAction> moduleAction;
    FlightSelector flightSelector;
    QTimer *searchTimer;
};

// PUBLIC

LogbookWidget::LogbookWidget(DatabaseService &databaseService, FlightService &flightService, QWidget *parent) noexcept
    : AbstractModuleWidget(flightService, parent),
      ui(std::make_unique<Ui::LogbookWidget>()),
      d(std::make_unique<LogbookWidgetPrivate>(this, databaseService, flightService))
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

std::int64_t LogbookWidget::getSelectedFlightId() const noexcept
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

    // Flight
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::aircraftStored,
            this, &LogbookWidget::updateUi);

    // Service
    connect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
            this, &LogbookWidget::updateUi);
    connect(&d->flightService, &FlightService::flightStored,
            this, &LogbookWidget::updateUi);
    connect(&d->flightService, &FlightService::flightRestored,
            this, &LogbookWidget::updateAircraftIcon);
    connect(&d->flightService, &FlightService::flightUpdated,
            this, &LogbookWidget::updateUi);
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &LogbookWidget::updateEditUi);

    updateUi();
    handleSelectionChanged();
}

void LogbookWidget::hideEvent(QHideEvent *event) noexcept
{
    AbstractModuleWidget::hideEvent(event);

    // Flight
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::aircraftStored,
               this, &LogbookWidget::updateUi);

    // Service
    disconnect(&ConnectionManager::getInstance(), &ConnectionManager::connectionChanged,
               this, &LogbookWidget::updateUi);
    disconnect(&d->flightService, &FlightService::flightStored,
               this, &LogbookWidget::updateUi);
    disconnect(&d->flightService, &FlightService::flightRestored,
               this, &LogbookWidget::updateAircraftIcon);
    disconnect(&d->flightService, &FlightService::flightUpdated,
               this, &LogbookWidget::updateUi);
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    disconnect(&skyConnectManager, &SkyConnectManager::stateChanged,
               this, &LogbookWidget::updateEditUi);
}

// PRIVATE

void LogbookWidget::initUi() noexcept
{
    d->moduleAction = std::make_unique<QAction>(getName());
    d->moduleAction->setCheckable(true);

    // Date selection
    ui->logTreeWidget->setHeaderLabels({tr("Creation Date"), tr("Flights")});

    // Flight log table
    ui->logTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->searchLineEdit->setPlaceholderText(tr("User aircraft, title, departure, arrival"));
    // Make sure that shortcuts are initially accepted
    ui->searchLineEdit->clearFocus();

    const QStringList headers {tr("Flight"), tr("Date"), tr("User Aircraft"), tr("Number of Aircraft"), tr("Departure Time"), tr("Departure"), tr("Arrival Time"), tr("Arrival"), tr("Total Time of Flight"), tr("Title")};
    ui->logTableWidget->setColumnCount(headers.count());
    ui->logTableWidget->setHorizontalHeaderLabels(headers);
    ui->logTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->logTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->logTableWidget->verticalHeader()->hide();
    ui->logTableWidget->setMinimumWidth(MinimumTableWidth);
    ui->logTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->logTableWidget->sortByColumn(FlightIdColumn, Qt::SortOrder::DescendingOrder);

    ui->splitter->setStretchFactor(1, 3);

    // Default "Delete" key deletes flights
    ui->deletePushButton->setShortcut(QKeySequence::Delete);

    initFilterUi();
}

void LogbookWidget::initFilterUi() noexcept
{
    // Duration
    ui->durationComboBox->addItem(tr("All"), Enum::toUnderlyingType(Duration::All));
    ui->durationComboBox->addItem(tr("2 minutes"), Enum::toUnderlyingType(Duration::TwoMinutes));
    ui->durationComboBox->addItem(tr("5 minutes"), Enum::toUnderlyingType(Duration::FiveMinutes));
    ui->durationComboBox->addItem(tr("10 minutes"), Enum::toUnderlyingType(Duration::TenMinutes));
    ui->durationComboBox->addItem(tr("15 minutes"), Enum::toUnderlyingType(Duration::Fifteen));
    ui->durationComboBox->addItem(tr("30 minutes"), Enum::toUnderlyingType(Duration::ThirtyMinutes));
    ui->durationComboBox->addItem(tr("1 hour"), Enum::toUnderlyingType(Duration::OneHour));
    ui->durationComboBox->addItem(tr("2 hours"), Enum::toUnderlyingType(Duration::TwoHours));
    ui->durationComboBox->addItem(tr("3 hours"), Enum::toUnderlyingType(Duration::ThreeHours));
    ui->durationComboBox->addItem(tr("4 hours"), Enum::toUnderlyingType(Duration::FourHours));

    // Engine type
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::All), Enum::toUnderlyingType(SimType::EngineType::All));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::Jet), Enum::toUnderlyingType(SimType::EngineType::Jet));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::Turboprop), Enum::toUnderlyingType(SimType::EngineType::Turboprop));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::Piston), Enum::toUnderlyingType(SimType::EngineType::Piston));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::HeloBellTurbine), Enum::toUnderlyingType(SimType::EngineType::HeloBellTurbine));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::None), Enum::toUnderlyingType(SimType::EngineType::None));
    ui->engineTypeComboBox->setCurrentText(0);
}

void LogbookWidget::updateFlightTable() noexcept
{
    d->selectedFlightId = Flight::InvalidId;
    if (ConnectionManager::getInstance().isConnected()) {

        const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
        const std::int64_t flightInMemoryId = flight.getId();
        std::vector<FlightSummary> summaries = d->logbookService->getFlightSummaries(d->flightSelector);
        ui->logTableWidget->blockSignals(true);
        ui->logTableWidget->setSortingEnabled(false);
        ui->logTableWidget->clearContents();
        ui->logTableWidget->setRowCount(summaries.size());
        const QIcon aircraftIcon(":/img/icons/aircraft-normal.png");
        int rowIndex = 0;
        for (const FlightSummary &summary : summaries) {

            int columnIndex = 0;

            // ID
            QTableWidgetItem *newItem = new QTableWidgetItem();
            if (summary.id == flightInMemoryId) {
                newItem->setIcon(aircraftIcon);
            }
            newItem->setData(Qt::DisplayRole, QVariant::fromValue(summary.id));
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            newItem->setToolTip(tr("Double-click to load flight"));
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // Creation date
            newItem = new TableDateItem(d->unit.formatDate(summary.creationDate), summary.creationDate.date());
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

            // Start time
            newItem = new TableTimeItem(d->unit.formatTime(summary.startDate), summary.startDate.time());
            newItem->setToolTip(tr("Simulation time: %1 (%2Z)").arg(d->unit.formatTime(summary.startSimulationLocalTime), d->unit.formatTime(summary.startSimulationZuluTime)));
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // Start location
            newItem = new QTableWidgetItem(summary.startLocation);
            newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->logTableWidget->setItem(rowIndex, columnIndex, newItem);
            ++columnIndex;

            // End time
            newItem = new TableTimeItem(d->unit.formatTime(summary.endDate), summary.endDate.time());
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
            std::int64_t durationMSec = summary.startDate.msecsTo(summary.endDate);
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
            newItem->setBackground(Platform::getEditableTableCellBGColor());
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

    const int flightCount = ui->logTableWidget->rowCount();
    ui->flightCountLabel->setText(tr("%1 flights", "Number of flights selected in the logbook", flightCount).arg(flightCount));

    updateEditUi();
}

void LogbookWidget::frenchConnection() noexcept
{
    // Search
    connect(d->searchTimer, &QTimer::timeout,
            this, &LogbookWidget::searchText);
    connect(ui->searchLineEdit, &QLineEdit::textChanged,
            this, &LogbookWidget::handleSearchTextChanged);

    // Logbook table
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

    // Date selection
    connect(ui->logTreeWidget, &QTreeWidget::itemClicked,
            this, &LogbookWidget::handleDateItemClicked);
}

inline void LogbookWidget::insertYear(QTreeWidgetItem *parent, std::forward_list<FlightDate> &flightDatesByYear, int nofFlightsPerYear) noexcept
{
    const int year = flightDatesByYear.cbegin()->year;
    QTreeWidgetItem *yearItem = new QTreeWidgetItem(parent, {QString::number(year), QString::number(nofFlightsPerYear)});
    yearItem->setData(DateColumn, Qt::UserRole, year);
    while (!flightDatesByYear.empty()) {
        std::forward_list<FlightDate>::const_iterator first = flightDatesByYear.cbegin();
        std::forward_list<FlightDate>::const_iterator last = first;

        // Group by month
        int currentMonth = first->month;
        int nofFlightsPerMonth = 0;
        while (last != flightDatesByYear.end() && last->month == currentMonth) {
            nofFlightsPerMonth += last->nofFlights;
            ++last;
        }
        std::forward_list<FlightDate> flightDatesByMonth = {};
        flightDatesByMonth.splice_after(flightDatesByMonth.cbefore_begin(), flightDatesByYear, flightDatesByYear.cbefore_begin(), last);
        insertMonth(yearItem, flightDatesByMonth, nofFlightsPerMonth);
    }
    // The parent takes ownership over yearItem
    // -> suppress the clang code analysis warning about "potential leak of memory"
    // * Is there a better way (annotation) to suppress this warning?
    // * https://stackoverflow.com/questions/65667955/clang-static-analyzer-complains-about-memory-leak-when-using-protobufs-set-allo
#ifdef __clang_analyzer__
    delete yearItem;
#endif
}

inline void LogbookWidget::insertMonth(QTreeWidgetItem *parent, std::forward_list<FlightDate> &flightDatesByMonth, int nofFlightsPerMonth) noexcept
{
    const int month = flightDatesByMonth.cbegin()->month;
    QTreeWidgetItem *monthItem = new QTreeWidgetItem(parent, {d->unit.formatMonth(month), QString::number(nofFlightsPerMonth)});
    monthItem->setData(DateColumn, Qt::UserRole, month);
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
        const int dayOfMonth = it.dayOfMonth;
        const int nofFlights = it.nofFlights;
        QTreeWidgetItem *dayItem = new QTreeWidgetItem(parent, {QString::number(dayOfMonth), QString::number(nofFlights)});
        dayItem->setData(DateColumn, Qt::UserRole, dayOfMonth);
    }
}

inline void LogbookWidget::updateSelectionDateRange(QTreeWidgetItem *item) const noexcept
{
    const QTreeWidgetItem *parent = item->parent();
    if (parent != nullptr) {
        const QTreeWidgetItem *parent1 = parent->parent();
        if (parent1 != nullptr) {
            const QTreeWidgetItem *parent2 = parent1->parent();
            if (parent2 != nullptr) {
                // Item: day selected
                const int year = parent1->data(DateColumn, Qt::UserRole).toInt();
                const int month = parent->data(DateColumn, Qt::UserRole).toInt();
                const int day = item->data(DateColumn, Qt::UserRole).toInt();
                d->flightSelector.fromDate.setDate(year, month, day);
                d->flightSelector.toDate = d->flightSelector.fromDate.addDays(1);
            } else {
                // Item: month selected
                const int year = parent->data(DateColumn, Qt::UserRole).toInt();
                const int month = item->data(DateColumn, Qt::UserRole).toInt();
                d->flightSelector.fromDate.setDate(year, month, 1);
                const int daysInMonth = d->flightSelector.fromDate.daysInMonth();
                d->flightSelector.toDate.setDate(year, month, daysInMonth);
            }
        } else {
            // Item: year selected
            const int year = item->data(DateColumn, Qt::UserRole).toInt();
            d->flightSelector.fromDate.setDate(year, 1, 1);
            d->flightSelector.toDate.setDate(year, 12, 31);
        }
    } else {
        // Item: Logbook selected (show all entries)
        d->flightSelector.fromDate = FlightSelector::MinDate;
        d->flightSelector.toDate = FlightSelector::MaxDate;
    }
}

const QString LogbookWidget::getName() noexcept
{
    return QCoreApplication::translate("LogbookWidget", "Logbook");
}

// PRIVATE SLOTS

void LogbookWidget::updateUi() noexcept
{
    updateFlightTable();
    updateDateSelectorUi();
}

void LogbookWidget::updateEditUi() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const bool active = skyConnect && skyConnect->get().isActive();
    ui->loadPushButton->setEnabled(!active && d->selectedFlightId != Flight::InvalidId);
    ui->deletePushButton->setEnabled(!active && d->selectedFlightId != Flight::InvalidId);
}

void LogbookWidget::updateAircraftIcon() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
    const std::int64_t flightInMemoryId = flight.getId();
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
    if (ConnectionManager::getInstance().isConnected()) {
        // Sorted by year, month, day
        std::forward_list<FlightDate> flightDates = d->logbookService->getFlightDates();
        ui->logTreeWidget->blockSignals(true);
        ui->logTreeWidget->clear();

        QTreeWidgetItem *logbookItem = new QTreeWidgetItem(ui->logTreeWidget, QStringList(tr("Logbook")));

        int totalFlights = 0;
        while (!flightDates.empty()) {
            std::forward_list<FlightDate>::const_iterator first = flightDates.cbegin();
            std::forward_list<FlightDate>::const_iterator last = first;

            // Group by year
            int currentYear = first->year;
            int nofFlightsPerYear = 0;
            while (last != flightDates.end() && last->year == currentYear) {
                nofFlightsPerYear += last->nofFlights;
                ++last;
            }
            std::forward_list<FlightDate> flightDatesByYear = {};
            flightDatesByYear.splice_after(flightDatesByYear.cbefore_begin(), flightDates, flightDates.cbefore_begin(), last);
            insertYear(logbookItem, flightDatesByYear, nofFlightsPerYear);

            totalFlights += nofFlightsPerYear;
        }

        // Expand all "first" children
        QTreeWidgetItem *item = logbookItem;
        while (item->childCount() > 0) {
            item->setExpanded(true);
            item = item->child(0);
        }

        ui->logTreeWidget->resizeColumnToContents(DateColumn);
        logbookItem->setData(NofFlightsColumn, Qt::DisplayRole, totalFlights);

        ui->logTreeWidget->blockSignals(false);
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
    std::int64_t selectedFlightId = d->selectedFlightId;
    if (selectedFlightId != Flight::InvalidId) {
        const bool ok = d->flightService.restore(selectedFlightId, Logbook::getInstance().getCurrentFlight());
        if (!ok) {
            QMessageBox::critical(this, tr("Database error"), tr("The flight %1 could not be read from the logbook.").arg(selectedFlightId));
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

void LogbookWidget::handleSearchTextChanged() noexcept
{
    d->searchTimer->start();
}

void LogbookWidget::searchText() noexcept
{
#ifdef DEBUG
    qDebug("LogbookWidget::searchText: initiating search for: %s", qPrintable(ui->searchLineEdit->text()));
#endif
    d->flightSelector.searchKeyword = ui->searchLineEdit->text();
    updateFlightTable();
}

void LogbookWidget::handleCellSelected(int row, int column) noexcept
{
    if (column == d->titleColumnIndex) {
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

        Flight &flight = Logbook::getInstance().getCurrentFlight();
        if (flight.getId() == d->selectedFlightId) {
            // Also update the current flight, if in memory
            d->flightService.updateTitle(flight, title);
        } else {
            d->flightService.updateTitle(d->selectedFlightId, title);
        }
    }
}

void LogbookWidget::handleDateItemClicked(QTreeWidgetItem *item) noexcept
{
    updateSelectionDateRange(item);
    updateFlightTable();
}

void LogbookWidget::on_formationCheckBox_toggled(bool checked) noexcept
{
    d->flightSelector.hasFormation = checked;
    updateFlightTable();
}

void LogbookWidget::on_engineTypeComboBox_activated([[maybe_unused]] int index) noexcept
{
    d->flightSelector.engineType = static_cast<SimType::EngineType>(ui->engineTypeComboBox->currentData().toInt());
    updateFlightTable();
}

void LogbookWidget::on_durationComboBox_activated([[maybe_unused]] int index) noexcept
{
    int minimumDurationMinutes;
    switch (static_cast<Duration>(ui->durationComboBox->currentData().toUInt())) {
    case Duration::All:
        minimumDurationMinutes = 0;
        break;
    case Duration::TwoMinutes:
        minimumDurationMinutes = 2;
        break;
    case Duration::FiveMinutes:
        minimumDurationMinutes = 5;
        break;
    case Duration::TenMinutes:
        minimumDurationMinutes = 10;
        break;
    case Duration::Fifteen:
        minimumDurationMinutes = 15;
        break;
    case Duration::ThirtyMinutes:
        minimumDurationMinutes = 30;
        break;
    case Duration::OneHour:
        minimumDurationMinutes = 60;
        break;
    case Duration::TwoHours:
        minimumDurationMinutes = 120;
        break;
    case Duration::ThreeHours:
        minimumDurationMinutes = 180;
        break;
    case Duration::FourHours:
        minimumDurationMinutes = 240;
        break;
    default:
        minimumDurationMinutes = 0;
        break;
    }

    d->flightSelector.mininumDurationMinutes = minimumDurationMinutes;
    updateFlightTable();
}
