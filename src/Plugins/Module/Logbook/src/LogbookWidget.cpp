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
#include <algorithm>
#include <memory>
#include <forward_list>
#include <vector>
#include <cstdint>
#include <utility>
#include <optional>
#include <cstdint>

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
#include <Persistence/PersistenceManager.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <Widget/Platform.h>
#include <Widget/TableDateItem.h>
#include <Widget/TableTimeItem.h>
#include <Widget/TableDurationItem.h>
#include "LogbookSettings.h"
#include "LogbookWidget.h"
#include "ui_LogbookWidget.h"

namespace
{
    constexpr int MinimumTableWidth {120};

    // Logbook table
    constexpr int InvalidRow {-1};
    constexpr int InvalidColumn {-1};

    // Date selection tree view
    constexpr int DateColumn {0};
    constexpr int NofFlightsColumn {1};
    constexpr int NofFlightsColumnWidth {40};

    enum struct Duration: std::uint8_t {
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

    constexpr int SearchTimeoutMSec {200};
}

struct LogbookWidgetPrivate
{
    LogbookWidgetPrivate(LogbookSettings &moduleSettings) noexcept
        : moduleSettings(moduleSettings)
    {
        searchTimer->setSingleShot(true);
        searchTimer->setInterval(::SearchTimeoutMSec);
    }

    LogbookSettings &moduleSettings;
    std::unique_ptr<FlightService> flightService {std::make_unique<FlightService>()};
    std::unique_ptr<DatabaseService> databaseService {std::make_unique<DatabaseService>()};
    std::unique_ptr<LogbookService> logbookService {std::make_unique<LogbookService>()};

    std::int64_t flightInMemoryId {Const::InvalidId};
    Unit unit;
    std::unique_ptr<QTimer> searchTimer {std::make_unique<QTimer>()};

