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

#include <QStringList>
#ifdef DEBUG
#include <QDebug>
#endif

#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/AbstractModule.h>
#include "LocationWidget.h"
#include "ui_LocationWidget.h"

namespace
{

}

class LocationWidgetPrivate
{
public:
    LocationWidgetPrivate() noexcept
    {}
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
    const QStringList headers {tr("Position"), tr("Description")};
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

// PRIVATE SLOTS

void LocationWidget::updateUi() noexcept
{

}
