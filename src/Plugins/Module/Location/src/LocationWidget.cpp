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
#ifdef DEBUG
#include <QDebug>
#endif

#include <GeographicLib/DMS.hpp>

#include <Kernel/Settings.h>
#include <Kernel/Unit.h>
#include <Kernel/PositionParser.h>
#include <Model/Logbook.h>
#include <Persistence/LogbookManager.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/Service/LocationService.h>
#include <Persistence/Service/EnumerationService.h>
#include <Widget/FocusPlainTextEdit.h>
#include <Widget/EnumerationWidgetItem.h>
#include <Widget/UnitWidgetItem.h>
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

    constexpr double DefaultAltitude {5000};
    // Dead Sea Depression - The lowest point on Earth: -413 meters
    // https://geology.com/below-sea-level/
    constexpr double MinimumAltitude {-1500};
    // https://www.reddit.com/r/flightsim/comments/ibstui/turns_out_the_maximum_altitude_in_fs2020_275000/
    constexpr double MaximumAltitude {275000};
    constexpr int DefaultIndicatedAirspeed {120};
    constexpr int MinimumIndicatedAirspeed {0};
    // NASA X-43 (Mach 9.6)
    // https://internationalaviationhq.com/2020/06/27/17-fastest-aircraft/
    constexpr int MaximumIndicatedAirspeed {6350};
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
}

struct LocationWidgetPrivate
{
    LocationWidgetPrivate() noexcept
    {
        if (typeEnumeration.count() == 0) {
            enumerationService->getEnumerationByName(typeEnumeration);
        }
        if (categoryEnumeration.count() == 0) {
            enumerationService->getEnumerationByName(categoryEnumeration);
        }
        if (countryEnumeration.count() == 0) {
            enumerationService->getEnumerationByName(countryEnumeration);
        }
    }

    std::unique_ptr<LocationService> locationService {std::make_unique<LocationService>()};
    std::unique_ptr<EnumerationService> enumerationService {std::make_unique<EnumerationService>()};
    std::unique_ptr<EnumerationItemDelegate> locationCategoryDelegate {std::make_unique<EnumerationItemDelegate>(EnumerationService::LocationCategory)};
    std::unique_ptr<EnumerationItemDelegate> countryDelegate {std::make_unique<EnumerationItemDelegate>(EnumerationService::Country)};

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

    static inline Enumeration typeEnumeration {EnumerationService::LocationType};
    static inline Enumeration categoryEnumeration {EnumerationService::LocationCategory};
    static inline Enumeration countryEnumeration {EnumerationService::Country};
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
#ifdef DEBUG
    qDebug() << "LocationWidget::LocationWidget: CREATED";
#endif
}

LocationWidget::~LocationWidget() noexcept
{
    const QByteArray tableState = ui->locationTableWidget->horizontalHeader()->saveState();
    Settings::getInstance().setLocationTableState(tableState);
#ifdef DEBUG
    qDebug() << "LocationWidget::~LocationWidget: DELETED";
#endif
}

void LocationWidget::addUserLocation(double latitude, double longitude)
{
    Location location;
    location.typeId = PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeUserSymbolicId).id();
    location.categoryId = PersistedEnumerationItem(EnumerationService::LocationCategory, EnumerationService::LocationCategoryNoneSymbolicId).id();
    location.countryId = PersistedEnumerationItem(EnumerationService::Country, EnumerationService::CountryWorldSymbolicId).id();
    location.latitude = latitude;
    location.longitude = longitude;
    location.altitude = ui->defaultAltitudeSpinBox->value();
    location.indicatedAirspeed = ui->defaultIndicatedAirspeedSpinBox->value();
    location.onGround = ui->defaultOnGroundCheckBox->isChecked();
    if (d->locationService->store(location)) {
        ui->locationTableWidget->blockSignals(true);
        ui->locationTableWidget->setSortingEnabled(false);
        const int row = ui->locationTableWidget->rowCount();
        ui->locationTableWidget->insertRow(row);
        updateLocationRow(location, row);
        ui->locationTableWidget->blockSignals(false);
        // Automatically select newly inserted item (make sure that signals are emitted
        // again)
        ui->locationTableWidget->selectRow(ui->locationTableWidget->rowCount() - 1);
        ui->locationTableWidget->setSortingEnabled(true);
    }
}