    // Flight table columns
    static inline int flightIdColumn {::InvalidColumn};
    static inline int titleColumn {::InvalidColumn};
    static inline int flightNumberColumn {::InvalidColumn};
    static inline int userAircraftColumn {::InvalidColumn};
    static inline int aircraftCountColumn {::InvalidColumn};
    static inline int recordingDateColumn {::InvalidColumn};
    static inline int startTimeColumn {::InvalidColumn};
    static inline int startLocationColumn {::InvalidColumn};
    static inline int endTimeColumn {::InvalidColumn};
    static inline int endLocationColumn {::InvalidColumn};
    static inline int durationColumn {::InvalidColumn};
};

// PUBLIC

LogbookWidget::LogbookWidget(LogbookSettings &moduleSettings, QWidget *parent) noexcept
    : QWidget {parent},
      ui {std::make_unique<Ui::LogbookWidget>()},
      d {std::make_unique<LogbookWidgetPrivate>(moduleSettings)}
{
    ui->setupUi(this);
    initUi();
    // The logbook table is updated once the plugin settings are restored (initiated
    // by LogbookPlugin)
    updateDateSelectorUi();
    frenchConnection();
}

LogbookWidget::~LogbookWidget() = default;

// PROTECTED

void LogbookWidget::showEvent(QShowEvent *event) noexcept
{
    QWidget::showEvent(event);

    QByteArray tableState = d->moduleSettings.getLogbookTableState();
    if (!tableState.isEmpty()) {
        ui->logTableWidget->horizontalHeader()->blockSignals(true);
        ui->logTableWidget->horizontalHeader()->restoreState(tableState);
        ui->logTableWidget->horizontalHeader()->blockSignals(false);
    } else {
        ui->logTableWidget->resizeColumnsToContents();
        // Reserve some space for the aircraft icon
        const int idColumnWidth = static_cast<int>(std::round(1.25 * ui->logTableWidget->columnWidth(LogbookWidgetPrivate::flightIdColumn)));
        ui->logTableWidget->setColumnWidth(LogbookWidgetPrivate::flightIdColumn, idColumnWidth);
    }
    // Sort with the current sort section and order
    ui->logTableWidget->setSortingEnabled(true);

    // Wait until table widget columns (e.g. visibility) have been fully initialised
    connect(ui->logTableWidget->horizontalHeader(), &QHeaderView::sectionMoved,
            this, &LogbookWidget::onTableLayoutChanged);
    connect(ui->logTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
            this, &LogbookWidget::onTableLayoutChanged);
    connect(ui->logTableWidget->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, &LogbookWidget::onTableLayoutChanged);
}

// PRIVATE

void LogbookWidget::initUi() noexcept
{
    // Date selection
    ui->logTreeWidget->setHeaderLabels({tr("Creation Date"), tr("Flights")});

    // Flight log table
    ui->logTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->searchLineEdit->setPlaceholderText(tr("User aircraft, title, flight number, departure, arrival"));
    // Make sure that shortcuts are initially accepted
    ui->searchLineEdit->clearFocus();
    ui->searchLineEdit->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    ui->searchLineEdit->setClearButtonEnabled(true);

    const QStringList headers {
        tr("Flight"),
        tr("Title"),
        tr("Flight Number"),
        tr("User Aircraft"),        
        tr("Number of Aircraft"),
        tr("Recording Date"),
        tr("Departure Time"),
        tr("Departure"),
        tr("Arrival Time"),
        tr("Arrival"),
        tr("Total Time of Flight")
    };
    LogbookWidgetPrivate::flightIdColumn = static_cast<int>(headers.indexOf(tr("Flight")));
    LogbookWidgetPrivate::titleColumn = static_cast<int>(headers.indexOf(tr("Title")));
    LogbookWidgetPrivate::flightNumberColumn = static_cast<int>(headers.indexOf(tr("Flight Number")));
    LogbookWidgetPrivate::userAircraftColumn = static_cast<int>(headers.indexOf(tr("User Aircraft")));
    LogbookWidgetPrivate::aircraftCountColumn = static_cast<int>(headers.indexOf(tr("Number of Aircraft")));
    LogbookWidgetPrivate::recordingDateColumn = static_cast<int>(headers.indexOf(tr("Recording Date")));
    LogbookWidgetPrivate::startTimeColumn = static_cast<int>(headers.indexOf(tr("Departure Time")));
    LogbookWidgetPrivate::startLocationColumn = static_cast<int>(headers.indexOf(tr("Departure")));
    LogbookWidgetPrivate::endTimeColumn = static_cast<int>(headers.indexOf(tr("Arrival Time")));
    LogbookWidgetPrivate::endLocationColumn = static_cast<int>(headers.indexOf(tr("Arrival")));
    LogbookWidgetPrivate::durationColumn = static_cast<int>(headers.indexOf(tr("Total Time of Flight")));

    ui->logTableWidget->setColumnCount(static_cast<int>(headers.count()));
    ui->logTableWidget->setHorizontalHeaderLabels(headers);
    ui->logTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->logTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->logTableWidget->verticalHeader()->hide();
    ui->logTableWidget->setMinimumWidth(::MinimumTableWidth);
    ui->logTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->logTableWidget->sortByColumn(LogbookWidgetPrivate::flightIdColumn, Qt::SortOrder::DescendingOrder);
    ui->logTableWidget->horizontalHeader()->setSectionsMovable(true);
    ui->logTableWidget->setAlternatingRowColors(true);

    QHeaderView *header = ui->logTreeWidget->header();
    header->setSectionResizeMode(QHeaderView::Fixed);
    header->resizeSection(::NofFlightsColumn, ::NofFlightsColumnWidth);

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
    ui->durationComboBox->addItem(tr("All"), Enum::underly(Duration::All));
    ui->durationComboBox->addItem(tr("2 minutes"), Enum::underly(Duration::TwoMinutes));
    ui->durationComboBox->addItem(tr("5 minutes"), Enum::underly(Duration::FiveMinutes));
    ui->durationComboBox->addItem(tr("10 minutes"), Enum::underly(Duration::TenMinutes));
    ui->durationComboBox->addItem(tr("15 minutes"), Enum::underly(Duration::Fifteen));
    ui->durationComboBox->addItem(tr("30 minutes"), Enum::underly(Duration::ThirtyMinutes));
    ui->durationComboBox->addItem(tr("1 hour"), Enum::underly(Duration::OneHour));
    ui->durationComboBox->addItem(tr("2 hours"), Enum::underly(Duration::TwoHours));
    ui->durationComboBox->addItem(tr("3 hours"), Enum::underly(Duration::ThreeHours));
    ui->durationComboBox->addItem(tr("4 hours"), Enum::underly(Duration::FourHours));

    // Engine type
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::All), Enum::underly(SimType::EngineType::All));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::Jet), Enum::underly(SimType::EngineType::Jet));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::Turboprop), Enum::underly(SimType::EngineType::Turboprop));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::Piston), Enum::underly(SimType::EngineType::Piston));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::HeloBellTurbine), Enum::underly(SimType::EngineType::HeloBellTurbine));
    ui->engineTypeComboBox->addItem(SimType::engineTypeToString(SimType::EngineType::None), Enum::underly(SimType::EngineType::None));
}

