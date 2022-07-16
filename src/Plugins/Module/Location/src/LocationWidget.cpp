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
#include <cstdint>

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStringList>
#include <QByteArray>
#include <QTextEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
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
#include <Persistence/LogbookManager.h>
#include <Persistence/PersistedEnumerationItem.h>
#include <Persistence/Service/LocationService.h>
#include <Persistence/Service/EnumerationService.h>
#include <Widget/FocusPlainTextEdit.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/AbstractModule.h>
#include "LocationWidget.h"
#include "EnumerationItemDelegate.h"
#include "PositionWidgetItem.h"
#include "EnumerationWidgetItem.h"
#include "ui_LocationWidget.h"

namespace
{
    constexpr int InvalidRowIndex {-1};
    constexpr int InvalidColumnIndex {-1};

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

    constexpr double MinimumPitch {-180.0};
    constexpr double MaximumPitch {180.0};
    constexpr double MinimumBank {-180.0};
    constexpr double MaximumBank {180.0};
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

    bool columnsAutoResized {false};
    int selectedRow {::InvalidRowIndex};
    std::int64_t selectedLocationId {Location::InvalidId};

    int idColumnIndex {InvalidColumnIndex};
    int titleColumnIndex {InvalidColumnIndex};
    int descriptionColumnIndex {InvalidColumnIndex};
    int typeColumnIndex {InvalidColumnIndex};
    int categoryColumnIndex {InvalidColumnIndex};
    int countryColumnIndex {InvalidColumnIndex};
    int identifierColumnIndex {InvalidColumnIndex};
    int positionColumnIndex {InvalidColumnIndex};
    int altitudeColumnIndex {InvalidColumnIndex};
    int pitchColumnIndex {InvalidColumnIndex};
    int bankColumnIndex {InvalidColumnIndex};
    int headingColumnIndex {InvalidColumnIndex};
    int indicatedAirspeedColumnIndex {InvalidColumnIndex};
    int onGroundColumnIndex {InvalidColumnIndex};
    int attributesColumnIndex {InvalidColumnIndex};

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
    qDebug() << "LocationWidget::LocationWidget: CREATED.";
#endif
}

LocationWidget::~LocationWidget() noexcept
{
    const QByteArray tableState = ui->locationTableWidget->horizontalHeader()->saveState();
    Settings::getInstance().setLocationTableState(tableState);
#ifdef DEBUG
    qDebug() << "LocationWidget::~LocationWidget: DELETED.";
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
        const int rowIndex = ui->locationTableWidget->rowCount();
        ui->locationTableWidget->insertRow(rowIndex);
        updateLocationRow(location, rowIndex);
        // Automatically select newly inserted item
        ui->locationTableWidget->selectRow(ui->locationTableWidget->rowCount() - 1);
        ui->locationTableWidget->setSortingEnabled(true);
        ui->locationTableWidget->blockSignals(false);
    }
}

