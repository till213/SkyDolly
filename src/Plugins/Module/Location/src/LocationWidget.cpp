/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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
#include <cstdint>

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStringList>
#include <QByteArray>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QKeyEvent>
#include <QKeySequence>
#include <QClipboard>
#include <QRegularExpression>
#include <QApplication>
#include <QTimer>
#include <QDate>
#include <QTime>

#include <GeographicLib/DMS.hpp>

#include <Kernel/Const.h>
#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Kernel/Unit.h>
#include <Kernel/PositionParser.h>
#include <Model/Logbook.h>
#include <Persistence/PersistenceManager.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/LocationSelector.h>
#include <Persistence/Service/LocationService.h>
#include <Persistence/Service/EnumerationService.h>
#include <Widget/FocusPlainTextEdit.h>
#include <Widget/EnumerationWidgetItem.h>
#include <Widget/UnitWidgetItem.h>
#include <Widget/LinkedOptionGroup.h>
#include <Widget/TableCheckableItem.h>
#include <PluginManager/SkyConnectManager.h>
#include "LocationWidget.h"
#include "EnumerationItemDelegate.h"
#include "DateItemDelegate.h"
#include "TimeItemDelegate.h"
#include "PositionWidgetItem.h"
#include "LocationSettings.h"
#include "ui_LocationWidget.h"

namespace
{
    constexpr int InvalidRow {-1};
    constexpr int InvalidColumn {-1};

    constexpr double DefaultPitch {0.0};
    constexpr double MinimumPitch {-90.0};
    constexpr double MaximumPitch {90.0};
    constexpr double DefaultBank {0.0};
    constexpr double MinimumBank {-180.0};
    constexpr double MaximumBank {180.0};
    constexpr double DefaultHeading {0.0};
    constexpr double MinimumHeading {0.0};
    constexpr double MaximumHeading {360.0};

    constexpr int SearchTimeoutMSec {200};
}

struct LocationWidgetPrivate
{
    LocationWidgetPrivate(LocationSettings &moduleSettings) noexcept
        : moduleSettings(moduleSettings)
    {
        if (typeEnumeration.count() == 0) {
            typeEnumeration = enumerationService->getEnumerationByName(EnumerationService::LocationType);
        }
        if (categoryEnumeration.count() == 0) {
            categoryEnumeration = enumerationService->getEnumerationByName(EnumerationService::LocationCategory);
        }
        if (countryEnumeration.count() == 0) {
            countryEnumeration = enumerationService->getEnumerationByName(EnumerationService::Country);
        }
        searchTimer->setSingleShot(true);
        searchTimer->setInterval(::SearchTimeoutMSec);
    }

    LocationSettings &moduleSettings;
    std::unique_ptr<QTimer> searchTimer {std::make_unique<QTimer>()};
    std::unique_ptr<LocationService> locationService {std::make_unique<LocationService>()};
    std::unique_ptr<EnumerationService> enumerationService {std::make_unique<EnumerationService>()};
    std::unique_ptr<EnumerationItemDelegate> locationCategoryDelegate {std::make_unique<EnumerationItemDelegate>(EnumerationService::LocationCategory)};
    std::unique_ptr<EnumerationItemDelegate> countryDelegate {std::make_unique<EnumerationItemDelegate>(EnumerationService::Country)};
    std::unique_ptr<DateItemDelegate> dateItemDelegate {std::make_unique<DateItemDelegate>()};
    std::unique_ptr<TimeItemDelegate> timeItemDelegate {std::make_unique<TimeItemDelegate>()};

    const std::int64_t PresetLocationTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypePresetSymId).id()};
    const std::int64_t UserLocationTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeUserSymId).id()};
    const std::int64_t ImportLocationTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeImportSymId).id()};
    const std::int64_t NoneLocationCategoryId {PersistedEnumerationItem(EnumerationService::LocationCategory, EnumerationService::LocationCategoryNoneSymId).id()};
    const std::int64_t WorldCountryId {PersistedEnumerationItem(EnumerationService::Country, EnumerationService::CountryWorldSymId).id()};

    Unit unit;

    static inline int idColumn {InvalidColumn};
    static inline int titleColumn {InvalidColumn};
    static inline int descriptionColumn {InvalidColumn};
    static inline int localSimulationDateColumn {InvalidColumn};
    static inline int localSimulationTimeColumn {InvalidColumn};
    static inline int typeColumn {InvalidColumn};
    static inline int categoryColumn {InvalidColumn};
    static inline int countryColumn {InvalidColumn};
    static inline int identifierColumn {InvalidColumn};
    static inline int positionColumn {InvalidColumn};
    static inline int altitudeColumn {InvalidColumn};
    static inline int pitchColumn {InvalidColumn};
    static inline int bankColumn {InvalidColumn};
    static inline int trueHeadingColumn {InvalidColumn};
    static inline int indicatedAirspeedColumn {InvalidColumn};
    static inline int onGroundColumn {InvalidColumn};
    static inline int engineColumn {InvalidColumn};

    static inline Enumeration typeEnumeration;
    static inline Enumeration categoryEnumeration;
    static inline Enumeration countryEnumeration;
};

// PUBLIC

LocationWidget::LocationWidget(LocationSettings &moduleSettings, QWidget *parent) noexcept
    : QWidget {parent},
      ui {std::make_unique<Ui::LocationWidget>()},
      d {std::make_unique<LocationWidgetPrivate>(moduleSettings)}
{
    ui->setupUi(this);
    initUi();
    // The location table is updated once the plugin settings are restored (initiated by LocationPlugin)
    updateEditUi();
    updateInfoUi();
    frenchConnection();
}

LocationWidget::~LocationWidget() = default;