void LogbookWidget::updateTable() noexcept
{
    if (PersistenceManager::getInstance().isConnected()) {

        const auto &flight = Logbook::getInstance().getCurrentFlight();
        d->flightInMemoryId = flight.getId();
        auto summaries = d->logbookService->getFlightSummaries(d->moduleSettings.getFlightSelector());

        const bool recording = SkyConnectManager::getInstance().isInRecordingState();
        if (recording) {
            FlightSummary summary = flight.getFlightSummary();
            summaries.push_back(std::move(summary));
        }

        ui->logTableWidget->blockSignals(true);
        ui->logTableWidget->setSortingEnabled(false);
        ui->logTableWidget->clearContents();
        ui->logTableWidget->setRowCount(static_cast<int>(summaries.size()));

        int row {0};
        for (const auto &summary : summaries) {
            initRow(summary, row);
            ++row;
        }

        ui->logTableWidget->setSortingEnabled(true);
        ui->logTableWidget->blockSignals(false);

    } else {
        // Clear existing entries
        ui->logTableWidget->setRowCount(0);
    }

    const int flightCount = ui->logTableWidget->rowCount();
    ui->flightCountLabel->setText(tr("%1 flights", "Number of flights selected in the logbook", flightCount).arg(flightCount));

    updateEditUi();
}

inline void LogbookWidget::initRow(const FlightSummary &summary, int row) noexcept
{
    int column {0};

    // ID
    std::unique_ptr<QTableWidgetItem> newItem = std::make_unique<QTableWidgetItem>();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newItem->setToolTip(tr("Double-click to load flight."));
    // Transfer ownership of newItem to table widget
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Title
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setToolTip(tr("Double-click to edit title."));
    newItem->setBackground(Platform::getEditableTableCellBGColor());
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Flight number
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setToolTip(tr("Double-click to edit flight number."));
    newItem->setBackground(Platform::getEditableTableCellBGColor());
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Aircraft type
    newItem = std::make_unique<QTableWidgetItem>();
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;    

    // Aircraft count
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Creation date
    newItem = std::make_unique<TableDateItem>();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Start time
    newItem = std::make_unique<TableTimeItem>();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Start location
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    // End time
    newItem = std::make_unique<TableTimeItem>();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    // End location
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Duration
    newItem = std::make_unique<TableDurationItem>();
    newItem->setToolTip(tr("Duration measured in simulation time."));
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->logTableWidget->setItem(row, column, newItem.release());
    ++column;

    updateRow(summary, row);
}

