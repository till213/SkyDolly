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
#include <cstdint>
#include <functional>

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStringList>
#include <QByteArray>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QKeyEvent>
#include <QKeySequence>
#include <QClipboard>
#include <QRegularExpression>
#include <QApplication>
#include <QTimer>

#include <GeographicLib/DMS.hpp>

#include <Kernel/Const.h>
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
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/AbstractModule.h>
#include "LocationWidget.h"
#include "EnumerationItemDelegate.h"
#include "PositionWidgetItem.h"
#include "ui_LocationWidget.h"

namespace
{
    constexpr int InvalidRow {-1};
    constexpr int InvalidColumn {-1};

    constexpr bool DefaultOnGround {false};

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
    LocationWidgetPrivate() noexcept
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

    LocationSelector locationSelector;
    std::unique_ptr<QTimer> searchTimer {std::make_unique<QTimer>()};
    std::unique_ptr<LocationService> locationService {std::make_unique<LocationService>()};
    std::unique_ptr<EnumerationService> enumerationService {std::make_unique<EnumerationService>()};
    std::unique_ptr<EnumerationItemDelegate> locationCategoryDelegate {std::make_unique<EnumerationItemDelegate>(EnumerationService::LocationCategory)};
    std::unique_ptr<EnumerationItemDelegate> countryDelegate {std::make_unique<EnumerationItemDelegate>(EnumerationService::Country)};

    const std::int64_t SystemLocationTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeSystemSymId).id()};
    const std::int64_t UserLocationTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeUserSymId).id()};
    const std::int64_t ImportLocationTypeId {PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeImportSymId).id()};
    const std::int64_t NoneLocationCategoryId {PersistedEnumerationItem(EnumerationService::LocationCategory, EnumerationService::LocationCategoryNoneSymId).id()};
    const std::int64_t WorldCountryId {PersistedEnumerationItem(EnumerationService::Country, EnumerationService::CountryWorldSymId).id()};
    const std::int64_t KeepEngineEventId {PersistedEnumerationItem(EnumerationService::EngineEvent, EnumerationService::EngineEventKeepSymId).id()};

    Unit unit;
    // Columns are only auto-resized the first time the table is loaded
    // After that manual column resizes are kept
    bool columnsAutoResized {false};

    static inline int idColumn {InvalidColumn};
    static inline int titleColumn {InvalidColumn};
    static inline int descriptionColumn {InvalidColumn};
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
    static inline int attributesColumn {InvalidColumn};
    static inline int engineColumn {InvalidColumn};

    static inline Enumeration typeEnumeration;
    static inline Enumeration categoryEnumeration;
    static inline Enumeration countryEnumeration;
};

// PUBLIC

LocationWidget::LocationWidget(QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::LocationWidget>()),
      d(std::make_unique<LocationWidgetPrivate>())
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
}

LocationWidget::~LocationWidget()
{
    const QByteArray tableState = ui->locationTableWidget->horizontalHeader()->saveState();
    Settings::getInstance().setLocationTableState(tableState);
}

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
    if (newLocation.typeId == Const::InvalidId) {
        newLocation.typeId = d->UserLocationTypeId;
    }
    if (newLocation.categoryId == Const::InvalidId) {
        newLocation.categoryId = d->NoneLocationCategoryId;
    }
    if (newLocation.countryId == Const::InvalidId) {
        newLocation.countryId = d->WorldCountryId;
    }
    if (newLocation.engineEventId == Const::InvalidId) {
        newLocation.engineEventId = ui->defaultEngineEventComboBox->getCurrentId();
    }
    Location location {std::move(newLocation)};
    if (d->locationService->store(location)) {
        if (!d->locationSelector.showUserLocations()) {
            // Make sure that user locations are visible
            ui->typeOptionGroup->setOptionEnabled(QVariant::fromValue(d->UserLocationTypeId), true);
        }
        ui->locationTableWidget->setSortingEnabled(false);
        ui->locationTableWidget->blockSignals(true);
        const QTableWidgetItem *firstItem = createRow(location);
        ui->locationTableWidget->blockSignals(false);
        // Automatically select newly inserted item (make sure that signals are emitted
        // again)
        ui->locationTableWidget->selectRow(ui->locationTableWidget->rowCount() - 1);
        ui->locationTableWidget->setSortingEnabled(true);
        ui->locationTableWidget->scrollToItem(firstItem); 
    }
}