void LocationWidget::addUserLocation(double latitude, double longitude)
{
    Location location;
    location.latitude = latitude;
    location.longitude = longitude;
    location.altitude = ui->defaultAltitudeSpinBox->value();
    location.indicatedAirspeed = ui->defaultIndicatedAirspeedSpinBox->value();
    location.onGround = ui->defaultOnGroundCheckBox->isChecked();
    location.engineEventId = ui->defaultEngineEventComboBox->getCurrentId();
    addLocation(location);
}

void LocationWidget::addLocation(Location newLocation)
{
    Location location {std::move(newLocation)};
    if (location.typeId == Const::InvalidId) {
        location.typeId = d->UserLocationTypeId;
    }
    if (location.categoryId == Const::InvalidId) {
        location.categoryId = d->NoneLocationCategoryId;
    }
    if (location.countryId == Const::InvalidId) {
        location.countryId = d->WorldCountryId;
    }
    if (location.engineEventId == Const::InvalidId) {
        location.engineEventId = ui->defaultEngineEventComboBox->getCurrentId();
    }    
    if (d->locationService->store(location)) {
        // Make sure that user locations are visible - this will also update
        // the table rows (stored location will already be added)
        resetFilter();

        const int row = getRowById(location.id);
        if (row != ::InvalidRow) {
            ui->locationTableWidget->setFocus();
            ui->locationTableWidget->selectRow(row);
            const auto item = ui->locationTableWidget->item(row, LocationWidgetPrivate::idColumn);
            // Give the repaint event a chance to get processed before scrolling
            // to make the item visible
            QTimer::singleShot(0, this, [this, item]() {ui->locationTableWidget->scrollToItem(item);});
        }
    }
}

void LocationWidget::updateLocation(const Location &location)
{
    const auto selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location selectedLocation = getLocationByRow(selectedRow);

        selectedLocation.latitude = location.latitude;
        selectedLocation.longitude = location.longitude;
        selectedLocation.altitude = location.altitude;
        selectedLocation.pitch = location.pitch;
        selectedLocation.bank = location.bank;
        selectedLocation.trueHeading = location.trueHeading;
        selectedLocation.indicatedAirspeed = location.indicatedAirspeed;
        selectedLocation.onGround = location.onGround;

        if (d->locationService->update(selectedLocation)) {
            ui->locationTableWidget->setSortingEnabled(false);
            ui->locationTableWidget->blockSignals(true);            
            updateRow(selectedLocation, selectedRow);
            ui->locationTableWidget->blockSignals(false);
            ui->locationTableWidget->setSortingEnabled(true);
            updateInfoUi();
        }
    }
}

// PROTECTED

void LocationWidget::showEvent([[maybe_unused]] QShowEvent *event) noexcept
{
    QWidget::showEvent(event);

    QByteArray tableState = d->moduleSettings.getLocationTableState();
    if (!tableState.isEmpty()) {
        ui->locationTableWidget->horizontalHeader()->blockSignals(true);
        ui->locationTableWidget->horizontalHeader()->restoreState(tableState);
        ui->locationTableWidget->horizontalHeader()->blockSignals(false);
    } else {
        ui->locationTableWidget->resizeColumnsToContents();
    }
    // Sort with the current sort section and order
    ui->locationTableWidget->setSortingEnabled(true);

    // Wait until table widget columns (e.g. visibility) have been fully initialised
    connect(ui->locationTableWidget->horizontalHeader(), &QHeaderView::sectionMoved,
            this, &LocationWidget::onTableLayoutChanged);
    connect(ui->locationTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
            this, &LocationWidget::onTableLayoutChanged);
    connect(ui->locationTableWidget->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, &LocationWidget::onTableLayoutChanged);
}

void LocationWidget::keyPressEvent(QKeyEvent *event) noexcept
{
    if (event->matches(QKeySequence::Paste)) {
        tryPasteLocation();
    } else {
        QWidget::keyPressEvent(event);
    }
}

// PRIVATE