inline void LogbookWidget::updateRow(const FlightSummary &summary, int row) noexcept
{
    // ID
    auto item = ui->logTableWidget->item(row, LogbookWidgetPrivate::flightIdColumn);
    auto flightId = QVariant::fromValue(summary.flightId);
    if (summary.flightId == d->flightInMemoryId) {
        item->setIcon(QIcon(":/img/icons/aircraft-normal.png"));
    } else if (summary.flightId == Const::RecordingId) {item->setIcon(QIcon(":/img/icons/aircraft-record-normal.png"));
        // Note: alphabetical characters (a-zA-Z) will be > numerical characters (0-9),
        //       so the flight being recorded will be properly sorted in the table
        flightId = QVariant::fromValue(tr("REC"));
        item->setData(Qt::UserRole, QVariant::fromValue(Const::RecordingId));
    }
    item->setData(Qt::DisplayRole, flightId);

    // Title
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::titleColumn);
    item->setData(Qt::EditRole, summary.title);

    // User aircraft
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::userAircraftColumn);
    item->setData(Qt::DisplayRole, summary.aircraftType);

    // Aircraft count
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::aircraftCountColumn);
    item->setData(Qt::DisplayRole, QVariant::fromValue(summary.aircraftCount));

    // Flight number
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::flightNumberColumn);
    item->setData(Qt::DisplayRole, summary.flightNumber);

    // Creation date
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::recordingDateColumn);
    item->setToolTip(tr("Recording time: %1").arg(d->unit.formatTime(summary.creationDate)));
    dynamic_cast<TableDateItem *>(item)->setDate(summary.creationDate.date());

    // Start time
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::startTimeColumn);
    item->setToolTip(tr("Simulation time %1 (%2Z)").arg(d->unit.formatDateTime(summary.startSimulationLocalTime),
                                                        d->unit.formatDateTime(summary.startSimulationZuluTime)));
    dynamic_cast<TableTimeItem *>(item)->setTime(summary.startSimulationLocalTime.time());

    // Start location
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::startLocationColumn);
    item->setData(Qt::DisplayRole, summary.startLocation);

    // End time
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::endTimeColumn);
    item->setToolTip(tr("Simulation time %1 (%2Z)").arg(d->unit.formatDateTime(summary.endSimulationLocalTime),
                                                        d->unit.formatDateTime(summary.endSimulationZuluTime)));
    dynamic_cast<TableTimeItem *>(item)->setTime(summary.endSimulationLocalTime.time());

    // End location
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::endLocationColumn);
    item->setData(Qt::DisplayRole, summary.endLocation);

    // Duration
    const std::int64_t durationMSec = summary.startSimulationLocalTime.msecsTo(summary.endSimulationLocalTime);
    item = ui->logTableWidget->item(row, LogbookWidgetPrivate::durationColumn);
    dynamic_cast<TableDurationItem *>(item)->setDuration(durationMSec);
}