void LocationWidget::updateLocation(const Location &location)
{
    const int selectedRow = getSelectedRow();
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
    ui->typeOptionGroup->addOption(tr("System"), QVariant::fromValue(d->SystemLocationTypeId), tr("Show system locations."));
    ui->typeOptionGroup->addOption(tr("User"), QVariant::fromValue(d->UserLocationTypeId), tr("Show user locations."));
    ui->typeOptionGroup->addOption(tr("Import"), QVariant::fromValue(d->ImportLocationTypeId), tr("Show imported locations."));

    // Table
    const QStringList headers {
        tr("ID"), tr("Title"), tr("Description"), tr("Type"), tr("Category"),
        tr("Country"), tr("Identifer"), tr("Position"), tr("Altitude"),
        tr("Pitch"), tr("Bank"), tr("True Heading"), tr("Indicated Airspeed"),
        tr("On Ground"), tr("Attributes"), tr("Engine")
    };
    LocationWidgetPrivate::idColumn = headers.indexOf(tr("ID"));
    LocationWidgetPrivate::titleColumn = headers.indexOf(tr("Title"));
    LocationWidgetPrivate::descriptionColumn = headers.indexOf(tr("Description"));
    LocationWidgetPrivate::typeColumn = headers.indexOf(tr("Type"));
    LocationWidgetPrivate::categoryColumn = headers.indexOf(tr("Category"));
    LocationWidgetPrivate::countryColumn = headers.indexOf(tr("Country"));
    LocationWidgetPrivate::identifierColumn = headers.indexOf(tr("Identifer"));
    LocationWidgetPrivate::positionColumn = headers.indexOf(tr("Position"));
    LocationWidgetPrivate::altitudeColumn = headers.indexOf(tr("Altitude"));
    LocationWidgetPrivate::pitchColumn = headers.indexOf(tr("Pitch"));
    LocationWidgetPrivate::bankColumn = headers.indexOf(tr("Bank"));
    LocationWidgetPrivate::trueHeadingColumn = headers.indexOf(tr("True Heading"));
    LocationWidgetPrivate::indicatedAirspeedColumn = headers.indexOf(tr("Indicated Airspeed"));
    LocationWidgetPrivate::onGroundColumn = headers.indexOf(tr("On Ground"));
    LocationWidgetPrivate::attributesColumn = headers.indexOf(tr("Attributes"));
    LocationWidgetPrivate::engineColumn = headers.indexOf(tr("Engine"));

    ui->locationTableWidget->setColumnCount(headers.count());
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
    ui->locationTableWidget->setColumnHidden(LocationWidgetPrivate::attributesColumn, true);
    ui->locationTableWidget->setColumnHidden(LocationWidgetPrivate::engineColumn, true);
    ui->locationTableWidget->setItemDelegateForColumn(LocationWidgetPrivate::categoryColumn, d->locationCategoryDelegate.get());
    ui->locationTableWidget->setItemDelegateForColumn(LocationWidgetPrivate::countryColumn, d->countryDelegate.get());

    QByteArray tableState = Settings::getInstance().getLocationTableState();
    ui->locationTableWidget->horizontalHeader()->restoreState(tableState);

    // Default "Delete" key deletes aircraft
    ui->deletePushButton->setShortcut(QKeySequence::Delete);

    ui->defaultAltitudeSpinBox->setMinimum(Const::MinimumAltitude);
    ui->defaultAltitudeSpinBox->setMaximum(Const::MaximumAltitude);
    ui->defaultAltitudeSpinBox->setValue(Const::DefaultAltitude);
    ui->defaultAltitudeSpinBox->setSuffix(tr(" feet"));
    ui->defaultIndicatedAirspeedSpinBox->setMinimum(Const::MinimumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setMaximum(Const::MaximumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setValue(Const::DefaultIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setSuffix(tr(" knots"));
    ui->defaultEngineEventComboBox->setEnumerationName(EnumerationService::EngineEvent);
    ui->defaultEngineEventComboBox->setCurrentId(d->KeepEngineEventId);
    ui->defaultOnGroundCheckBox->setChecked(::DefaultOnGround);    

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
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
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

    // Persistence
    PersistenceManager &persistenceManager = PersistenceManager::getInstance();
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
        const int selectedRow = getSelectedRow();
        QTableWidgetItem *item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::typeColumn);
        readOnly = item->data(Qt::EditRole).toLongLong() == d->SystemLocationTypeId;
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

        std::vector<Location> locations = d->locationSelector.hasSelectors() ?
                    d->locationService->getSelectedLocations(d->locationSelector) :
                    d->locationService->getAll();

        ui->locationTableWidget->blockSignals(true);
        ui->locationTableWidget->setSortingEnabled(false);           
        ui->locationTableWidget->clearContents();
        ui->locationTableWidget->setRowCount(static_cast<int>(locations.size()));

        int row {0};
        for (const Location &location : locations) {
            initRow(location, row);
            ++row;
        }
        if (!d->columnsAutoResized) {
            ui->locationTableWidget->resizeColumnsToContents();
            d->columnsAutoResized = true;
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
    const bool isSystemLocation {location.typeId == d->SystemLocationTypeId};
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
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit title."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Description
    newItem = std::make_unique<QTableWidgetItem>();
    if (isSystemLocation) {
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
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit category."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Country
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::countryEnumeration);
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit country."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Identifier
    newItem = std::make_unique<QTableWidgetItem>();
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit identifier."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Position
    newItem = std::make_unique<PositionWidgetItem>();
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit position."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Altitude
    newItem = std::make_unique<UnitWidgetItem>(d->unit, Unit::Name::Feet);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (isSystemLocation) {
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

    // On ground
    newItem = std::make_unique<QTableWidgetItem>();
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsUserCheckable);
    } else {
        newItem->setToolTip(tr("Click to toggle on ground."));
        newItem->setFlags((newItem->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsEditable);
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Attributes
    newItem = std::make_unique<QTableWidgetItem>();
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Engine event
    newItem = std::make_unique<QTableWidgetItem>();
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    updateRow(location, row);

    return firstItem;
}

inline void LocationWidget::updateRow(const Location &location, int row) noexcept
{
    // ID
    QTableWidgetItem *item = ui->locationTableWidget->item(row, LocationWidgetPrivate::idColumn);
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

    // On ground
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::onGroundColumn);
    item->setCheckState(location.onGround ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    // Attributes
    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::attributesColumn);
    item->setData(Qt::EditRole, QVariant::fromValue(location.attributes));

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
        emit teleportTo(location);
    }
}

Location LocationWidget::getLocationByRow(int row) const noexcept
{
    Location location;

    QTableWidgetItem *item = ui->locationTableWidget->item(row, LocationWidgetPrivate::idColumn);
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
    const QStringList coordinates = item->data(Qt::EditRole).toString().split(',');
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

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::onGroundColumn);
    location.onGround = item->checkState() == Qt::CheckState::Checked;

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::attributesColumn);
    location.attributes = item->data(Qt::EditRole).toLongLong();

    item = ui->locationTableWidget->item(row, LocationWidgetPrivate::engineColumn);
    location.engineEventId = item->data(Qt::EditRole).toLongLong();

    return location;
}