void LocationWidget::addLocation(Location newLocation)
{
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
        tr("Pitch"), tr("Bank"), tr("Heading"), tr("Indicated Airspeed"),
        tr("On Ground"), tr("Attributes")
    };
    d->idColumnIndex = headers.indexOf(tr("ID"));
    d->titleColumnIndex = headers.indexOf(tr("Title"));
    d->descriptionColumnIndex = headers.indexOf(tr("Description"));
    d->typeColumnIndex = headers.indexOf(tr("Type"));
    d->categoryColumnIndex = headers.indexOf(tr("Category"));
    d->countryColumnIndex = headers.indexOf(tr("Country"));
    d->identifierColumnIndex = headers.indexOf(tr("Identifer"));
    d->positionColumnIndex = headers.indexOf(tr("Position"));
    d->altitudeColumnIndex = headers.indexOf(tr("Altitude"));
    d->pitchColumnIndex = headers.indexOf(tr("Pitch"));
    d->bankColumnIndex = headers.indexOf(tr("Bank"));
    d->headingColumnIndex = headers.indexOf(tr("Heading"));
    d->indicatedAirspeedColumnIndex = headers.indexOf(tr("Indicated Airspeed"));
    d->onGroundColumnIndex = headers.indexOf(tr("On Ground"));
    d->attributesColumnIndex = headers.indexOf(tr("Attributes"));

    ui->locationTableWidget->setColumnCount(headers.count());
    ui->locationTableWidget->setHorizontalHeaderLabels(headers);
    ui->locationTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->locationTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->locationTableWidget->verticalHeader()->hide();
    ui->locationTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->locationTableWidget->sortByColumn(d->idColumnIndex, Qt::SortOrder::DescendingOrder);
    ui->locationTableWidget->horizontalHeader()->setSectionsMovable(true);
    ui->locationTableWidget->setAlternatingRowColors(true);
    ui->locationTableWidget->setColumnHidden(d->typeColumnIndex, true);
    ui->locationTableWidget->setColumnHidden(d->descriptionColumnIndex, true);
    ui->locationTableWidget->setColumnHidden(d->pitchColumnIndex, true);
    ui->locationTableWidget->setColumnHidden(d->bankColumnIndex, true);
    ui->locationTableWidget->setColumnHidden(d->headingColumnIndex, true);
    ui->locationTableWidget->setColumnHidden(d->indicatedAirspeedColumnIndex, true);
    ui->locationTableWidget->setColumnHidden(d->attributesColumnIndex, true);
    ui->locationTableWidget->setItemDelegateForColumn(d->categoryColumnIndex, d->locationCategoryDelegate.get());
    ui->locationTableWidget->setItemDelegateForColumn(d->countryColumnIndex, d->countryDelegate.get());

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
    ui->bankSpinBox->setMinimum(::MinimumBank);
    ui->bankSpinBox->setMaximum(::MaximumBank);
    ui->headingSpinBox->setMinimum(::MinimumHeading);
    ui->headingSpinBox->setMaximum(::MaximumHeading);
    ui->indicatedAirspeedSpinBox->setMinimum(::MinimumIndicatedAirspeed);
    ui->indicatedAirspeedSpinBox->setMaximum(::MaximumIndicatedAirspeed);

    const int infoGroupBoxHeight = ui->informationGroupBox->minimumHeight();
    ui->splitter->setSizes({height() - infoGroupBoxHeight, infoGroupBoxHeight});
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 0);

    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void LocationWidget::frenchConnection() noexcept
{
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
    connect(ui->headingSpinBox, &QDoubleSpinBox::valueChanged,
            this, &LocationWidget::onHeadingChanged);
    connect(ui->indicatedAirspeedSpinBox, &QSpinBox::valueChanged,
            this, &LocationWidget::onIndicatedAirspeedChanged);
}