void LocationWidget::initUi() noexcept
{
    // Search
    ui->categoryComboBox->setEnumerationName(EnumerationService::LocationCategory);
    ui->categoryComboBox->setEditable(true);
    ui->categoryComboBox->setCurrentId(d->NoneLocationCategoryId);
    ui->countryComboBox->setEnumerationName(EnumerationService::Country);
    ui->countryComboBox->setEditable(true);
    ui->countryComboBox->setCurrentId(d->WorldCountryId);
    ui->searchLineEdit->setPlaceholderText(tr("Title, description, identifier"));
    // Make sure that shortcuts are initially accepted
    ui->searchLineEdit->clearFocus();
    ui->searchLineEdit->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    ui->searchLineEdit->setClearButtonEnabled(true);

    // Type
    ui->typeOptionGroup->addOption(tr("Preset"), QVariant::fromValue(d->PresetLocationTypeId), tr("Show preset locations."));
    ui->typeOptionGroup->addOption(tr("User"), QVariant::fromValue(d->UserLocationTypeId), tr("Show user locations."));
    ui->typeOptionGroup->addOption(tr("Import"), QVariant::fromValue(d->ImportLocationTypeId), tr("Show imported locations."));

    // Table
    const QStringList headers {
        tr("ID"), tr("Title"), tr("Description"), tr("Type"), tr("Category"), tr("Country"), tr("Identifer"),
        tr("Position"), tr("Altitude"), tr("Pitch"), tr("Bank"), tr("True Heading"), tr("Indicated Airspeed"),
        tr("Local Date"), tr("Local Time"), tr("On Ground"), tr("Engine")
    };
    LocationWidgetPrivate::idColumn = static_cast<int>(headers.indexOf(tr("ID")));
    LocationWidgetPrivate::titleColumn = static_cast<int>(headers.indexOf(tr("Title")));
    LocationWidgetPrivate::descriptionColumn = static_cast<int>(headers.indexOf(tr("Description")));
    LocationWidgetPrivate::typeColumn = static_cast<int>(headers.indexOf(tr("Type")));
    LocationWidgetPrivate::categoryColumn = static_cast<int>(headers.indexOf(tr("Category")));
    LocationWidgetPrivate::countryColumn = static_cast<int>(headers.indexOf(tr("Country")));
    LocationWidgetPrivate::identifierColumn = static_cast<int>(headers.indexOf(tr("Identifer")));
    LocationWidgetPrivate::positionColumn = static_cast<int>(headers.indexOf(tr("Position")));
    LocationWidgetPrivate::altitudeColumn = static_cast<int>(headers.indexOf(tr("Altitude")));
    LocationWidgetPrivate::pitchColumn = static_cast<int>(headers.indexOf(tr("Pitch")));
    LocationWidgetPrivate::bankColumn = static_cast<int>(headers.indexOf(tr("Bank")));
    LocationWidgetPrivate::trueHeadingColumn = static_cast<int>(headers.indexOf(tr("True Heading")));
    LocationWidgetPrivate::indicatedAirspeedColumn = static_cast<int>(headers.indexOf(tr("Indicated Airspeed")));
    LocationWidgetPrivate::localSimulationDateColumn = static_cast<int>(headers.indexOf(tr("Local Date")));
    LocationWidgetPrivate::localSimulationTimeColumn = static_cast<int>(headers.indexOf(tr("Local Time")));
    LocationWidgetPrivate::onGroundColumn = static_cast<int>(headers.indexOf(tr("On Ground")));
    LocationWidgetPrivate::engineColumn = static_cast<int>(headers.indexOf(tr("Engine")));

    ui->locationTableWidget->setColumnCount(static_cast<int>(headers.count()));
    ui->locationTableWidget->setHorizontalHeaderLabels(headers);
    ui->locationTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->locationTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->locationTableWidget->verticalHeader()->hide();
    ui->locationTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->locationTableWidget->sortByColumn(LocationWidgetPrivate::idColumn, Qt::SortOrder::DescendingOrder);
    ui->locationTableWidget->horizontalHeader()->setSectionsMovable(true);
    ui->locationTableWidget->setAlternatingRowColors(true);
    ui->locationTableWidget->setColumnHidden(LocationWidgetPrivate::typeColumn, true);
    ui->locationTableWidget->setColumnHidden(LocationWidgetPrivate::descriptionColumn, true);
    ui->locationTableWidget->setColumnHidden(LocationWidgetPrivate::pitchColumn, true);
    ui->locationTableWidget->setColumnHidden(LocationWidgetPrivate::bankColumn, true);
    ui->locationTableWidget->setColumnHidden(LocationWidgetPrivate::trueHeadingColumn, true);
    ui->locationTableWidget->setColumnHidden(LocationWidgetPrivate::indicatedAirspeedColumn, true);
    ui->locationTableWidget->setColumnHidden(LocationWidgetPrivate::engineColumn, true);
    ui->locationTableWidget->setItemDelegateForColumn(LocationWidgetPrivate::categoryColumn, d->locationCategoryDelegate.get());
    ui->locationTableWidget->setItemDelegateForColumn(LocationWidgetPrivate::countryColumn, d->countryDelegate.get());
    ui->locationTableWidget->setItemDelegateForColumn(LocationWidgetPrivate::localSimulationDateColumn, d->dateItemDelegate.get());
    ui->locationTableWidget->setItemDelegateForColumn(LocationWidgetPrivate::localSimulationTimeColumn, d->timeItemDelegate.get());

    // Date and time
    ui->dateComboBox->addItem(tr("Today"), Enum::underly(LocationSettings::DateSelection::Today));
    ui->dateComboBox->addItem(tr("Date"), Enum::underly(LocationSettings::DateSelection::Date));
    ui->dateComboBox->addItem(tr("Location date & time"), Enum::underly(LocationSettings::DateSelection::LocationDateTime));

    ui->timeComboBox->addItem(tr("Now"), Enum::underly(LocationSettings::TimeSelection::Now));
    ui->timeComboBox->addItem(tr("Morning"), Enum::underly(LocationSettings::TimeSelection::Morning));
    ui->timeComboBox->addItem(tr("Noon"), Enum::underly(LocationSettings::TimeSelection::Noon));
    ui->timeComboBox->addItem(tr("Afternoon"), Enum::underly(LocationSettings::TimeSelection::Afternoon));
    ui->timeComboBox->addItem(tr("Evening"), Enum::underly(LocationSettings::TimeSelection::Evening));
    ui->timeComboBox->addItem(tr("Night"), Enum::underly(LocationSettings::TimeSelection::Night));
    ui->timeComboBox->addItem(tr("Midnight"), Enum::underly(LocationSettings::TimeSelection::Midnight));
    ui->timeComboBox->addItem(tr("Sunrise"), Enum::underly(LocationSettings::TimeSelection::Sunrise));
    ui->timeComboBox->addItem(tr("Sunset"), Enum::underly(LocationSettings::TimeSelection::Sunset));

    // Default "Delete" key deletes aircraft
    ui->deletePushButton->setShortcut(QKeySequence::Delete);

    ui->defaultAltitudeSpinBox->setMinimum(Const::MinimumAltitude);
    ui->defaultAltitudeSpinBox->setMaximum(Const::MaximumAltitude);;
    ui->defaultAltitudeSpinBox->setSuffix(tr(" feet"));
    ui->defaultIndicatedAirspeedSpinBox->setMinimum(Const::MinimumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setMaximum(Const::MaximumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setSuffix(tr(" knots"));
    ui->defaultEngineEventComboBox->setEnumerationName(EnumerationService::EngineEvent);

    ui->pitchSpinBox->setMinimum(::MinimumPitch);
    ui->pitchSpinBox->setMaximum(::MaximumPitch);
    ui->pitchSpinBox->setSuffix("°");
    ui->bankSpinBox->setMinimum(::MinimumBank);
    ui->bankSpinBox->setMaximum(::MaximumBank);
    ui->bankSpinBox->setSuffix("°");
    ui->trueHeadingSpinBox->setMinimum(::MinimumHeading);
    ui->trueHeadingSpinBox->setMaximum(::MaximumHeading);
    ui->trueHeadingSpinBox->setSuffix("°");
    ui->indicatedAirspeedSpinBox->setMinimum(Const::MinimumIndicatedAirspeed);
    ui->indicatedAirspeedSpinBox->setMaximum(Const::MaximumIndicatedAirspeed);
    ui->indicatedAirspeedSpinBox->setSuffix(tr(" knots"));
    ui->engineEventComboBox->setEnumerationName(EnumerationService::EngineEvent);

    const int infoGroupBoxHeight = ui->informationGroupBox->minimumHeight();
    ui->splitter->setSizes({height() - infoGroupBoxHeight, infoGroupBoxHeight});
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 0);

    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void LocationWidget::frenchConnection() noexcept
{
    // Logbook
    connect(&PersistenceManager::getInstance(), &PersistenceManager::connectionChanged,
            this, &LocationWidget::updateUi);

    // Connection
    auto &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &LocationWidget::updateEditUi);

    // Search
    connect(ui->categoryComboBox, &EnumerationComboBox::currentIndexChanged,
            this, &LocationWidget::onCategoryChanged);
    connect(ui->countryComboBox, &EnumerationComboBox::currentIndexChanged,
            this, &LocationWidget::onCountryChanged);
    connect(ui->searchLineEdit, &QLineEdit::textChanged,
            this, &LocationWidget::onSearchTextChanged);
    connect(d->searchTimer.get(), &QTimer::timeout,
            this, &LocationWidget::searchText);
    connect(ui->typeOptionGroup, &LinkedOptionGroup::optionToggled,
            this, &LocationWidget::onTypeOptionToggled);
    connect(ui->resetFilterPushButton, &QPushButton::clicked,
            this, &LocationWidget::resetFilter);
    connect(ui->resetDefaultsPushButton, &QPushButton::clicked,
            this, &LocationWidget::resetDefaultValues);

    // Persistence
    const auto &persistenceManager = PersistenceManager::getInstance();
    connect(&persistenceManager, &PersistenceManager::locationsImported,
            this, &LocationWidget::updateUi);

    // Location table
    connect(ui->locationTableWidget, &QTableWidget::cellDoubleClicked,
            this, &LocationWidget::onCellSelected);
    connect(ui->locationTableWidget, &QTableWidget::cellChanged,
            this, &LocationWidget::onCellChanged);
    connect(ui->locationTableWidget, &QTableWidget::itemSelectionChanged,
            this, &LocationWidget::onSelectionChanged);
    connect(ui->addPushButton, &QPushButton::clicked,
            this, &LocationWidget::onAddLocation);
    connect(ui->updatePushButton, &QPushButton::clicked,
            this, &LocationWidget::onUpdateLocation);
    connect(ui->capturePushButton, &QPushButton::clicked,
            this, &LocationWidget::onCaptureLocation);
    connect(ui->teleportPushButton, &QPushButton::clicked,
            this, &LocationWidget::onTeleportToSelectedLocation);
    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &LocationWidget::onDeleteLocation);

    // Information group
    connect(ui->descriptionPlainTextEdit, &FocusPlainTextEdit::focusLost,
            this, &LocationWidget::onDescriptionChanged);
    connect(ui->pitchSpinBox, &QDoubleSpinBox::valueChanged,
            this, &LocationWidget::onPitchChanged);
    connect(ui->bankSpinBox, &QDoubleSpinBox::valueChanged,
            this, &LocationWidget::onBankChanged);
    connect(ui->trueHeadingSpinBox, &QDoubleSpinBox::valueChanged,
            this, &LocationWidget::onHeadingChanged);
    connect(ui->indicatedAirspeedSpinBox, &QSpinBox::valueChanged,
            this, &LocationWidget::onIndicatedAirspeedChanged);
    connect(ui->engineEventComboBox, &EnumerationComboBox::currentIndexChanged,
            this, &LocationWidget::onEngineEventChanged);

    // Date and time
    connect(ui->dateComboBox, &QComboBox::currentIndexChanged,
            this, &LocationWidget::onDateSelected);
    connect(ui->dateEdit, &QDateEdit::userDateChanged,
            this, &LocationWidget::onDateChanged);
    connect(ui->timeComboBox, &QComboBox::currentIndexChanged,
            this, &LocationWidget::onTimeSelected);

    // Default values group
    connect(ui->defaultAltitudeSpinBox, &QSpinBox::valueChanged,
            this, &LocationWidget::onDefaultAltitudeChanged);
    connect(ui->defaultIndicatedAirspeedSpinBox, &QSpinBox::valueChanged,
            this, &LocationWidget::onDefaultIndicatedAirspeedChanged);
    connect(ui->defaultEngineEventComboBox, &EnumerationComboBox::currentIndexChanged,
            this, &LocationWidget::onDefaultEngineEventChanged);
    connect(ui->defaultOnGroundCheckBox, &QCheckBox::toggled,
            this, &LocationWidget::onDefaultOnGroundChanged);

    // Module settings
    connect(&d->moduleSettings, &ModuleBaseSettings::changed,
            this, &LocationWidget::onModuleSettingsChanged);
}