void LogbookWidget::updateDateSelectorUi() noexcept
{
    if (PersistenceManager::getInstance().isConnected()) {
        // Sorted by year, month, day
        std::forward_list<FlightDate> flightDates = d->logbookService->getFlightDates();
        ui->logTreeWidget->blockSignals(true);
        ui->logTreeWidget->clear();

        auto logbookItem = new QTreeWidgetItem(ui->logTreeWidget, QStringList(tr("Logbook")));

        int totalFlights = 0;
        while (!flightDates.empty()) {
            auto first = flightDates.cbegin();
            auto last = first;

            // Group by year
            int currentYear {first->year};
            int nofFlightsPerYear {0};
            while (last != flightDates.end() && last->year == currentYear) {
                nofFlightsPerYear += last->nofFlights;
                ++last;
            }
            std::forward_list<FlightDate> flightDatesByYear = {};
            flightDatesByYear.splice_after(flightDatesByYear.cbefore_begin(), flightDates, flightDates.cbefore_begin(), last);
            insertYear(logbookItem, flightDatesByYear, nofFlightsPerYear);

            totalFlights += nofFlightsPerYear;
        }
        logbookItem->setData(::NofFlightsColumn, Qt::DisplayRole, totalFlights);

        // Adjust column size when all items are expanded
        ui->logTreeWidget->expandAll();
        ui->logTreeWidget->resizeColumnToContents(::DateColumn);
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
    const std::int64_t selectedFlightId = getSelectedFlightId();
    ui->loadPushButton->setEnabled(!active && selectedFlightId != Const::InvalidId);
    ui->deletePushButton->setEnabled(!active && selectedFlightId != Const::InvalidId);
}

void LogbookWidget::frenchConnection() noexcept
{
    // Logbook
    const auto &logbook = Logbook::getInstance();
    connect(&PersistenceManager::getInstance(), &PersistenceManager::connectionChanged,
            this, &LogbookWidget::updateUi);

    // Flight
    const auto &flight = logbook.getCurrentFlight();
    connect(&flight, &Flight::flightStored,
            this, &LogbookWidget::updateUi);
    connect(&flight, &Flight::flightRestored,
            this, &LogbookWidget::updateAircraftIcons);
    connect(&flight, &Flight::aircraftStored,
            this, &LogbookWidget::updateUi);
    connect(&flight, &Flight::flightNumberChanged,
            this, &LogbookWidget::onFlightNumberChanged);
    connect(&flight, &Flight::aircraftInfoChanged,
            this, &LogbookWidget::onAircraftInfoChanged);
    connect(&flight, &Flight::titleChanged,
            this, &LogbookWidget::onFlightTitleChanged);
    connect(&flight, &Flight::cleared,
            this, &LogbookWidget::updateAircraftIcons);

    // Connection
    auto &skyConnectManager = SkyConnectManager::getInstance();
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
    connect(ui->resetFilterPushButton, &QPushButton::clicked,
            this, &LogbookWidget::resetFilter);

    // Date selection
    connect(ui->logTreeWidget, &QTreeWidget::itemClicked,
            this, &LogbookWidget::onDateItemClicked);

    // Module settings
    connect(&d->moduleSettings, &ModuleBaseSettings::changed,
            this, &LogbookWidget::onModuleSettingsChanged);
}

inline void LogbookWidget::insertYear(QTreeWidgetItem *parent, std::forward_list<FlightDate> &flightDatesByYear, int nofFlightsPerYear) noexcept
{
    const int year = flightDatesByYear.cbegin()->year;
    auto yearItem = new QTreeWidgetItem(parent, {QString::number(year), QString::number(nofFlightsPerYear)});
    yearItem->setData(::DateColumn, Qt::UserRole, year);
    while (!flightDatesByYear.empty()) {
        auto first = flightDatesByYear.cbegin();
        auto last = first;

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
    auto monthItem = new QTreeWidgetItem(parent, {d->unit.formatMonth(month), QString::number(nofFlightsPerMonth)});
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
        auto dayItem = new QTreeWidgetItem(parent, {QString::number(dayOfMonth), QString::number(nofFlights)});
        dayItem->setData(DateColumn, Qt::UserRole, dayOfMonth);
    }
}

inline void LogbookWidget::updateSelectionDateRange(QTreeWidgetItem *item) const noexcept
{
    d->moduleSettings.blockSignals(true);
    const QTreeWidgetItem *parent = item->parent();
    if (parent != nullptr) {
        const QTreeWidgetItem *parent1 = parent->parent();
        if (parent1 != nullptr) {
            const QTreeWidgetItem *parent2 = parent1->parent();
            if (parent2 != nullptr) {
                // Item: day selected
                const int year = parent1->data(::DateColumn, Qt::UserRole).toInt();
                const int month = parent->data(::DateColumn, Qt::UserRole).toInt();
                const int day = item->data(::DateColumn, Qt::UserRole).toInt();
                QDate fromDate {year, month, day};
                QDate toDate {fromDate.addDays(1)};
                d->moduleSettings.setFromDate(fromDate);
                d->moduleSettings.setToDate(toDate);
            } else {
                // Item: month selected
                const int year = parent->data(::DateColumn, Qt::UserRole).toInt();
                const int month = item->data(::DateColumn, Qt::UserRole).toInt();
                QDate fromDate {year, month, 1};
                const int daysInMonth = fromDate.daysInMonth();
                d->moduleSettings.setFromDate(fromDate);
                QDate toDate {year, month, daysInMonth};
                d->moduleSettings.setToDate(toDate);
            }
        } else {
            // Item: year selected
            const int year = item->data(::DateColumn, Qt::UserRole).toInt();
            QDate fromDate {year, 1, 1};
            d->moduleSettings.setFromDate(fromDate);
            QDate toDate {year, 12, 31};
            d->moduleSettings.setToDate(toDate);
        }
    } else {
        // Item: Logbook selected (show all entries)
        d->moduleSettings.setFromDate(FlightSelector::MinDate);
        d->moduleSettings.setToDate(FlightSelector::MaxDate);
    }
    d->moduleSettings.blockSignals(false);
    emit d->moduleSettings.changed();
}

int LogbookWidget::getSelectedRow() const noexcept
{
    int selectedRow {::InvalidRow};
    const auto select = ui->logTableWidget->selectionModel();
    const auto modelIndices = select->selectedRows(LogbookWidgetPrivate::flightIdColumn);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        selectedRow = modelIndex.row();
    }
    return selectedRow;
}