void LocationWidget::tryPasteLocation() noexcept
{
    const QString text = QApplication::clipboard()->text();
    bool ok {false};
    PositionParser::Coordinate coordinate = PositionParser::parse(text, &ok);
    if (ok) {
        addUserLocation(coordinate.first, coordinate.second);
    }
}

int LocationWidget::getSelectedRow() const noexcept
{
    int selectedRow {::InvalidRow};
    const QItemSelectionModel *select = ui->locationTableWidget->selectionModel();
    const QModelIndexList modelIndices = select->selectedRows(LocationWidgetPrivate::idColumn);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        selectedRow = modelIndex.row();
    }
    return selectedRow;
}

std::int64_t LocationWidget::getSelectedLocationId() const noexcept
{
    std::int64_t selectedLocationId {Const::InvalidId};
    const int selectedRow = getSelectedRow();
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
    const SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const bool isActive = skyConnectManager.isActive();
    const bool hasSelection = ui->locationTableWidget->selectionModel()->hasSelection();

    ui->teleportPushButton->setEnabled(hasSelection && !isActive);
    bool editableRow {false};
    if (hasSelection) {
        const int selectedRow = getSelectedRow();
        Location location =  getLocationByRow(selectedRow);
        editableRow = location.typeId != d->SystemLocationTypeId;
    }
    ui->updatePushButton->setEnabled(editableRow);
    ui->deletePushButton->setEnabled(editableRow);
}

