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
#include <algorithm>
#include <memory>
#include <forward_list>
#include <vector>
#include <iterator>
#include <cstdint>
#include <limits>

#include <QByteArray>
#include <QVariant>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QStringList>
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
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Version.h>
#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include <Kernel/Unit.h>
#include <Kernel/Settings.h>
#include <Model/Flight.h>
#include <Model/FlightSummary.h>
#include <Model/Logbook.h>
#include <Model/SimType.h>
#include <Model/FlightDate.h>
#include <Persistence/FlightSelector.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/Service/LogbookService.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/LogbookManager.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <Widget/Platform.h>
#include <Widget/TableDateItem.h>
#include <Widget/TableTimeItem.h>
#include "LogbookWidget.h"
#include "ui_LogbookWidget.h"

namespace
{
    constexpr int MinimumTableWidth {120};

    // Logbook table
    constexpr int InvalidRowIndex {-1};
    constexpr int InvalidColumnIndex {-1};

    // Date selection tree view
    constexpr int DateColumnIndex {0};
    constexpr int NofFlightsColumnIndex {1};
    constexpr int NofFlightsColumnWidth {40};

    constexpr int SearchTimeoutMSec {200};

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

    constexpr int RecordingInProgressId {std::numeric_limits<int>::max()};
    constexpr bool RecordingInProgress {true};
}

struct LogbookWidgetPrivate
{
    LogbookWidgetPrivate(FlightService &theFlightService) noexcept
        : flightService(theFlightService)
    {
        searchTimer->setSingleShot(true);
        searchTimer->setInterval(::SearchTimeoutMSec);
    }

    std::unique_ptr<DatabaseService> databaseService {std::make_unique<DatabaseService>()};
    std::unique_ptr<LogbookService> logbookService {std::make_unique<LogbookService>()};
    FlightService &flightService;

    // Flight table columns
    int flightIdColumnIndex {::InvalidColumnIndex};
    int userAircraftColumnIndex {::InvalidColumnIndex};
    int titleColumnIndex {::InvalidColumnIndex};
    int selectedRow {::InvalidRowIndex};


    std::int64_t selectedFlightId {Flight::InvalidId};
    std::int64_t flightInMemoryId {Flight::InvalidId};
    Unit unit;
    FlightSelector flightSelector;
    std::unique_ptr<QTimer> searchTimer {std::make_unique<QTimer>()};
    // Columns are only auto-resized the first time the table is loaded
    // After that manual column resizes are kept
    bool columnsAutoResized {false};
};

// PUBLIC

LogbookWidget::LogbookWidget(FlightService &flightService, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::LogbookWidget>()),
      d(std::make_unique<LogbookWidgetPrivate>(flightService))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    onSelectionChanged();
    frenchConnection();
#ifdef DEBUG
    qDebug() << "LogbookWidget::LogbookWidget: CREATED.";
#endif
}

LogbookWidget::~LogbookWidget() noexcept
{
    const QByteArray logbookState = ui->logTableWidget->horizontalHeader()->saveState();
    Settings::getInstance().setLogbookState(logbookState);
#ifdef DEBUG
    qDebug() << "LogbookWidget::~LogbookWidget: DELETED.";
#endif
}

std::int64_t LogbookWidget::getSelectedFlightId() const noexcept
{
    return d->selectedFlightId;
}

// PRIVATE