void LocationWidget::updateInfoUi() noexcept
{
    ui->descriptionPlainTextEdit->blockSignals(true);
    ui->pitchSpinBox->blockSignals(true);
    ui->bankSpinBox->blockSignals(true);
    ui->headingSpinBox->blockSignals(true);
    ui->indicatedAirspeedSpinBox->blockSignals(true);

    const bool hasSelection = ui->locationTableWidget->selectionModel()->hasSelection();
    bool readOnly {true};
    if (hasSelection) {
        QTableWidgetItem *item = ui->locationTableWidget->item(d->selectedRow, d->typeColumnIndex);
        readOnly = item->data(Qt::EditRole).toLongLong() == PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeSystemSymbolicId).id();
        item = ui->locationTableWidget->item(d->selectedRow, d->descriptionColumnIndex);
        ui->descriptionPlainTextEdit->setPlainText(item->text());
        item = ui->locationTableWidget->item(d->selectedRow, d->pitchColumnIndex);
        ui->pitchSpinBox->setValue(item->text().toDouble());
        item = ui->locationTableWidget->item(d->selectedRow, d->bankColumnIndex);
        ui->bankSpinBox->setValue(item->text().toDouble());
        item = ui->locationTableWidget->item(d->selectedRow, d->headingColumnIndex);
        ui->headingSpinBox->setValue(item->text().toDouble());
        item = ui->locationTableWidget->item(d->selectedRow, d->indicatedAirspeedColumnIndex);
        ui->indicatedAirspeedSpinBox->setValue(item->text().toInt());
    } else {
        ui->descriptionPlainTextEdit->clear();
        ui->pitchSpinBox->clear();
        ui->bankSpinBox->clear();
        ui->headingSpinBox->clear();
        ui->indicatedAirspeedSpinBox->clear();
    }

    ui->descriptionPlainTextEdit->setReadOnly(readOnly);
    ui->pitchSpinBox->setReadOnly(readOnly);
    ui->bankSpinBox->setReadOnly(readOnly);
    ui->headingSpinBox->setReadOnly(readOnly);
    ui->indicatedAirspeedSpinBox->setReadOnly(readOnly);

    ui->descriptionPlainTextEdit->blockSignals(false);
    ui->pitchSpinBox->blockSignals(false);
    ui->bankSpinBox->blockSignals(false);
    ui->headingSpinBox->blockSignals(false);
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

        int rowIndex {0};
        for (const Location &location : locations) {
            updateLocationRow(location, rowIndex);
            ++rowIndex;
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

inline void LocationWidget::updateLocationRow(const Location &location, int rowIndex) noexcept
{
    const bool isSystemLocation = location.typeId == PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeSystemSymbolicId).id();
    const Qt::ItemFlags systemFlags {Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled};
    int columnIndex {0};

    // ID
    std::unique_ptr<QTableWidgetItem> newItem = std::make_unique<QTableWidgetItem>();
    QVariant locationId = QVariant::fromValue(location.id);
    newItem->setData(Qt::DisplayRole, locationId);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newItem->setFlags(systemFlags);
    newItem->setToolTip(tr("Double-click to teleport to location."));
    // Transfer ownership of newItem to table widget
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Title
    newItem = std::make_unique<QTableWidgetItem>(location.title);
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    } else {
        newItem->setToolTip(tr("Double-click to edit title."));
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Description
    newItem = std::make_unique<QTableWidgetItem>(location.description);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Type
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::typeEnumeration);
    newItem->setData(Qt::EditRole, QVariant::fromValue(location.typeId));
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Category
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::categoryEnumeration);
    newItem->setData(Qt::EditRole, QVariant::fromValue(location.categoryId));
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    } else {
        newItem->setToolTip(tr("Double-click to edit category."));
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Country
    newItem = std::make_unique<EnumerationWidgetItem>(LocationWidgetPrivate::countryEnumeration);
    newItem->setData(Qt::EditRole, QVariant::fromValue(location.countryId));
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    } else {
        newItem->setToolTip(tr("Double-click to edit category."));
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Identifier
    newItem = std::make_unique<QTableWidgetItem>(location.identifier);
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    } else {
        newItem->setToolTip(tr("Double-click to edit identifier."));
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Position
    newItem = std::make_unique<PositionWidgetItem>();
    newItem->setData(Qt::ItemDataRole::EditRole, Unit::formatCoordinates(location.latitude, location.longitude));
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    } else {
        newItem->setToolTip(tr("Double-click to edit position."));
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Altitude
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.altitude);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    } else {
        newItem->setToolTip(tr("Double-click to edit altitude."));
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Pitch
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.pitch);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Bank
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.bank);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Heading
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.heading);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Indicated airspeed
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.indicatedAirspeed);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // On Ground
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setCheckState(location.onGround ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    if (isSystemLocation) {
        newItem->setFlags(systemFlags);
    } else {
        newItem->setToolTip(tr("Click to toggle on ground."));
    }
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Attributes
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.attributes);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;
}

void LocationWidget::teleportToLocation(int row) noexcept
{
    Location location = getLocationByRow(row);
    emit teleportTo(location);
}