void LocationWidget::updateInfoUi() noexcept
{
    ui->descriptionPlainTextEdit->blockSignals(true);
    ui->pitchSpinBox->blockSignals(true);
    ui->bankSpinBox->blockSignals(true);
    ui->trueHeadingSpinBox->blockSignals(true);
    ui->indicatedAirspeedSpinBox->blockSignals(true);
    ui->engineEventComboBox->blockSignals(true);

    const bool hasSelection = ui->locationTableWidget->selectionModel()->hasSelection();
    bool readOnly {true};
    if (hasSelection) {
        const auto selectedRow = getSelectedRow();
        auto item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::typeColumn);
        readOnly = item->data(Qt::EditRole).toLongLong() == d->PresetLocationTypeId;
        item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::descriptionColumn);
        ui->descriptionPlainTextEdit->setPlainText(item->text());
        item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::pitchColumn);
        ui->pitchSpinBox->setValue(item->text().toDouble());
        item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::bankColumn);
        ui->bankSpinBox->setValue(item->text().toDouble());
        item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::trueHeadingColumn);
        ui->trueHeadingSpinBox->setValue(item->text().toDouble());
        item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::indicatedAirspeedColumn);
        ui->indicatedAirspeedSpinBox->setValue(item->text().toInt());
        item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::engineColumn);
        ui->engineEventComboBox->setCurrentId(item->text().toLongLong());
    } else {
        ui->descriptionPlainTextEdit->clear();
        ui->pitchSpinBox->setValue(::DefaultPitch);
        ui->bankSpinBox->setValue(::DefaultBank);
        ui->trueHeadingSpinBox->setValue(::DefaultHeading);
        ui->indicatedAirspeedSpinBox->setValue(ui->defaultIndicatedAirspeedSpinBox->value());
        ui->engineEventComboBox->setCurrentId(ui->defaultEngineEventComboBox->getCurrentId());
    }

    ui->descriptionPlainTextEdit->setReadOnly(readOnly);
    ui->pitchSpinBox->setReadOnly(readOnly);
    ui->bankSpinBox->setReadOnly(readOnly);
    ui->trueHeadingSpinBox->setReadOnly(readOnly);
    ui->indicatedAirspeedSpinBox->setReadOnly(readOnly);

    ui->descriptionPlainTextEdit->blockSignals(false);
    ui->pitchSpinBox->blockSignals(false);
    ui->bankSpinBox->blockSignals(false);
    ui->trueHeadingSpinBox->blockSignals(false);
    ui->indicatedAirspeedSpinBox->blockSignals(false);
    ui->engineEventComboBox->blockSignals(false);
}