void LogbookWidget::initUi() noexcept
{
    // Date selection
    ui->logTreeWidget->setHeaderLabels({tr("Creation Date"), tr("Flights")});

    // Flight log table
    ui->logTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->searchLineEdit->setPlaceholderText(tr("User aircraft, title, departure, arrival"));
    // Make sure that shortcuts are initially accepted
    ui->searchLineEdit->clearFocus();
    ui->searchLineEdit->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

    const QStringList headers {tr("Flight"), tr("Title"), tr("User Aircraft"), tr("Number of Aircraft"), tr("Date"), tr("Departure Time"), tr("Departure"), tr("Arrival Time"), tr("Arrival"), tr("Total Time of Flight")};
    d->flightIdColumnIndex = headers.indexOf(tr("Flight"));
    d->userAircraftColumnIndex = headers.indexOf(tr("User Aircraft"));
    d->titleColumnIndex = headers.indexOf(tr("Title"));

    ui->logTableWidget->setColumnCount(headers.count());
    ui->logTableWidget->setHorizontalHeaderLabels(headers);
    ui->logTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->logTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->logTableWidget->verticalHeader()->hide();
    ui->logTableWidget->setMinimumWidth(::MinimumTableWidth);
    ui->logTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->logTableWidget->sortByColumn(d->flightIdColumnIndex, Qt::SortOrder::DescendingOrder);
    ui->logTableWidget->horizontalHeader()->setSectionsMovable(true);
    ui->logTableWidget->setAlternatingRowColors(true);

    QByteArray logbookState = Settings::getInstance().getLogbookState();
    ui->logTableWidget->horizontalHeader()->restoreState(logbookState);

    QHeaderView *header = ui->logTreeWidget->header();
    header->setSectionResizeMode(QHeaderView::Fixed);
    header->resizeSection(::NofFlightsColumnIndex, ::NofFlightsColumnWidth);

    const int logTreeWidth = ui->logTreeWidget->minimumWidth();
    ui->splitter->setSizes({logTreeWidth, width() - logTreeWidth});
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);

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
    if (LogbookManager::getInstance().isConnected()) {

        const Flight &flight = Logbook::getInstance().getCurrentFlight();
        d->flightInMemoryId = flight.getId();
        std::vector<FlightSummary> summaries = d->logbookService->getFlightSummaries(d->flightSelector);

        const bool recording = SkyConnectManager::getInstance().isInRecordingState();
        if (recording) {
            FlightSummary summary = flight.getFlightSummary();
            summary.flightId = ::RecordingInProgressId;
            summaries.push_back(std::move(summary));
        }

        ui->logTableWidget->blockSignals(true);
        ui->logTableWidget->setSortingEnabled(false);
        ui->logTableWidget->clearContents();
        ui->logTableWidget->setRowCount(summaries.size());

        int rowIndex {0};
        for (const FlightSummary &summary : summaries) {
            updateFlightSummaryRow(summary, rowIndex);
            ++rowIndex;
        }

        ui->logTableWidget->setSortingEnabled(true);
        if (!d->columnsAutoResized) {
            ui->logTableWidget->resizeColumnsToContents();
            // Reserve some space for the aircraft icon
            const int idColumnWidth = static_cast<int>(std::round(1.25 * ui->logTableWidget->columnWidth(d->flightIdColumnIndex)));
            ui->logTableWidget->setColumnWidth(d->flightIdColumnIndex, idColumnWidth);
            d->columnsAutoResized = true;
        }
        ui->logTableWidget->blockSignals(false);

    } else {
        // Clear existing entries
        ui->logTableWidget->setRowCount(0);
    }

    const int flightCount = ui->logTableWidget->rowCount();
    ui->flightCountLabel->setText(tr("%1 flights", "Number of flights selected in the logbook", flightCount).arg(flightCount));

    updateEditUi();
}

inline void LogbookWidget::updateFlightSummaryRow(const FlightSummary &summary, int rowIndex) noexcept
{
    int columnIndex {0};

    // ID
    std::unique_ptr<QTableWidgetItem> newItem = std::make_unique<QTableWidgetItem>();
    QVariant flightId = QVariant::fromValue(summary.flightId);
    if (summary.flightId == d->flightInMemoryId) {
        newItem->setIcon(QIcon(":/img/icons/aircraft-normal.png"));
    } else if (summary.flightId == ::RecordingInProgressId) {
        newItem->setIcon(QIcon(":/img/icons/aircraft-record-normal.png"));
        // Note: alphabetical characters (a-zA-Z) will be > numerical characters (0-9),
        //       so the flight being recorded will be properly sorted in the table
        flightId = QVariant::fromValue(tr("REC"));
        newItem->setData(Qt::UserRole, ::RecordingInProgress);
    }
    newItem->setData(Qt::DisplayRole, flightId);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newItem->setToolTip(tr("Double-click to load flight."));
    // Transfer ownership of newItem to table widget
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Title
    newItem = std::make_unique<QTableWidgetItem>(summary.title);
    newItem->setToolTip(tr("Double-click to edit title."));
    newItem->setBackground(Platform::getEditableTableCellBGColor());
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Aircraft type
    newItem = std::make_unique<QTableWidgetItem>(summary.aircraftType);
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Aircraft count
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::DisplayRole, summary.aircraftCount);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Creation date
    newItem = std::make_unique<TableDateItem>(d->unit.formatDate(summary.creationDate), summary.creationDate.date());
    newItem->setToolTip(tr("Recording time: %1.").arg(d->unit.formatTime(summary.creationDate)));
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Start time
    newItem = std::make_unique<TableTimeItem>(d->unit.formatTime(summary.startSimulationLocalTime), summary.startSimulationLocalTime.time());
    newItem->setToolTip(tr("Simulation time (%1Z).").arg(d->unit.formatTime(summary.startSimulationZuluTime)));
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Start location
    newItem = std::make_unique<QTableWidgetItem>(summary.startLocation);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // End time
    newItem = std::make_unique<TableTimeItem>(d->unit.formatTime(summary.endSimulationLocalTime), summary.endSimulationLocalTime.time());
    newItem->setToolTip(tr("Simulation time (%1Z).").arg(d->unit.formatTime(summary.endSimulationZuluTime)));
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // End location
    newItem = std::make_unique<QTableWidgetItem>(summary.endLocation);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Duration
    const std::int64_t durationMSec = summary.startSimulationLocalTime.msecsTo(summary.endSimulationLocalTime);
    const QTime time = QTime::fromMSecsSinceStartOfDay(static_cast<int>(durationMSec));
    newItem = std::make_unique<QTableWidgetItem>(d->unit.formatDuration(time));
    newItem->setToolTip(tr("Simulation duration."));
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;
}