Location LocationWidget::getLocationByRow(int row) const noexcept
{
    Location location;

    QTableWidgetItem *item = ui->locationTableWidget->item(row, d->idColumnIndex);
    location.id = item->data(Qt::EditRole).toLongLong();

    item = ui->locationTableWidget->item(row, d->titleColumnIndex);
    location.title = item->data(Qt::EditRole).toString();

    item = ui->locationTableWidget->item(row, d->descriptionColumnIndex);
    location.description = item->data(Qt::EditRole).toString();

    item = ui->locationTableWidget->item(row, d->typeColumnIndex);
    location.typeId = item->data(Qt::EditRole).toLongLong();

    item = ui->locationTableWidget->item(row, d->categoryColumnIndex);
    location.categoryId = item->data(Qt::EditRole).toLongLong();

    item = ui->locationTableWidget->item(row, d->countryColumnIndex);
    location.countryId = item->data(Qt::EditRole).toLongLong();

    item = ui->locationTableWidget->item(row, d->identifierColumnIndex);
    location.identifier = item->data(Qt::EditRole).toString();

    item = ui->locationTableWidget->item(row, d->positionColumnIndex);
    const QStringList coordinates = item->data(Qt::EditRole).toString().split(',');
    location.latitude = coordinates.first().toDouble();
    location.longitude = coordinates.last().toDouble();

    item = ui->locationTableWidget->item(row, d->altitudeColumnIndex);
    location.altitude = item->data(Qt::EditRole).toDouble();

    item = ui->locationTableWidget->item(row, d->pitchColumnIndex);
    location.pitch = item->data(Qt::EditRole).toDouble();

    item = ui->locationTableWidget->item(row, d->bankColumnIndex);
    location.bank = item->data(Qt::EditRole).toDouble();

    item = ui->locationTableWidget->item(row, d->headingColumnIndex);
    location.heading = item->data(Qt::EditRole).toDouble();

    item = ui->locationTableWidget->item(row, d->indicatedAirspeedColumnIndex);
    location.indicatedAirspeed = item->data(Qt::EditRole).toInt();

    item = ui->locationTableWidget->item(row, d->onGroundColumnIndex);
    location.onGround = item->checkState() == Qt::CheckState::Checked;

    item = ui->locationTableWidget->item(row, d->attributesColumnIndex);
    location.attributes = item->data(Qt::EditRole).toLongLong();

    return location;
}

QStringList LocationWidget::parseCoordinates(QString value)
{
    static QRegularExpression numberRexExp(R"(^([+-]?[0-9]*[.]?[0-9]+)[,]?[\s]*([+-]?[0-9]*[.]?[0-9]+)$)");
    static QRegularExpression dmsRegExp(R"(^([\d\W]+[N|S|E|W])[,]?([\d\W]+[E|W|N|S])$)");
    QStringList values;

    // GeographicLib DMS does not like whitespace in dms strings
    const QString trimmedValue = value.trimmed().replace(" ","");
    // First try to match comma-separated floating point numbers
    // (e.g. 46.94697890467696, 7.444134280004356)
    QRegularExpressionMatch match = numberRexExp.match(trimmedValue);
    if (match.hasMatch() && match.lastCapturedIndex() == 2) {
        values << match.captured(1).trimmed();
        values << match.captured(2).trimmed();
    } else {
        // Try parsing latitude/longitude DMS values
        // (e.g. 46° 56' 52.519" N 7° 26' 40.589" E or 7° 26' 40.589" E, 46° 56' 52.519" N)
        match = dmsRegExp.match(trimmedValue);
        if (match.hasMatch() && match.lastCapturedIndex() == 2) {
            values << match.captured(1).trimmed();
            values << match.captured(2).trimmed();
        }
    }
    return values;
}

void LocationWidget::tryPasteLocation() noexcept
{
    const QString text = QApplication::clipboard()->text();
    QStringList values = parseCoordinates(text);
    if (values.count() == 2) {
        try {
            double latitude {0.0};
            double longitude {0.0};
            const std::string first = values.first().toStdString();
            const std::string second = values.last().trimmed().toStdString();
            GeographicLib::DMS::flag flag {GeographicLib::DMS::NONE};
            double value = GeographicLib::DMS::Decode(first, flag);
            if (flag == GeographicLib::DMS::LATITUDE || flag == GeographicLib::DMS::NONE) {
                latitude = value;
            } else {
                longitude = value;
            }
            value = GeographicLib::DMS::Decode(second, flag);
            if (flag == GeographicLib::DMS::LONGITUDE || flag == GeographicLib::DMS::NONE) {
                longitude = value;
            } else {
                latitude = value;
            }
            addUserLocation(latitude, longitude);
        } catch (GeographicLib::GeographicErr err) {
#ifdef DEBUG
        qDebug() << "LocationWidget::tryPasteLocation: Not a coordinate:" << err.what();
#endif
        }
    }
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
        Location location =  getLocationByRow(d->selectedRow);
        editableRow = location.typeId != PersistedEnumerationItem(EnumerationService::LocationType, EnumerationService::LocationTypeSystemSymbolicId).id();
    }
    ui->deletePushButton->setEnabled(editableRow);
}

