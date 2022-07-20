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
#ifndef LOGBOOKWIDGET_H
#define LOGBOOKWIDGET_H

#include <memory>
#include <forward_list>
#include <cstdint>

#include <QWidget>

class QShowEvent;
class QHideEvent;
class QAction;
class QTreeWidgetItem;
class QString;

#include <PluginManager/ModuleIntf.h>
#include <PluginManager/AbstractModule.h>

class DatabaseService;
class FlightService;
class FlightDate;
class FlightSummary;
class Aircraft;
struct LogbookWidgetPrivate;

namespace Ui {
    class LogbookWidget;
}

class LogbookWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LogbookWidget(FlightService &flightService, QWidget *parent = nullptr) noexcept;
    ~LogbookWidget() noexcept override;

private:
    Q_DISABLE_COPY(LogbookWidget)
    std::unique_ptr<Ui::LogbookWidget> ui;
    std::unique_ptr<LogbookWidgetPrivate> d;

    void initUi() noexcept;
    void initFilterUi() noexcept;
    void updateFlightTable() noexcept;
    inline void updateFlightSummaryRow(const FlightSummary &summary, int row) noexcept;
    void updateDateSelectorUi() noexcept;    
    void updateEditUi() noexcept;
    void frenchConnection() noexcept;
    inline void insertYear(QTreeWidgetItem *parent, std::forward_list<FlightDate> &flightDatesByYear, int nofFlightsPerYear) noexcept;
    inline void insertMonth(QTreeWidgetItem *parent, std::forward_list<FlightDate> &flightDatesByMonth, int nofFlightsPerMonth) noexcept;
    inline void insertDay(QTreeWidgetItem *parent, std::forward_list<FlightDate> &flightDatesByDayOfMonth) noexcept;
    inline void updateSelectionDateRange(QTreeWidgetItem *item) const noexcept;

    int getSelectedRow() const noexcept;
    std::int64_t getSelectedFlightId() const noexcept;

private slots:
    void onRecordingStarted() noexcept;
    void updateUi() noexcept;
    void updateAircraftIcons() noexcept;
    void onAircraftInfoChanged(const Aircraft &aircraft);

    void loadFlight() noexcept;
    void deleteFlight() noexcept;
    // Search
    void onSearchTextChanged() noexcept;
    void searchText() noexcept;

    // Flight log table
    void onSelectionChanged() noexcept;
    void onCellSelected(int row, int column) noexcept;
    void onCellChanged(int row, int column) noexcept;
    // Flight date tree
    void onDateItemClicked(QTreeWidgetItem *item) noexcept;

    // Filter options
    void filterByFormationFlights(bool checked) noexcept;
    void filterByEngineType(int index) noexcept;
    void filterByDuration(int index) noexcept;
};

#endif // LOGBOOKWIDGET_H