void LogbookWidget::updateDateSelectorUi() noexcept
{
    if (LogbookManager::getInstance().isConnected()) {
        // Sorted by year, month, day
        std::forward_list<FlightDate> flightDates;
        d->logbookService->getFlightDates(std::front_insert_iterator(flightDates));
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
        logbookItem->setData(::NofFlightsColumnIndex, Qt::DisplayRole, totalFlights);

        // Adjust column size when all items are expanded
        ui->logTreeWidget->expandAll();
        ui->logTreeWidget->resizeColumnToContents(::DateColumnIndex);
        ui->logTreeWidget->collapseAll();

        // Expand all "first" children (only)
        QTreeWidgetItem *item = logbookItem;
        while (item->childCount() > 0) {
            item->setExpanded(true);
            item = item->child(0);
        }
        ui->logTreeWidget->blockSignals(false);
    }
}

void LogbookWidget::updateEditUi() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const bool active = skyConnect && skyConnect->get().isActive();
    ui->loadPushButton->setEnabled(!active && d->selectedFlightId != Flight::InvalidId);
    ui->deletePushButton->setEnabled(!active && d->selectedFlightId != Flight::InvalidId);
}

void LogbookWidget::frenchConnection() noexcept
{
    // Logbook
    const Logbook &logbook = Logbook::getInstance();
    connect(&LogbookManager::getInstance(), &LogbookManager::connectionChanged,
            this, &LogbookWidget::updateUi);
    connect(&logbook, &Logbook::flightTitleOrDescriptionChanged,
            this, &LogbookWidget::updateUi);

    // Flight
    const Flight &flight = logbook.getCurrentFlight();
    connect(&flight, &Flight::flightStored,
            this, &LogbookWidget::updateUi);
    connect(&flight, &Flight::flightRestored,
            this, &LogbookWidget::updateAircraftIcons);
    connect(&flight, &Flight::aircraftStored,
            this, &LogbookWidget::updateUi);
    connect(&flight, &Flight::aircraftInfoChanged,
            this, &LogbookWidget::onAircraftInfoChanged);

    // Connection
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::recordingStarted,
            this, &LogbookWidget::onRecordingStarted);
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &LogbookWidget::updateEditUi);

    // Search
    connect(ui->searchLineEdit, &QLineEdit::textChanged,
            this, &LogbookWidget::onSearchTextChanged);
    connect(d->searchTimer.get(), &QTimer::timeout,
            this, &LogbookWidget::searchText);    

    // Logbook table
    connect(ui->logTableWidget, &QTableWidget::itemSelectionChanged,
            this, &LogbookWidget::onSelectionChanged);
    connect(ui->loadPushButton, &QPushButton::clicked,
            this, &LogbookWidget::loadFlight);
    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &LogbookWidget::deleteFlight);
    connect(ui->logTableWidget, &QTableWidget::cellDoubleClicked,
            this, &LogbookWidget::onCellSelected);
    connect(ui->logTableWidget, &QTableWidget::cellChanged,
            this, &LogbookWidget::onCellChanged);

    // Filter options
    connect(ui->formationCheckBox, &QCheckBox::toggled,
            this, &LogbookWidget::filterByFormationFlights);
    connect(ui->engineTypeComboBox, &QComboBox::activated,
            this, &LogbookWidget::filterByEngineType);
    connect(ui->durationComboBox, &QComboBox::activated,
            this, &LogbookWidget::filterByDuration);

    // Date selection
    connect(ui->logTreeWidget, &QTreeWidget::itemClicked,
            this, &LogbookWidget::onDateItemClicked);
}

