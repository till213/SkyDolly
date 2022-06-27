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

#include <QTableWidget>
#include <QStringList>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Unit.h>
#include <Persistence/LogbookManager.h>
#include <Persistence/Service/LocationService.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/AbstractModule.h>
#include "LocationWidget.h"
#include "ui_LocationWidget.h"

namespace
{
    constexpr int InvalidColumnIndex {-1};
}

class LocationWidgetPrivate
{
public:
    LocationWidgetPrivate() noexcept
    {}

    std::unique_ptr<LocationService> locationService {std::make_unique<LocationService>()};
    bool columnsAutoResized {false};
    int idColumnIndex {::InvalidColumnIndex};
    Unit unit;
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
#ifdef DEBUG
    qDebug() << "LocationWidget::~LocationWidget: DELETED.";
#endif
}

// PRIVATE

void LocationWidget::initUi() noexcept
{
    const QStringList headers {tr("ID"), tr("Position"), tr("Description")};
    ui->locationTableWidget->setColumnCount(headers.count());
    ui->locationTableWidget->setHorizontalHeaderLabels(headers);
    ui->locationTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->locationTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->locationTableWidget->verticalHeader()->hide();
    ui->locationTableWidget->horizontalHeader()->setStretchLastSection(true);
    // ui->locationTableWidget->sortByColumn(::FlightIdColumn, Qt::SortOrder::DescendingOrder);
    ui->locationTableWidget->horizontalHeader()->setSectionsMovable(true);
    ui->locationTableWidget->setAlternatingRowColors(true);
}

void LocationWidget::frenchConnection() noexcept
{

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
            addLocation(location, rowIndex);
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

inline void LocationWidget::addLocation(const Location &location, int rowIndex) noexcept
{
    int columnIndex {0};

    // ID
    std::unique_ptr<QTableWidgetItem> newItem = std::make_unique<QTableWidgetItem>();
    QVariant locationId = QVariant::fromValue(location.id);
    newItem->setData(Qt::DisplayRole, locationId);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newItem->setToolTip(tr("Double-click to load location."));
    // Transfer ownership of newItem to table widget
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    d->idColumnIndex = columnIndex;
    ++columnIndex;

    // Location
    newItem = std::make_unique<QTableWidgetItem>(d->unit.formatLatLongPosition(location.latitude, location.longitude));
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Description
    newItem = std::make_unique<QTableWidgetItem>(location.description);
    newItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->locationTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;
}

// PRIVATE SLOTS

void LocationWidget::updateUi() noexcept
{
    updateLocationTable();
}