void LocationWidget::addLocation(Location newLocation)
{
    if (newLocation.typeId == Location::InvalidId) {
        newLocation.typeId = PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeUserSymbolicId).id();
    }
    if (newLocation.categoryId == Location::InvalidId) {
        newLocation.categoryId = PersistedEnumerationItem(EnumerationService::LocationCategory, EnumerationService::LocationCategoryNoneSymbolicId).id();
    }
    if (newLocation.countryId == Location::InvalidId) {
        newLocation.countryId = PersistedEnumerationItem(EnumerationService::Country, EnumerationService::CountryWorldSymbolicId).id();
    }
    Location location {newLocation};
    if (d->locationService->store(location)) {
        const int rowCount = ui->locationTableWidget->rowCount();
        ui->locationTableWidget->blockSignals(true);
        ui->locationTableWidget->setSortingEnabled(false);
        ui->locationTableWidget->insertRow(rowCount);
        updateLocationRow(location, rowCount);
        ui->locationTableWidget->setSortingEnabled(true);
        ui->locationTableWidget->blockSignals(false);
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
    const QStringList headers {
        tr("ID"), tr("Title"), tr("Description"), tr("Type"), tr("Category"),
        tr("Country"), tr("Identifer"), tr("Position"), tr("Altitude"),
        tr("Pitch"), tr("Bank"), tr("True Heading"), tr("Indicated Airspeed"),
        tr("On Ground"), tr("Attributes")
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
    ui->locationTableWidget->setItemDelegateForColumn(LocationWidgetPrivate::categoryColumn, d->locationCategoryDelegate.get());
    ui->locationTableWidget->setItemDelegateForColumn(LocationWidgetPrivate::countryColumn, d->countryDelegate.get());

    QByteArray tableState = Settings::getInstance().getLocationTableState();
    ui->locationTableWidget->horizontalHeader()->restoreState(tableState);

    // Default "Delete" key deletes aircraft
    ui->deletePushButton->setShortcut(QKeySequence::Delete);

    ui->defaultAltitudeSpinBox->setMinimum(::MinimumAltitude);
    ui->defaultAltitudeSpinBox->setMaximum(::MaximumAltitude);
    ui->defaultAltitudeSpinBox->setValue(::DefaultAltitude);
    ui->defaultIndicatedAirspeedSpinBox->setMinimum(::MinimumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setMaximum(::MaximumIndicatedAirspeed);
    ui->defaultIndicatedAirspeedSpinBox->setValue(::DefaultIndicatedAirspeed);
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
    ui->indicatedAirspeedSpinBox->setMinimum(::MinimumIndicatedAirspeed);
    ui->indicatedAirspeedSpinBox->setMaximum(::MaximumIndicatedAirspeed);
    ui->indicatedAirspeedSpinBox->setSuffix(tr(" knots"));

    const int infoGroupBoxHeight = ui->informationGroupBox->minimumHeight();
    ui->splitter->setSizes({height() - infoGroupBoxHeight, infoGroupBoxHeight});
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 0);

    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void LocationWidget::frenchConnection() noexcept
{
    // Logbook
    const Logbook &logbook = Logbook::getInstance();
    connect(&LogbookManager::getInstance(), &LogbookManager::connectionChanged,
            this, &LocationWidget::updateUi);

    // Connection
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &LocationWidget::updateEditUi);

    // Location table
    connect(ui->locationTableWidget, &QTableWidget::cellDoubleClicked,
            this, &LocationWidget::onCellSelected);
    connect(ui->locationTableWidget, &QTableWidget::cellChanged,
            this, &LocationWidget::onCellChanged);
    connect(ui->locationTableWidget, &QTableWidget::itemSelectionChanged,
            this, &LocationWidget::onSelectionChanged);
    connect(ui->addPushButton, &QPushButton::clicked,
            this, &LocationWidget::onAddLocation);
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
}

void LocationWidget::updateInfoUi() noexcept
{
    ui->descriptionPlainTextEdit->blockSignals(true);
    ui->pitchSpinBox->blockSignals(true);
    ui->bankSpinBox->blockSignals(true);
    ui->trueHeadingSpinBox->blockSignals(true);
    ui->indicatedAirspeedSpinBox->blockSignals(true);

    const bool hasSelection = ui->locationTableWidget->selectionModel()->hasSelection();
    bool readOnly {true};
    if (hasSelection) {
        const int selectedRow = getSelectedRow();
        QTableWidgetItem *item = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::typeColumn);
        readOnly = item->data(Qt::EditRole).toLongLong() == PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeSystemSymbolicId).id();
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
    } else {
        ui->descriptionPlainTextEdit->clear();
        ui->pitchSpinBox->setValue(::DefaultPitch);
        ui->bankSpinBox->setValue(::DefaultBank);
        ui->trueHeadingSpinBox->setValue(::DefaultHeading);
        ui->indicatedAirspeedSpinBox->setValue(ui->defaultIndicatedAirspeedSpinBox->value());
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
}