void LocationWidget::updateTable() noexcept
{
    if (PersistenceManager::getInstance().isConnected()) {

        std::vector<Location> locations = d->moduleSettings.hasSelectors() ?
                                              d->locationService->getSelectedLocations(d->moduleSettings.getLocationSelector()) :
                                              d->locationService->getAll();

        // Prevent table state changes notify the module settings
        ui->locationTableWidget->blockSignals(true);
        ui->locationTableWidget->setSortingEnabled(false);           
        ui->locationTableWidget->clearContents();
        ui->locationTableWidget->setRowCount(static_cast<int>(locations.size()));

        int row {0};
        for (const auto &location : locations) {
            initRow(location, row);
            ++row;
        }

        ui->locationTableWidget->setSortingEnabled(true);        
        ui->locationTableWidget->blockSignals(false);

    } else {
        // Clear existing entries
        ui->locationTableWidget->setRowCount(0);
    }
    updateLocationCount();
}

inline const QTableWidgetItem *LocationWidget::createRow(const Location &location) noexcept
{
    const int row = ui->locationTableWidget->rowCount();
    ui->locationTableWidget->insertRow(row);
    updateLocationCount();
    return initRow(location, row);
}

inline const QTableWidgetItem *LocationWidget::initRow(const Location &location, int row) noexcept
{
    const bool presetLocation {location.typeId == d->PresetLocationTypeId};
    int column {0};

    // ID
    std::unique_ptr<QTableWidgetItem> newItem = std::make_unique<QTableWidgetItem>();
    const QTableWidgetItem *firstItem = newItem.get();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    newItem->setToolTip(tr("Double-click to teleport to location."));
    // Transfer ownership of newItem to table widget
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Title
    newItem = std::make_unique<QTableWidgetItem>();
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit title."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Description
    newItem = std::make_unique<QTableWidgetItem>();
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit description."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Type
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::typeEnumeration);
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Category
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::categoryEnumeration);
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit category."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Country
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::countryEnumeration);
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit country."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Identifier
    newItem = std::make_unique<QTableWidgetItem>();
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit identifier."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Position
    newItem = std::make_unique<PositionWidgetItem>();
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit position."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Altitude
    newItem = std::make_unique<UnitWidgetItem>(d->unit, Unit::Name::Feet);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit altitude."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Pitch
    newItem = std::make_unique<QTableWidgetItem>();
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Bank
    newItem = std::make_unique<QTableWidgetItem>();
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Heading
    newItem = std::make_unique<QTableWidgetItem>();
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Indicated airspeed
    newItem = std::make_unique<QTableWidgetItem>();
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Local date
    newItem = std::make_unique<QTableWidgetItem>();
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit the local simulation date."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Local time
    newItem = std::make_unique<QTableWidgetItem>();
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit the local simulation time."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // On ground
    newItem = std::make_unique<TableCheckableItem>();
    if (presetLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsUserCheckable);
    } else {
        newItem->setToolTip(tr("Click to toggle on ground."));
        newItem->setFlags((newItem->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Engine event
    newItem = std::make_unique<QTableWidgetItem>();
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    updateRow(location, row);

    return firstItem;
}

int LocationWidget::getRowById(std::int64_t id) const noexcept
{
    int row {::InvalidRow};
    const auto rowCount = ui->locationTableWidget->rowCount();
    int currentRow {rowCount - 1};
    while (row == ::InvalidRow && currentRow >= 0) {
        const auto *currentItem = ui->locationTableWidget->item(currentRow, LocationWidgetPrivate::idColumn);
        if (currentItem->data(Qt::DisplayRole).toLongLong() == id) {
            row = currentRow;
        } else {
            --currentRow;
        }
    }
    return row;
}

inline void LocationWidget::updateRow(const Location &location, int row) noexcept
{
    // ID
    auto item = ui->locationTableWidget->item(row, LocationWidgetPrivate::idColumn);
    item->setData(Qt::DisplayRole, QVariant::fromValue(location.id));

    // Title
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::titleColumn);
    item->setData(Qt::DisplayRole, location.title);

    // Description
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::descriptionColumn);
    item->setData(Qt::DisplayRole, location.description);

    // Type
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::typeColumn);
    item->setData(Qt::EditRole, QVariant::fromValue(location.typeId));

    // Category
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::categoryColumn);
    item->setData(Qt::EditRole, QVariant::fromValue(location.categoryId));

    // Country
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::countryColumn);
    item->setData(Qt::EditRole, QVariant::fromValue(location.countryId));

    // Identifier
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::identifierColumn);
    item->setData(Qt::DisplayRole, location.identifier);

    // Position
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::positionColumn);
    item->setData(Qt::EditRole, Unit::formatCoordinates(location.latitude, location.longitude));

    // Altitude
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::altitudeColumn);
    item->setData(Qt::EditRole, location.altitude);

    // Pitch
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::pitchColumn);
    item->setData(Qt::EditRole, location.pitch);

    // Bank
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::bankColumn);
    item->setData(Qt::EditRole, location.bank);

    // True heading
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::trueHeadingColumn);
    item->setData(Qt::EditRole, location.trueHeading);

    // Indicated airspeed
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::indicatedAirspeedColumn);
    item->setData(Qt::EditRole, location.indicatedAirspeed);

    // Local simulation date
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::localSimulationDateColumn);
    item->setData(Qt::DisplayRole, location.localSimulationDate);

    // Local simulation time
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::localSimulationTimeColumn);
    item->setData(Qt::DisplayRole, location.localSimulationTime);

    // On ground
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::onGroundColumn);
    item->setCheckState(location.onGround ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    // Engine event
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::engineColumn);
    item->setData(Qt::EditRole, QVariant::fromValue(location.engineEventId));
}