inline void LogbookWidget::insertYear(QTreeWidgetItem *parent, std::forward_list<FlightDate> &flightDatesByYear, int nofFlightsPerYear) noexcept
{
    const int year = flightDatesByYear.cbegin()->year;
    QTreeWidgetItem *yearItem = new QTreeWidgetItem(parent, {QString::number(year), QString::number(nofFlightsPerYear)});
    yearItem->setData(::DateColumnIndex, Qt::UserRole, year);
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
    monthItem->setData(DateColumnIndex, Qt::UserRole, month);
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
        dayItem->setData(DateColumnIndex, Qt::UserRole, dayOfMonth);
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
                const int year = parent1->data(::DateColumnIndex, Qt::UserRole).toInt();
                const int month = parent->data(::DateColumnIndex, Qt::UserRole).toInt();
                const int day = item->data(::DateColumnIndex, Qt::UserRole).toInt();
                d->flightSelector.fromDate.setDate(year, month, day);
                d->flightSelector.toDate = d->flightSelector.fromDate.addDays(1);
            } else {
                // Item: month selected
                const int year = parent->data(::DateColumnIndex, Qt::UserRole).toInt();
                const int month = item->data(::DateColumnIndex, Qt::UserRole).toInt();
                d->flightSelector.fromDate.setDate(year, month, 1);
                const int daysInMonth = d->flightSelector.fromDate.daysInMonth();
                d->flightSelector.toDate.setDate(year, month, daysInMonth);
            }
        } else {
            // Item: year selected
            const int year = item->data(::DateColumnIndex, Qt::UserRole).toInt();
            d->flightSelector.fromDate.setDate(year, 1, 1);
            d->flightSelector.toDate.setDate(year, 12, 31);
        }
    } else {
        // Item: Logbook selected (show all entries)
        d->flightSelector.fromDate = FlightSelector::MinDate;
        d->flightSelector.toDate = FlightSelector::MaxDate;
    }
}

// PRIVATE SLOTS

void LogbookWidget::onRecordingStarted() noexcept
{
    if (SkyConnectManager::getInstance().isInRecordingState()) {
        const Flight &flight = Logbook::getInstance().getCurrentFlight();
        FlightSummary summary = flight.getFlightSummary();
        summary.flightId = ::RecordingInProgressId;
        ui->logTableWidget->blockSignals(true);
        ui->logTableWidget->setSortingEnabled(false);
        const int rowIndex = ui->logTableWidget->rowCount();
        ui->logTableWidget->insertRow(rowIndex);
        updateFlightSummaryRow(summary, rowIndex);
        updateAircraftIcons();
        ui->logTableWidget->setSortingEnabled(true);
        ui->logTableWidget->blockSignals(false);
    }
}

void LogbookWidget::updateUi() noexcept
{
    updateFlightTable();
    updateDateSelectorUi();
}

void LogbookWidget::updateAircraftIcons() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const std::int64_t flightInMemoryId = flight.getId();

    for (int row = 0; row < ui->logTableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = ui->logTableWidget->item(row, d->flightIdColumnIndex);
        if (item->data(Qt::DisplayRole).toLongLong() == flightInMemoryId) {
            item->setIcon(QIcon(":/img/icons/aircraft-normal.png"));
        } else if (item->data(Qt::UserRole).toBool() == ::RecordingInProgress) {
            item->setIcon(QIcon(":/img/icons/aircraft-record-normal.png"));
        } else {
            item->setIcon(QIcon());
        }
    }
}

void LogbookWidget::onAircraftInfoChanged(Aircraft &aircraft)
{
    ui->logTableWidget->setSortingEnabled(false);
    for (int row = 0; row < ui->logTableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = ui->logTableWidget->item(row, d->userAircraftColumnIndex);
        if (item->data(Qt::DisplayRole).toString().isEmpty()) {
            item = ui->logTableWidget->item(row, d->userAircraftColumnIndex);
            item->setData(Qt::DisplayRole, aircraft.getAircraftInfo().aircraftType.type);
            break;
        }
    }
     ui->logTableWidget->setSortingEnabled(true);
}