void LocationWidget::onCellSelected(int row, [[maybe_unused]] int column) noexcept
{
    QTableWidgetItem *item = ui->locationTableWidget->item(row, column);
    if (column != d->idColumnIndex) {
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
    QItemSelectionModel *select = ui->locationTableWidget->selectionModel();
    QModelIndexList modelIndices = select->selectedRows(d->idColumnIndex);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        d->selectedRow = modelIndex.row();
        d->selectedLocationId = ui->locationTableWidget->model()->data(modelIndex).toLongLong();
    } else {
        d->selectedRow = ::InvalidRowIndex;
        d->selectedLocationId = Location::InvalidId;
    }
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
    if (d->selectedLocationId != ::Location::InvalidId) {
        Settings &settings = Settings::getInstance();
        bool doDelete {true};
        if (settings.isDeleteLocationConfirmationEnabled()) {
            std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
            QCheckBox *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), messageBox.get());

            // Sequence numbers start at 1
            messageBox->setWindowTitle(tr("Delete Aircraft"));
            messageBox->setText(tr("The location with ID %1 is about to be deleted. Do you want to delete the location?").arg(d->selectedLocationId));
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
            d->locationService->deleteById(d->selectedLocationId);
            int lastSelectedRow = d->selectedRow;
            updateUi();
            int selectedRow = std::min(lastSelectedRow, ui->locationTableWidget->rowCount() - 1);
            ui->locationTableWidget->selectRow(selectedRow);
            ui->locationTableWidget->setFocus(Qt::NoFocusReason);
        }
    }
}

void LocationWidget::onDescriptionChanged() noexcept
{
    if (d->selectedRow != ::InvalidRowIndex) {
        Location location = getLocationByRow(d->selectedRow);
        location.description = ui->descriptionPlainTextEdit->toPlainText();
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(d->selectedRow, d->descriptionColumnIndex);
            item->setData(Qt::EditRole, location.description);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onPitchChanged(double value) noexcept
{
    if (d->selectedRow != ::InvalidRowIndex) {
        Location location = getLocationByRow(d->selectedRow);
        location.pitch = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(d->selectedRow, d->pitchColumnIndex);
            item->setData(Qt::EditRole, location.pitch);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onBankChanged(double value) noexcept
{
    if (d->selectedRow != ::InvalidRowIndex) {
        Location location = getLocationByRow(d->selectedRow);
        location.bank = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(d->selectedRow, d->bankColumnIndex);
            item->setData(Qt::EditRole, location.bank);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onHeadingChanged(double value) noexcept
{
    if (d->selectedRow != ::InvalidRowIndex) {
        Location location = getLocationByRow(d->selectedRow);
        location.heading = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(d->selectedRow, d->headingColumnIndex);
            item->setData(Qt::EditRole, location.heading);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}

void LocationWidget::onIndicatedAirspeedChanged(int value) noexcept
{
    if (d->selectedRow != ::InvalidRowIndex) {
        Location location = getLocationByRow(d->selectedRow);
        location.indicatedAirspeed = value;
        if (d->locationService->update(location)) {
            ui->locationTableWidget->blockSignals(true);
            QTableWidgetItem *item = ui->locationTableWidget->item(d->selectedRow, d->indicatedAirspeedColumnIndex);
            item->setData(Qt::EditRole, location.indicatedAirspeed);
            ui->locationTableWidget->blockSignals(false);
        }
    }
}