std::int64_t LogbookWidget::getSelectedFlightId() const noexcept
{
    std::int64_t selectedFlightId {Const::InvalidId};
    const auto selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        bool ok {false};
        selectedFlightId = ui->logTableWidget->item(selectedRow, LogbookWidgetPrivate::flightIdColumn)->data(Qt::DisplayRole).toLongLong(&ok);
        if (!ok) {
            // Flight is being recorded (no valid ID yet)
            selectedFlightId = Const::RecordingId;
        }
    }
    return selectedFlightId;
}

inline bool LogbookWidget::isMatch(QTableWidgetItem *flightIdItem, std::int64_t flightId) const noexcept
{
    return flightId != Const::RecordingId && flightIdItem->data(Qt::DisplayRole).toLongLong() == flightId ||
           flightIdItem->data(Qt::UserRole).toLongLong() == flightId;
}

// PRIVATE SLOTS

void LogbookWidget::onRecordingStarted() noexcept
{
    if (SkyConnectManager::getInstance().isInRecordingState()) {
        const auto &flight = Logbook::getInstance().getCurrentFlight();
        FlightSummary summary = flight.getFlightSummary();
        ui->logTableWidget->blockSignals(true);
        ui->logTableWidget->setSortingEnabled(false);
        const int row = ui->logTableWidget->rowCount();
        ui->logTableWidget->insertRow(row);
        initRow(summary, row);
        const auto item = ui->logTableWidget->item(row, LogbookWidgetPrivate::flightIdColumn);
        // Give the repaint event a chance to get processed before scrolling
        // to make the item visible
        QTimer::singleShot(0, this, [this, item]() {ui->logTableWidget->scrollToItem(item);});
        updateAircraftIcons();
        ui->logTableWidget->setSortingEnabled(true);
        ui->logTableWidget->blockSignals(false);
    }
}

void LogbookWidget::updateUi() noexcept
{
    updateTable();
    updateDateSelectorUi();
}

void LogbookWidget::updateAircraftIcons() noexcept
{
    const auto &flight = Logbook::getInstance().getCurrentFlight();
    const auto flightInMemoryId = flight.getId();

    for (int row = 0; row < ui->logTableWidget->rowCount(); ++row) {
        const auto item = ui->logTableWidget->item(row, LogbookWidgetPrivate::flightIdColumn);
        if (item->data(Qt::DisplayRole).toLongLong() == flightInMemoryId) {
            item->setIcon(QIcon(":/img/icons/aircraft-normal.png"));
        } else if (item->data(Qt::UserRole).toLongLong() == Const::RecordingId) {
            item->setIcon(QIcon(":/img/icons/aircraft-record-normal.png"));
        } else {
            item->setIcon(QIcon());
        }
    }
}

void LogbookWidget::onFlightTitleChanged(std::int64_t flightId, const QString &title) noexcept
{
    for (int row = 0; row < ui->logTableWidget->rowCount(); ++row) {
        QTableWidgetItem *flightIdItem = ui->logTableWidget->item(row, d->flightIdColumn);
        if (isMatch(flightIdItem, flightId)) {
            QTableWidgetItem *titleItem = ui->logTableWidget->item(row, d->titleColumn);
            titleItem->setData(Qt::EditRole, title);
            break;
        }
    }
}

void LogbookWidget::onFlightNumberChanged(std::int64_t flightId, const QString &flightNumber) noexcept
{
    for (int row = 0; row < ui->logTableWidget->rowCount(); ++row) {
        QTableWidgetItem *flightIdItem = ui->logTableWidget->item(row, d->flightIdColumn);
        if (isMatch(flightIdItem, flightId)) {
            QTableWidgetItem *flightNumberItem = ui->logTableWidget->item(row, d->flightNumberColumn);
            flightNumberItem->setData(Qt::DisplayRole, flightNumber);
            break;
        }
    }
}