void LogbookWidget::loadFlight() noexcept
{
    if (!SkyConnectManager::getInstance().isInRecordingState()) {
        std::int64_t selectedFlightId = d->selectedFlightId;
        if (selectedFlightId != Flight::InvalidId) {
            const bool ok = d->flightService.restore(selectedFlightId, Logbook::getInstance().getCurrentFlight());
            if (!ok) {
                QMessageBox::critical(this, tr("Logbook error"), tr("The flight %1 could not be read from the logbook.").arg(selectedFlightId));
            }
        }
    }
}

void LogbookWidget::deleteFlight() noexcept
{
    if (d->selectedFlightId != Flight::InvalidId) {

        Settings &settings = Settings::getInstance();
        bool doDelete {true};
        if (settings.isDeleteFlightConfirmationEnabled()) {
            std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
            QCheckBox *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), messageBox.get());

            messageBox->setWindowTitle(tr("Delete Flight"));
            messageBox->setText(tr("The flight %1 is about to be deleted. Deletion cannot be undone.").arg(d->selectedFlightId));
            messageBox->setInformativeText(tr("Do you want to delete the flight?"));
            QPushButton *deleteButton = messageBox->addButton(tr("&Delete"), QMessageBox::AcceptRole);
            QPushButton *keepButton = messageBox->addButton(tr("&Keep"), QMessageBox::RejectRole);
            messageBox->setDefaultButton(keepButton);
            messageBox->setCheckBox(dontAskAgainCheckBox);
            messageBox->setIcon(QMessageBox::Icon::Question);

            messageBox->exec();
            doDelete = messageBox->clickedButton() == deleteButton;
            settings.setDeleteFlightConfirmationEnabled(!dontAskAgainCheckBox->isChecked());
        }

        if (doDelete) {
            d->flightService.deleteById(d->selectedFlightId);
            int lastSelectedRow = d->selectedRow;
            updateUi();
            int selectedRow = std::min(lastSelectedRow, ui->logTableWidget->rowCount() - 1);
            ui->logTableWidget->selectRow(selectedRow);
            ui->logTableWidget->setFocus(Qt::NoFocusReason);
        }
    }
}

void LogbookWidget::onSearchTextChanged() noexcept
{
    d->searchTimer->start();
}

void LogbookWidget::searchText() noexcept
{
    d->flightSelector.searchKeyword = ui->searchLineEdit->text();
    updateFlightTable();
}

void LogbookWidget::onSelectionChanged() noexcept
{
    QItemSelectionModel *select = ui->logTableWidget->selectionModel();
    QModelIndexList modelIndices = select->selectedRows(d->flightIdColumnIndex);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        d->selectedRow = modelIndex.row();
        d->selectedFlightId = ui->logTableWidget->model()->data(modelIndex).toLongLong();
    } else {
        d->selectedRow = ::InvalidRowIndex;
        d->selectedFlightId = Flight::InvalidId;
    }
    updateEditUi();
}

void LogbookWidget::onCellSelected(int row, int column) noexcept
{
    if (column == d->titleColumnIndex) {
        QTableWidgetItem *item = ui->logTableWidget->item(row, column);
        ui->logTableWidget->editItem(item);
    } else {
        loadFlight();
    }
}

void LogbookWidget::onCellChanged(int row, int column) noexcept
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

void LogbookWidget::onDateItemClicked(QTreeWidgetItem *item) noexcept
{
    updateSelectionDateRange(item);
    updateFlightTable();
}

void LogbookWidget::filterByFormationFlights(bool checked) noexcept
{
    d->flightSelector.hasFormation = checked;
    updateFlightTable();
}

void LogbookWidget::filterByEngineType([[maybe_unused]] int index) noexcept
{
    d->flightSelector.engineType = static_cast<SimType::EngineType>(ui->engineTypeComboBox->currentData().toInt());
    updateFlightTable();
}

void LogbookWidget::filterByDuration([[maybe_unused]] int index) noexcept
{
    int minimumDurationMinutes {0};
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
    }

    d->flightSelector.mininumDurationMinutes = minimumDurationMinutes;
    updateFlightTable();
}