inline void LocationWidget::updateLocationCount() const noexcept
{
    const int locationCount = ui->locationTableWidget->rowCount();
    ui->locationCountLabel->setText(tr("%1 locations", "Number of locations selected", locationCount).arg(locationCount));
}

void LocationWidget::teleportToLocation(int row) noexcept
{
    if (!SkyConnectManager::getInstance().isActive()) {
        Location location = getLocationByRow(row);

        QDate localSimulationDate;
        QTime localSimulationTime;
        if (d->moduleSettings.getDateSelection() == LocationSettings::DateSelection::LocationDateTime) {
            localSimulationDate = location.localSimulationDate.isValid() ? location.localSimulationDate : ui->dateEdit->date();
            localSimulationTime = location.localSimulationTime;
        } else {
            localSimulationDate = ui->dateEdit->date();
        }
        emit teleportTo(location, localSimulationDate, localSimulationTime);
    }
}

Location LocationWidget::getLocationByRow(int row) const noexcept
{
    Location location;

    auto item = ui->locationTableWidget->item(row, LocationWidgetPrivate::idColumn);
    location.id = item->data(Qt::EditRole).toLongLong();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::titleColumn);
    location.title = item->data(Qt::EditRole).toString();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::descriptionColumn);
    location.description = item->data(Qt::EditRole).toString();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::typeColumn);
    location.typeId = item->data(Qt::EditRole).toLongLong();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::categoryColumn);
    location.categoryId = item->data(Qt::EditRole).toLongLong();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::countryColumn);
    location.countryId = item->data(Qt::EditRole).toLongLong();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::identifierColumn);
    location.identifier = item->data(Qt::EditRole).toString();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::positionColumn);
    const auto coordinates = item->data(Qt::EditRole).toString().split(',');
    location.latitude = coordinates.first().toDouble();
    location.longitude = coordinates.last().toDouble();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::altitudeColumn);
    location.altitude = item->data(Qt::EditRole).toDouble();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::pitchColumn);
    location.pitch = item->data(Qt::EditRole).toDouble();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::bankColumn);
    location.bank = item->data(Qt::EditRole).toDouble();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::trueHeadingColumn);
    location.trueHeading = item->data(Qt::EditRole).toDouble();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::indicatedAirspeedColumn);
    location.indicatedAirspeed = item->data(Qt::EditRole).toInt();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::localSimulationDateColumn);
    location.localSimulationDate = item->data(Qt::EditRole).toDate();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::localSimulationTimeColumn);
    location.localSimulationTime = item->data(Qt::EditRole).toTime();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::onGroundColumn);
    location.onGround = item->checkState() == Qt::CheckState::Checked;

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::engineColumn);
    location.engineEventId = item->data(Qt::EditRole).toLongLong();

    return location;
}

void LocationWidget::tryPasteLocation() noexcept
{
    const auto text = QApplication::clipboard()->text();
    bool ok {false};
    PositionParser::Coordinate coordinate = PositionParser::parse(text, &ok);
    if (ok) {
        addUserLocation(coordinate.first, coordinate.second);
    }
}

int LocationWidget::getSelectedRow() const noexcept
{
    int selectedRow {::InvalidRow};
    const auto select = ui->locationTableWidget->selectionModel();
    const auto modelIndices = select->selectedRows(LocationWidgetPrivate::idColumn);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        selectedRow = modelIndex.row();
    }
    return selectedRow;
}

std::int64_t LocationWidget::getSelectedLocationId() const noexcept
{
    std::int64_t selectedLocationId {Const::InvalidId};
    const auto selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        selectedLocationId = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::idColumn)->data(Qt::EditRole).toLongLong();
    }
    return selectedLocationId;
}

// PRIVATE SLOTS

void LocationWidget::updateUi() noexcept
{
    updateTable();
    updateEditUi();
    updateInfoUi();
}