void LogbookWidget::onAircraftInfoChanged(const Aircraft &aircraft) noexcept
{
    const std::int64_t flightId = Logbook::getInstance().getCurrentFlight().getId();
    for (int row = 0; row < ui->logTableWidget->rowCount(); ++row) {
        QTableWidgetItem *flightIdItem = ui->logTableWidget->item(row, d->flightIdColumn);
        if (isMatch(flightIdItem, flightId)) {
            const auto &aircraftInfo = aircraft.getAircraftInfo();
            QTableWidgetItem *userAircraftItem = ui->logTableWidget->item(row, d->userAircraftColumn);
            userAircraftItem->setData(Qt::DisplayRole, aircraftInfo.aircraftType.type);
            break;
        }
    }
}

void LogbookWidget::loadFlight() noexcept
{
    if (!SkyConnectManager::getInstance().isInRecordingState()) {
        const std::int64_t selectedFlightId = getSelectedFlightId();
        if (selectedFlightId != Const::InvalidId) {
            const bool ok = d->flightService->restoreFlight(selectedFlightId, Logbook::getInstance().getCurrentFlight());
            if (!ok) {
                QMessageBox::critical(this, tr("Read Error"), tr("The flight %1 could not be read from the logbook.").arg(selectedFlightId));
            }
        }
    }
}

void LogbookWidget::deleteFlight() noexcept
{
    const std::int64_t selectedFlightId = getSelectedFlightId();
    if (selectedFlightId != Const::InvalidId) {

        auto &settings = Settings::getInstance();
        bool doDelete {true};
        if (settings.isDeleteFlightConfirmationEnabled()) {
            auto messageBox = std::make_unique<QMessageBox>(this);
            const auto dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), messageBox.get());

            messageBox->setWindowTitle(tr("Delete Flight"));
            messageBox->setText(tr("The flight %1 is about to be deleted. Deletion cannot be undone.").arg(selectedFlightId));
            messageBox->setInformativeText(tr("Do you want to delete the flight?"));
            const auto deleteButton = messageBox->addButton(tr("&Delete"), QMessageBox::AcceptRole);
            const auto keepButton = messageBox->addButton(tr("&Keep"), QMessageBox::RejectRole);
            messageBox->setDefaultButton(keepButton);
            messageBox->setCheckBox(dontAskAgainCheckBox);
            messageBox->setIcon(QMessageBox::Icon::Question);

            messageBox->exec();
            doDelete = messageBox->clickedButton() == deleteButton;
            settings.setDeleteFlightConfirmationEnabled(!dontAskAgainCheckBox->isChecked());
        }

        if (doDelete) {
            const auto lastSelectedRow = getSelectedRow();
            d->flightService->deleteById(selectedFlightId);
            updateUi();
            const auto selectedRow = std::min(lastSelectedRow, ui->logTableWidget->rowCount() - 1);
            ui->logTableWidget->selectRow(selectedRow);
            ui->logTableWidget->setFocus();
        }
    }
}

void LogbookWidget::onSearchTextChanged() noexcept
{
    d->searchTimer->start();
}

void LogbookWidget::searchText() noexcept
{
    d->moduleSettings.setSearchKeyword(ui->searchLineEdit->text());
    updateTable();
}

void LogbookWidget::onSelectionChanged() noexcept
{
    updateEditUi();
}

void LogbookWidget::onCellSelected(int row, int column) noexcept
{
    if (column == d->titleColumn) {
        const auto item = ui->logTableWidget->item(row, column);
        ui->logTableWidget->editItem(item);
    } else if (column == d->flightNumberColumn) {
        const auto item = ui->logTableWidget->item(row, column);
        ui->logTableWidget->editItem(item);
    } else {
        loadFlight();
    }
}