void LocationWidget::onCategoryChanged() noexcept
{
    d->locationSelector.categoryId = ui->categoryComboBox->getCurrentId();
    if (d->locationSelector.categoryId == d->NoneLocationCategoryId) {
        d->locationSelector.categoryId = Const::InvalidId;
    }
    updateTable();
}

void LocationWidget::onCountryChanged() noexcept
{
    d->locationSelector.countryId = ui->countryComboBox->getCurrentId();
    if (d->locationSelector.countryId == d->WorldCountryId) {
        d->locationSelector.countryId = Const::InvalidId;
    }
    updateTable();
}

void LocationWidget::onSearchTextChanged() noexcept
{
    d->searchTimer->start();
}

void LocationWidget::searchText() noexcept
{
    d->locationSelector.searchKeyword = ui->searchLineEdit->text();
    updateTable();
}

void LocationWidget::onTypeOptionToggled(bool enable, const QVariant &userData) noexcept
{
    std::int64_t typeId = userData.toLongLong();
    if (enable) {
        d->locationSelector.typeIds.insert(typeId);
    } else {
        d->locationSelector.typeIds.erase(typeId);
    }
    updateTable();
}

void LocationWidget::onCellSelected(int row, [[maybe_unused]] int column) noexcept
{
    QTableWidgetItem *item = ui->locationTableWidget->item(row, column);
    if (column != LocationWidgetPrivate::idColumn) {
        ui->locationTableWidget->editItem(item);
    } else {
        teleportToLocation(row);
    }
}

void LocationWidget::onCellChanged(int row, [[maybe_unused]]int column) noexcept
{
    Location location = getLocationByRow(row);
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
    QList<QTableWidgetItem *> selectedItems = ui->locationTableWidget->selectedItems();
    if (selectedItems.count() > 0) {
        const int row = selectedItems.last()->row();
        teleportToLocation(row);
    }
}

void LocationWidget::onDeleteLocation() noexcept
{
    const std::int64_t selectedLocationId = getSelectedLocationId();
    if (selectedLocationId != Const::InvalidId) {
        Settings &settings = Settings::getInstance();
        bool doDelete {true};
        if (settings.isDeleteLocationConfirmationEnabled()) {
            std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
            QCheckBox *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), messageBox.get());

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
            const int lastSelectedRow = getSelectedRow();
            d->locationService->deleteById(selectedLocationId);
            updateUi();
            const int selectedRow = std::min(lastSelectedRow, ui->locationTableWidget->rowCount() - 1);
            ui->locationTableWidget->selectRow(selectedRow);
            ui->locationTableWidget->setFocus(Qt::NoFocusReason);
        }
    }
}

void LocationWidget::onDescriptionChanged() noexcept
{
    const int selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.description = ui->descriptionPlainTextEdit->toPlainText();
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::descriptionColumn);
            item->setData(Qt::EditRole, location.description);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onPitchChanged(double value) noexcept
{
    const int selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.pitch = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::pitchColumn);
            item->setData(Qt::EditRole, location.pitch);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onBankChanged(double value) noexcept
{
    const int selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.bank = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::bankColumn);
            item->setData(Qt::EditRole, location.bank);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onHeadingChanged(double value) noexcept
{
    const int selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.trueHeading = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::trueHeadingColumn);
            item->setData(Qt::EditRole, location.trueHeading);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onIndicatedAirspeedChanged(int value) noexcept
{
    const int selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.indicatedAirspeed = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::indicatedAirspeedColumn);
            item->setData(Qt::EditRole, location.indicatedAirspeed);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onEngineEventChanged([[maybe_unused]]int index) noexcept
{
    const int selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        Location location = getLocationByRow(selectedRow);
        location.engineEventId = ui->engineEventComboBox->getCurrentId();
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::engineColumn);
            item->setData(Qt::EditRole, QVariant::fromValue(location.engineEventId));
            ui->locationTableWidget->blockSignals(false);
        }
    }
}