void LocationWidget::updateEditUi() noexcept
{
    const auto &skyConnectManager = SkyConnectManager::getInstance();
    const bool isActive = skyConnectManager.isActive();
    const bool hasSelection = ui->locationTableWidget->selectionModel()->hasSelection();

    ui->teleportPushButton->setEnabled(hasSelection && !isActive);
    bool editableRow {false};
    if (hasSelection) {
        const auto selectedRow = getSelectedRow();
        Location location =  getLocationByRow(selectedRow);
        editableRow = location.typeId != d->PresetLocationTypeId;
    }
    ui->updatePushButton->setEnabled(editableRow);
    ui->deletePushButton->setEnabled(editableRow);

    ui->pitchSpinBox->setEnabled(editableRow);
    ui->bankSpinBox->setEnabled(editableRow);
    ui->trueHeadingSpinBox->setEnabled(editableRow);
    ui->indicatedAirspeedSpinBox->setEnabled(editableRow);
    ui->engineEventComboBox->setEnabled(editableRow);

    const auto dateSelection = static_cast<LocationSettings::DateSelection>(ui->dateComboBox->currentData().toInt());
    ui->dateEdit->setEnabled(dateSelection != LocationSettings::DateSelection::Today);
    if (dateSelection == LocationSettings::DateSelection::Today || !ui->dateEdit->date().isValid()) {
        ui->dateEdit->setDate(QDate::currentDate());
    }
}

void LocationWidget::onCategoryChanged() noexcept
{
    auto categoryId = ui->categoryComboBox->getCurrentId();
    if (categoryId == d->NoneLocationCategoryId) {
        categoryId = Const::InvalidId;
    }
    d->moduleSettings.setCategoryId(categoryId);
}

void LocationWidget::onCountryChanged() noexcept
{
    auto countryId = ui->countryComboBox->getCurrentId();
    if (countryId == d->WorldCountryId) {
        countryId = Const::InvalidId;
    }
    d->moduleSettings.setCountryId(countryId);
}

void LocationWidget::onSearchTextChanged() noexcept
{
    d->searchTimer->start();
}

void LocationWidget::searchText() noexcept
{
    d->moduleSettings.setSearchKeyword(ui->searchLineEdit->text());
}

void LocationWidget::onTypeOptionToggled(const QVariant &optionValue, bool enable) noexcept
{
    auto typeSelection = d->moduleSettings.getTypeSelection();
    auto typeId = optionValue.toLongLong();
    if (enable) {
        typeSelection.insert(typeId);
    } else {
        typeSelection.erase(typeId);
    }
    d->moduleSettings.setTypeSelection(typeSelection);
}

void LocationWidget::resetFilter() noexcept
{
    d->moduleSettings.resetFilter();
}

void LocationWidget::resetDefaultValues() noexcept
{
    d->moduleSettings.resetDefaultValues();
}

void LocationWidget::onCellSelected(int row, [[maybe_unused]] int column) noexcept
{
    const auto item = ui->locationTableWidget->item(row, column);
    if (column != LocationWidgetPrivate::idColumn && (item->flags() & Qt::ItemIsEditable)) {
        ui->locationTableWidget->editItem(item);
    } else {
        teleportToLocation(row);
    }
}

void LocationWidget::onCellChanged(int row, [[maybe_unused]] int column) noexcept
{
    const auto location = getLocationByRow(row);
    d->locationService->update(location);
}

void LocationWidget::onSelectionChanged() noexcept
{
    updateEditUi();
    updateInfoUi();
}

void LocationWidget::onAddLocation() noexcept
{
    addUserLocation(0.0, 0.0);
}

void LocationWidget::onCaptureLocation() noexcept
{
    emit doCaptureLocation();
}

void LocationWidget::onUpdateLocation() noexcept
{
    emit doUpdateLocation();
}

void LocationWidget::onTeleportToSelectedLocation() noexcept
{
    const auto selectedItems = ui->locationTableWidget->selectionModel()->selectedRows();
    if (selectedItems.count() > 0) {
        const int row = selectedItems.last().row();
        teleportToLocation(row);
    }
}

void LocationWidget::onDeleteLocation() noexcept
{
    const auto selectedLocationId = getSelectedLocationId();
    if (selectedLocationId != Const::InvalidId) {
        auto &settings = Settings::getInstance();
        bool doDelete {true};
        if (settings.isDeleteLocationConfirmationEnabled()) {
            std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
            auto *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), messageBox.get());

            // Sequence numbers start at 1
            messageBox->setWindowTitle(tr("Delete Aircraft"));
            messageBox->setText(tr("The location with ID %1 is about to be deleted. Do you want to delete the location?").arg(selectedLocationId));
            messageBox->setInformativeText(tr("Deletion cannot be undone."));
            QPushButton *deleteButton = messageBox->addButton(tr("&Delete"), QMessageBox::AcceptRole);
            QPushButton *keepButton = messageBox->addButton(tr("&Keep"), QMessageBox::RejectRole);
            messageBox->setDefaultButton(keepButton);
            messageBox->setCheckBox(dontAskAgainCheckBox);
            messageBox->setIcon(QMessageBox::Icon::Question);

            messageBox->exec();
            doDelete = messageBox->clickedButton() == deleteButton;
            settings.setDeleteLocationConfirmationEnabled(!dontAskAgainCheckBox->isChecked());
        }

        if (doDelete) {
            const auto lastSelectedRow = getSelectedRow();
            d->locationService->deleteById(selectedLocationId);
            updateUi();
            const auto selectedRow = std::min(lastSelectedRow, ui->locationTableWidget->rowCount() - 1);
            ui->locationTableWidget->selectRow(selectedRow);
            ui->locationTableWidget->setFocus(Qt::NoFocusReason);
        }
    }
}