void LocationWidget::updateLocationTable() noexcept
{
    if (LogbookManager::getInstance().isConnected()) {

        std::vector<Location> locations;
        d->locationService->getAll(std::back_inserter(locations));

        ui->locationTableWidget->blockSignals(true);
        ui->locationTableWidget->setSortingEnabled(false);
        ui->locationTableWidget->clearContents();
        ui->locationTableWidget->setRowCount(static_cast<int>(locations.size()));

        int row {0};
        for (const Location &location : locations) {
            updateLocationRow(location, row);
            ++row;
        }

        ui->locationTableWidget->setSortingEnabled(true);
        if (!d->columnsAutoResized) {
            ui->locationTableWidget->resizeColumnsToContents();
            d->columnsAutoResized = true;
        }
        ui->locationTableWidget->blockSignals(false);

    } else {
        // Clear existing entries
        ui->locationTableWidget->setRowCount(0);
    }
}

inline void LocationWidget::updateLocationRow(const Location &location, int row) noexcept
{
    const bool isSystemLocation {location.typeId == PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeSystemSymbolicId).id()};
    int column {0};

    // ID
    std::unique_ptr<QTableWidgetItem> newItem = std::make_unique<QTableWidgetItem>();
    QVariant locationId = QVariant::fromValue(location.id);
    newItem->setData(Qt::DisplayRole, locationId);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    newItem->setToolTip(tr("Double-click to teleport to location."));
    // Transfer ownership of newItem to table widget
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Title
    newItem = std::make_unique<QTableWidgetItem>(location.title);
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit title."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Description
    newItem = std::make_unique<QTableWidgetItem>(location.description);
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit description."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Type
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::typeEnumeration);
    newItem->setData(Qt::EditRole, QVariant::fromValue(location.typeId));
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Category
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::categoryEnumeration);
    newItem->setData(Qt::EditRole, QVariant::fromValue(location.categoryId));
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit category."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Country
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::countryEnumeration);
    newItem->setData(Qt::EditRole, QVariant::fromValue(location.countryId));
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit category."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Identifier
    newItem = std::make_unique<QTableWidgetItem>(location.identifier);
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit identifier."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Position
    newItem = std::make_unique<PositionWidgetItem>();
    newItem->setData(Qt::ItemDataRole::EditRole, Unit::formatCoordinates(location.latitude, location.longitude));
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
    } else {
        newItem->setToolTip(tr("Double-click to edit position."));
    }
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Altitude
    newItem = std::make_unique<UnitWidgetItem>(d->unit, Unit::Name::Feet);
    newItem->setData(Qt::EditRole, location.altitude);
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
    newItem->setData(Qt::EditRole, location.pitch);
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Bank
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.bank);
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Heading
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.trueHeading);
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Indicated airspeed
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.indicatedAirspeed);
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;

    // On Ground
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setCheckState(location.onGround ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
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
    newItem->setData(Qt::EditRole, QVariant::fromValue(location.attributes));
    ui->locationTableWidget->setItem(row, column, newItem.release());
    ++column;
}

void LocationWidget::teleportToLocation(int row) noexcept
{
    Location location = getLocationByRow(row);
    emit teleportTo(location);
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
    std::int64_t selectedLocationId {::Location::InvalidId};
    const int selectedRow = getSelectedRow();
    if (selectedRow != ::InvalidRow) {
        selectedLocationId = ui->locationTableWidget->item(selectedRow, LocationWidgetPrivate::idColumn)->data(Qt::EditRole).toLongLong();
    }
    return selectedLocationId;
}

// PRIVATE SLOTS

void LocationWidget::updateUi() noexcept
{
    updateLocationTable();
    updateEditUi();
    updateInfoUi();
}

void LocationWidget::updateEditUi() noexcept
{
    const bool isActive = SkyConnectManager::getInstance().isActive();
    const bool hasSelection = ui->locationTableWidget->selectionModel()->hasSelection();

    ui->teleportPushButton->setEnabled(hasSelection && !isActive);
    bool editableRow {false};
    if (hasSelection) {
        const int selectedRow = getSelectedRow();
        Location location =  getLocationByRow(selectedRow);
        editableRow = location.typeId != PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeSystemSymbolicId).id();
    }
    ui->deletePushButton->setEnabled(editableRow);
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
    if (selectedLocationId != ::Location::InvalidId) {
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
