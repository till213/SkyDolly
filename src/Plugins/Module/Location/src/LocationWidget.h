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
#ifndef LOCATIONWIDGET_H
#define LOCATIONWIDGET_H

#include <memory>
#include <cstdint>

#include <QWidget>
#include <QStringList>
#include <QDateTime>
#include <QDate>

class QTableWidgetItem;
class QKeyEvent;
class QShowEvent;

#include <Model/Location.h>
#include "LocationSettings.h"

class LocationSettings;
struct LocationWidgetPrivate;

namespace Ui {
    class LocationWidget;
}

class LocationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LocationWidget(LocationSettings &moduleSettings, QWidget *parent = nullptr) noexcept;
    LocationWidget(const LocationWidget &rhs) = delete;
    LocationWidget(LocationWidget &&rhs) = delete;
    LocationWidget &operator=(const LocationWidget &rhs) = delete;
    LocationWidget &operator=(LocationWidget &&rhs) = delete;
    ~LocationWidget() override;

    void addUserLocation(double latitude, double longitude);
    void addLocation(Location location);
    void updateLocation(const Location &location);

signals:
    void doUpdateLocation();
    void doCaptureLocation();
    void teleportTo(Location location, QDate localSimulationDate, QTime localSimulationTime);

protected:
    void showEvent(QShowEvent *event) noexcept override;
    void keyPressEvent(QKeyEvent *event) noexcept override;

private:
    const std::unique_ptr<Ui::LocationWidget> ui;
    const std::unique_ptr<LocationWidgetPrivate> d;

    void initUi() noexcept;
    void frenchConnection() noexcept;
    void updateInfoUi() noexcept;

    void updateTable() noexcept;
    inline const QTableWidgetItem *createRow(const Location &location) noexcept;
    inline const QTableWidgetItem *initRow(const Location &location, int row) noexcept;
    int getRowById(std::int64_t id) const noexcept;
    inline void updateRow(const Location &location, int row) noexcept;
    inline void updateLocationCount() const noexcept;

    void teleportToLocation(int row) noexcept;
    Location getLocationByRow(int row) const noexcept;

    void tryPasteLocation() noexcept;

    int getSelectedRow() const noexcept;
    std::int64_t getSelectedLocationId() const noexcept;

private slots:
    void updateUi() noexcept;
    void updateEditUi() noexcept;

    // Search
    void onCategoryChanged() noexcept;
    void onCountryChanged() noexcept;
    void onSearchTextChanged() noexcept;
    void searchText() noexcept;
    void onTypeOptionToggled(const QVariant &optionValue, bool enable) noexcept;
    void resetFilter() noexcept;
    void resetDefaultValues() noexcept;

    void onCellSelected(int row, int column) noexcept;
    void onCellChanged(int row, int column) noexcept;
    void onSelectionChanged() noexcept;

    void onAddLocation() noexcept;
    void onCaptureLocation() noexcept;
    void onUpdateLocation() noexcept;
    void onTeleportToSelectedLocation() noexcept;
    void onDeleteLocation() noexcept;

    void onDescriptionChanged() noexcept;
    void onPitchChanged(double value) noexcept;
    void onBankChanged(double value) noexcept;
    void onHeadingChanged(double value) noexcept;
    void onIndicatedAirspeedChanged(int value) noexcept;
    void onEngineEventChanged() noexcept;

    // Date and time
    void onDateSelected() noexcept;
    void onDateChanged(QDate date) noexcept;
    void onTimeSelected() noexcept;

    // Default values
    void onDefaultAltitudeChanged(int value) noexcept;
    void onDefaultIndicatedAirspeedChanged(int value) noexcept;
    void onDefaultEngineEventChanged() noexcept;
    void onDefaultOnGroundChanged(bool enable) noexcept;

    // Settings
    void onTableLayoutChanged() noexcept;
    void onModuleSettingsChanged() noexcept;
};

#endif // LOCATIONWIDGET_H