void LocationWidget::onDescriptionChanged() noexcept
{
    const auto selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.description = ui->descriptionPlainTextEdit->toPlainText();
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            const auto item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::descriptionColumn);
            item->setData(Qt::EditRole, location.description);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onPitchChanged(double value) noexcept
{
    const auto selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.pitch = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            const auto item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::pitchColumn);
            item->setData(Qt::EditRole, location.pitch);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onBankChanged(double value) noexcept
{
    const auto selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.bank = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            const auto item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::bankColumn);
            item->setData(Qt::EditRole, location.bank);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onHeadingChanged(double value) noexcept
{
    const auto selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.trueHeading = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            const auto item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::trueHeadingColumn);
            item->setData(Qt::EditRole, location.trueHeading);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onIndicatedAirspeedChanged(int value) noexcept
{
    const auto selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.indicatedAirspeed = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            const auto item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::indicatedAirspeedColumn);
            item->setData(Qt::EditRole, location.indicatedAirspeed);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onEngineEventChanged() noexcept
{
    const auto selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.engineEventId = ui->engineEventComboBox->getCurrentId();
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            const auto item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::engineColumn);
            item->setData(Qt::EditRole, QVariant::fromValue(location.engineEventId));
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onDateSelected() noexcept
{
    const auto dateSelection = static_cast<LocationSettings::DateSelection>(ui->dateComboBox->currentData().toInt());
    d->moduleSettings.setDateSelection(dateSelection);
}

void LocationWidget::onDateChanged(QDate date) noexcept
{
    d->moduleSettings.setDate(date);
}

void LocationWidget::onTimeSelected() noexcept
{
    const auto timeSelection = static_cast<LocationSettings::TimeSelection>(ui->timeComboBox->currentData().toInt());
    d->moduleSettings.setTimeSelection(timeSelection);
}

void LocationWidget::onDefaultAltitudeChanged(int value) noexcept
{
    d->moduleSettings.setDefaultAltitude(value);
}

void LocationWidget::onDefaultIndicatedAirspeedChanged(int value) noexcept
{
    d->moduleSettings.setDefaultIndicatedAirspeed(value);
}

void LocationWidget::onDefaultEngineEventChanged() noexcept
{
    d->moduleSettings.setDefaultEngineEventId(ui->defaultEngineEventComboBox->getCurrentId());
}

void LocationWidget::onDefaultOnGroundChanged(bool enable) noexcept
{
    d->moduleSettings.setDefaultOnGround(enable);
}

void LocationWidget::onTableLayoutChanged() noexcept
{
    QByteArray tableState = ui->locationTableWidget->horizontalHeader()->saveState();
    d->moduleSettings.setLocationTableState(std::move(tableState));
}

void LocationWidget::onModuleSettingsChanged() noexcept
{
    // Filters
    ui->typeOptionGroup->blockSignals(true);
    ui->typeOptionGroup->clearOptions();
    for (const auto type : d->moduleSettings.getTypeSelection()) {
        ui->typeOptionGroup->setOptionEnabled(QVariant::fromValue(type), true);
    }
    ui->typeOptionGroup->blockSignals(false);

    ui->categoryComboBox->blockSignals(true);
    std::int64_t categoryId = d->moduleSettings.getCategoryId();
    if (categoryId == Const::InvalidId) {
        categoryId = d->NoneLocationCategoryId;
    }
    ui->categoryComboBox->setCurrentId(categoryId);
    ui->categoryComboBox->blockSignals(false);

    ui->countryComboBox->blockSignals(true);
    std::int64_t countryId = d->moduleSettings.getCountryId();
    if (countryId == Const::InvalidId) {
        countryId = d->WorldCountryId;
    }
    ui->countryComboBox->setCurrentId(countryId);
    ui->countryComboBox->blockSignals(false);

    ui->searchLineEdit->blockSignals(true);
    ui->searchLineEdit->setText(d->moduleSettings.getSearchKeyword());
    ui->searchLineEdit->blockSignals(false);

    // Default values
    ui->defaultAltitudeSpinBox->blockSignals(true);
    ui->defaultAltitudeSpinBox->setValue(d->moduleSettings.getDefaultAltitude());
    ui->defaultAltitudeSpinBox->blockSignals(false);

    ui->defaultIndicatedAirspeedSpinBox->blockSignals(true);
    ui->defaultIndicatedAirspeedSpinBox->setValue(d->moduleSettings.getDefaultIndicatedAirspeed());
    ui->defaultIndicatedAirspeedSpinBox->blockSignals(false);

    ui->defaultEngineEventComboBox->blockSignals(true);
    ui->defaultEngineEventComboBox->setCurrentId(d->moduleSettings.getDefaultEngineEventId());
    ui->defaultEngineEventComboBox->blockSignals(false);

    ui->defaultOnGroundCheckBox->blockSignals(true);
    ui->defaultOnGroundCheckBox->setChecked(d->moduleSettings.isDefaultOnGround());
    ui->defaultOnGroundCheckBox->blockSignals(false);

    // Date and time
    const auto dateSelection = d->moduleSettings.getDateSelection();
    int currentIndex {0};
    while (currentIndex < ui->dateComboBox->count() &&
           static_cast<LocationSettings::DateSelection>(ui->dateComboBox->itemData(currentIndex).toInt()) != dateSelection) {
        ++currentIndex;
    }
    ui->dateComboBox->blockSignals(true);
    ui->dateComboBox->setCurrentIndex(currentIndex);
    ui->dateComboBox->blockSignals(false);

    ui->dateEdit->blockSignals(true);
    ui->dateEdit->setDate(d->moduleSettings.getDate());
    ui->dateEdit->blockSignals(false);

    const auto timeSelection = d->moduleSettings.getTimeSelection();
    currentIndex = 0;
    while (currentIndex < ui->timeComboBox->count() &&
           static_cast<LocationSettings::TimeSelection>(ui->timeComboBox->itemData(currentIndex).toInt()) != timeSelection) {
        ++currentIndex;
    }
    ui->timeComboBox->blockSignals(true);
    ui->timeComboBox->setCurrentIndex(currentIndex);
    ui->timeComboBox->blockSignals(false);

    updateTable();
    updateEditUi();
}
