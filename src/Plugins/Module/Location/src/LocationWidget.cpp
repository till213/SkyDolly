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
#include <QStringList>
#include <QByteArray>
#include <QItemSelectionModel>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Settings.h>
#include <Persistence/LogbookManager.h>
#include <Persistence/Service/LocationService.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/AbstractModule.h>
#include "LocationWidget.h"
#include "PositionWidgetItem.h"
#include "ui_LocationWidget.h"

namespace
{
    constexpr int InvalidRowIndex {-1};
    constexpr int InvalidColumnIndex {-1};
}

class LocationWidgetPrivate
{
public:
    LocationWidgetPrivate() noexcept
    {}

    std::unique_ptr<LocationService> locationService {std::make_unique<LocationService>()};
    bool columnsAutoResized {false};
    int selectedRow {::InvalidRowIndex};
    std::int64_t selectedLocationId {Location::InvalidId};

    int idColumnIndex {InvalidColumnIndex};
    int positionColumnIndex {InvalidColumnIndex};
    int altitudeColumnIndex {InvalidColumnIndex};
    int pitchColumnIndex {InvalidColumnIndex};
    int bankColumnIndex {InvalidColumnIndex};
    int headingColumnIndex {InvalidColumnIndex};
    int indicatedAirspeedColumnIndex {InvalidColumnIndex};
    int onGroundColumnIndex {InvalidColumnIndex};
    int descriptionColumnIndex {InvalidColumnIndex};
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

void LocationWidget::addLocation(Location newLocation)
{
    Location location {newLocation};
    if (d->locationService->store(location)) {
        const int rowCount = ui->locationTableWidget->rowCount();
        ui->locationTableWidget->setSortingEnabled(false);
        ui->locationTableWidget->insertRow(rowCount);
        updateLocation(location, rowCount);
        ui->locationTableWidget->setSortingEnabled(true);
    }
}

// PRIVATE

void LocationWidget::initUi() noexcept
{
    const QStringList headers {tr("ID"), tr("Position"), tr("Altitude"), tr("Pitch"), tr("Bank"), tr("Heading"), tr("Indicated Airspeed"), tr("On Ground"), tr("Description")};
    d->idColumnIndex = headers.indexOf(tr("ID"));
    d->positionColumnIndex = headers.indexOf(tr("Position"));
    d->altitudeColumnIndex = headers.indexOf(tr("Altitude"));
    d->pitchColumnIndex = headers.indexOf(tr("Pitch"));
    d->bankColumnIndex = headers.indexOf(tr("Bank"));
    d->headingColumnIndex = headers.indexOf(tr("Heading"));
    d->indicatedAirspeedColumnIndex = headers.indexOf(tr("Indicated Airspeed"));
    d->onGroundColumnIndex = headers.indexOf(tr("On Ground"));
    d->descriptionColumnIndex = headers.indexOf(tr("Description"));

    ui->locationTableWidget->setColumnCount(headers.count());
    ui->locationTableWidget->setHorizontalHeaderLabels(headers);
    ui->locationTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->locationTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->locationTableWidget->verticalHeader()->hide();
    ui->locationTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->locationTableWidget->sortByColumn(d->idColumnIndex, Qt::SortOrder::DescendingOrder);
    ui->locationTableWidget->horizontalHeader()->setSectionsMovable(true);
    ui->locationTableWidget->setAlternatingRowColors(true);

    QByteArray tableState = Settings::getInstance().getLocationTableState();
    ui->locationTableWidget->horizontalHeader()->restoreState(tableState);

    // Default "Delete" key deletes aircraft
    ui->deletePushButton->setShortcut(QKeySequence::Delete);
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
            updateLocation(location, rowIndex);
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

inline void LocationWidget::updateLocation(const Location &location, int rowIndex) noexcept
{
    int columnIndex {0};

    // ID
    std::unique_ptr<QTableWidgetItem> newItem = std::make_unique<QTableWidgetItem>();
    QVariant locationId = QVariant::fromValue(location.id);
    newItem->setData(Qt::DisplayRole, locationId);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newItem->setFlags(Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled);
    newItem->setToolTip(tr("Double-click to teleport to location."));
    // Transfer ownership of newItem to table widget
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Position
    newItem = std::make_unique<PositionWidgetItem>();
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    newItem->setData(Qt::ItemDataRole::EditRole, Unit::formatCoordinates(location.latitude, location.longitude));
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Altitude
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.altitude);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Pitch
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.pitch);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Bank
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.bank);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Heading
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.heading);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Indicated airspeed
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::EditRole, location.indicatedAirspeed);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // On Ground
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setCheckState(location.onGround ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Description
    newItem = std::make_unique<QTableWidgetItem>(location.description);
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;
}

void LocationWidget::teleportToLocation(int row) noexcept
{
    Location location = rowToLocation(row);
    emit teleportTo(location);
}

Location LocationWidget::rowToLocation(int row) const noexcept
{
    Location location;

    QTableWidgetItem *item = ui->locationTableWidget->item(row, d->idColumnIndex);
    location.id = item->data(Qt::EditRole).toLongLong();

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

    return location;
}

// PRIVATE SLOTS

void LocationWidget::updateUi() noexcept
{
    updateLocationTable();
    updateEditUi();
}

void LocationWidget::updateEditUi() noexcept
{
    const bool isActive = SkyConnectManager::getInstance().isActive();
    const bool hasSelection = ui->locationTableWidget->selectionModel()->hasSelection();

    ui->teleportPushButton->setEnabled(!isActive);
    ui->deletePushButton->setEnabled(hasSelection);
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

void LocationWidget::onCellChanged(int row, int column) noexcept
{
    Location location = rowToLocation(row);
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
}

void LocationWidget::onAddLocation() noexcept
{
    Location location;
    if (d->locationService->store(location)) {
        ui->locationTableWidget->blockSignals(true);
        ui->locationTableWidget->setSortingEnabled(false);
        const int rowIndex = ui->locationTableWidget->rowCount();
        ui->locationTableWidget->insertRow(rowIndex);
        updateLocation(location, rowIndex);
        ui->locationTableWidget->setSortingEnabled(true);
        ui->locationTableWidget->blockSignals(false);
    }
}

void LocationWidget::onCaptureLocation() noexcept
{
    emit captureLocation();
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
    // TODO IMPLEMENT ME
    if (d->selectedLocationId != ::Location::InvalidId) {
        d->locationService->deleteById(d->selectedLocationId);
        int lastSelectedRow = d->selectedRow;
        updateUi();
        int selectedRow = std::min(lastSelectedRow, ui->locationTableWidget->rowCount() - 1);
        ui->locationTableWidget->selectRow(selectedRow);
        ui->locationTableWidget->setFocus(Qt::NoFocusReason);
    }
}
