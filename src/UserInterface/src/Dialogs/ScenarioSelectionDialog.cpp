/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <QTableWidgetItem>
#include <QVector>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QLocale>
#include <QDateTime>
#include <QTime>
#include <QPushButton>

#include "../../../Model/src/Scenario.h"
#include "../../../Model/src/ScenarioDescription.h"
#include "../../../Persistence/src/Service/ScenarioService.h"
#include "../Unit.h"
#include "ScenarioSelectionDialog.h"
#include "ui_ScenarioSelectionDialog.h"

namespace
{
    constexpr int MinimumTableWidth = 600;
    constexpr int InvalidSelection = -1;
}

class ScenarioSelectionDialogPrivate
{
public:
    ScenarioSelectionDialogPrivate(ScenarioService &theScenarioService)
        : scenarioService(theScenarioService),
          selectedRow(InvalidSelection),
          selectedScenarioId(Scenario::InvalidId)

    {}

    ScenarioService &scenarioService;
    int selectedRow;
    qint64 selectedScenarioId;
};

// PUBLIC

ScenarioSelectionDialog::ScenarioSelectionDialog(ScenarioService &scenarioService, QWidget *parent) noexcept
    : QDialog(parent),
      ui(new Ui::ScenarioSelectionDialog),
      d(std::make_unique<ScenarioSelectionDialogPrivate>(scenarioService))
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

ScenarioSelectionDialog::~ScenarioSelectionDialog() noexcept
{
    delete ui;
}

qint64 ScenarioSelectionDialog::getSelectedScenarioId() const noexcept
{
    return d->selectedScenarioId;
}

// PROTECTED

void ScenarioSelectionDialog::showEvent(QShowEvent *event) noexcept
{
    Q_UNUSED(event)
    updateUi();
}

// PRIVATE

void ScenarioSelectionDialog::initUi() noexcept
{
    ui->scenarioTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    const QStringList headers {tr("Scenario"), tr("Date"), tr("Aircraft"), tr("Departure Time"), tr("Departure Place"), tr("Arrival Time"), tr("Arrival Place"), tr("Total Time of Flight"), tr("Description")};
    ui->scenarioTableWidget->setColumnCount(headers.count());
    ui->scenarioTableWidget->setHorizontalHeaderLabels(headers);
    ui->scenarioTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->scenarioTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->scenarioTableWidget->setMinimumWidth(MinimumTableWidth);
}

void ScenarioSelectionDialog::updateUi() noexcept
{
    QVector<ScenarioDescription> descriptions = d->scenarioService.getScenarioDescriptions();
    QLocale systemLocale = QLocale::system();
    ui->scenarioTableWidget->setSortingEnabled(false);
    ui->scenarioTableWidget->clearContents();
    ui->scenarioTableWidget->setRowCount(descriptions.count());
    int rowIndex = 0;
    for (const ScenarioDescription &desc : descriptions) {
        QTableWidgetItem *newItem = new QTableWidgetItem();

        newItem->setData(Qt::DisplayRole, desc.id);
        ui->scenarioTableWidget->setItem(rowIndex, 0, newItem);

        newItem = new QTableWidgetItem(Unit::formatDate(desc.creationDate));
        ui->scenarioTableWidget->setItem(rowIndex, 1, newItem);

        newItem = new QTableWidgetItem(desc.aircraftType);
        ui->scenarioTableWidget->setItem(rowIndex, 2, newItem);

        newItem = new QTableWidgetItem(Unit::formatTime(desc.startDate));
        ui->scenarioTableWidget->setItem(rowIndex, 3, newItem);

        newItem = new QTableWidgetItem(Unit::formatTime(desc.endDate));
        ui->scenarioTableWidget->setItem(rowIndex, 5, newItem);

        const qint64 durationMSec = desc.startDate.msecsTo(desc.endDate);
        const QTime time = QTime(0, 0).addMSecs(durationMSec);
        newItem = new QTableWidgetItem(systemLocale.toString(time));
        ui->scenarioTableWidget->setItem(rowIndex, 7, newItem);

        newItem = new QTableWidgetItem(desc.description);
        ui->scenarioTableWidget->setItem(rowIndex, 8, newItem);

        ++rowIndex;
    }
    ui->scenarioTableWidget->resizeColumnsToContents();
    ui->scenarioTableWidget->setSortingEnabled(true);

    updateEditUi();
}

void ScenarioSelectionDialog::updateEditUi() noexcept
{
    ui->deletePushButton->setEnabled(d->selectedScenarioId != Scenario::InvalidId);
}

void ScenarioSelectionDialog::frenchConnection() noexcept
{
    connect(ui->scenarioTableWidget, &QTableWidget::itemSelectionChanged,
            this, &ScenarioSelectionDialog::handleSelectionChanged);
    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &ScenarioSelectionDialog::handleDelete);
}

// PRIVATE SLOTS

void ScenarioSelectionDialog::handleSelectionChanged() noexcept
{
    QItemSelectionModel *select = ui->scenarioTableWidget->selectionModel();
    QModelIndexList selectedRows = select->selectedRows(0);
    if (selectedRows.count() > 0) {
        QModelIndex modelIndex = selectedRows.at(0);
        d->selectedRow = modelIndex.row();
        d->selectedScenarioId = ui->scenarioTableWidget->model()->data(modelIndex).toLongLong();
    } else {
        d->selectedRow = InvalidSelection;
        d->selectedScenarioId = Scenario::InvalidId;
    }
    updateEditUi();
}

void ScenarioSelectionDialog::handleDelete() noexcept
{
    if (d->selectedScenarioId != Scenario::InvalidId) {
        d->scenarioService.deleteById(d->selectedScenarioId);
        int lastSelectedRow = d->selectedRow;
        updateUi();
        int selectedRow = qMin(lastSelectedRow, ui->scenarioTableWidget->rowCount() - 1);
        ui->scenarioTableWidget->selectRow(selectedRow);
    }
}