void LogbookWidget::onCellChanged(int row, int column) noexcept
{
    const auto item = ui->logTableWidget->item(row, column);
    const QString value = item->data(Qt::EditRole).toString();
    auto &flight = Logbook::getInstance().getCurrentFlight();
    const std::int64_t selectedFlightId = getSelectedFlightId();

    if (column == d->titleColumn) {
        if (flight.getId() == selectedFlightId) {
            // Update the current flight, if in memory
            d->flightService->updateTitle(flight, value);
        } else {
            d->flightService->updateTitle(selectedFlightId, value);
        }
    } else if (column == d->flightNumberColumn) {
        if (flight.getId() == selectedFlightId) {
            // Update the current flight, if in memory
            d->flightService->updateFlightNumber(flight, value);
        } else {
            d->flightService->updateFlightNumber(selectedFlightId, value);
        }
    }
}

void LogbookWidget::onDateItemClicked(QTreeWidgetItem *item) noexcept
{
    updateSelectionDateRange(item);
}

void LogbookWidget::filterByFormationFlights(bool checked) noexcept
{
    d->moduleSettings.setFormation(checked);
}

void LogbookWidget::filterByEngineType([[maybe_unused]] int index) noexcept
{
    d->moduleSettings.setEngineType(static_cast<SimType::EngineType>(ui->engineTypeComboBox->currentData().toInt()));
}

void LogbookWidget::filterByDuration([[maybe_unused]] int index) noexcept
{
    int minimumDurationMinutes {0};
    Duration duration = static_cast<Duration>(ui->durationComboBox->currentData().toInt());
    switch (duration) {
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
    d->moduleSettings.setMinimumDurationMinutes(minimumDurationMinutes);
}

void LogbookWidget::resetFilter() noexcept
{
    d->moduleSettings.resetFilter();
    ui->searchLineEdit->clear();
}

void LogbookWidget::onTableLayoutChanged() noexcept
{
    QByteArray tableState = ui->logTableWidget->horizontalHeader()->saveState();
    d->moduleSettings.setLogbookTableState(std::move(tableState));
}

void LogbookWidget::onModuleSettingsChanged() noexcept
{
    ui->searchLineEdit->blockSignals(true);
    ui->searchLineEdit->setText(d->moduleSettings.getSearchKeyword());
    ui->searchLineEdit->blockSignals(false);

    ui->formationCheckBox->blockSignals(true);
    ui->formationCheckBox->setChecked(d->moduleSettings.hasFormation());
    ui->formationCheckBox->blockSignals(false);

    Duration duration {Duration::All};
    const int minimumDurationMinutes = d->moduleSettings.getMinimumDurationMinutes();
    if (minimumDurationMinutes < 2) {
        duration = Duration::All;
    } else if (minimumDurationMinutes < 5) {
        duration = Duration::TwoMinutes;
    } else if (minimumDurationMinutes < 10) {
        duration = Duration::FiveMinutes;
    } else if (minimumDurationMinutes < 15) {
        duration = Duration::TenMinutes;
    } else if (minimumDurationMinutes < 30) {
        duration = Duration::Fifteen;
    } else if (minimumDurationMinutes < 60) {
        duration = Duration::ThirtyMinutes;
    } else if (minimumDurationMinutes < 120) {
        duration = Duration::OneHour;
    } else if (minimumDurationMinutes < 180) {
        duration = Duration::TwoHours;
    } else if (minimumDurationMinutes < 240) {
        duration = Duration::ThreeHours;
    } else {
        duration = Duration::FourHours;
    }
    ui->durationComboBox->blockSignals(true);
    for (int index = 0; index < ui->durationComboBox->count(); ++index) {
        if (static_cast<Duration>(ui->durationComboBox->itemData(index).toInt()) == duration) {
            ui->durationComboBox->setCurrentIndex(index);
            break;
        }
    }
    ui->durationComboBox->blockSignals(false);

    ui->engineTypeComboBox->blockSignals(true);
    for (int index = 0; index < ui->engineTypeComboBox->count(); ++index) {
        if (static_cast<SimType::EngineType>(ui->engineTypeComboBox->itemData(index).toInt()) == d->moduleSettings.getEngineType()) {
            ui->engineTypeComboBox->setCurrentIndex(index);
            break;
        }
    }
    ui->engineTypeComboBox->blockSignals(false);

    updateTable();
}
