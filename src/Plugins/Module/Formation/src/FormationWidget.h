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
#ifndef FORMATIONWIDGET_H
#define FORMATIONWIDGET_H

#include <memory>
#include <cstdint>

#include <QWidget>

class QShowEvent;
class QHideEvent;
class QAction;
class QTableWidgetItem;
class QRadioButton;
class QAbstractButton;

#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Module/ModuleIntf.h>
#include <PluginManager/Module/AbstractModule.h>
#include "FormationSettings.h"
#include "Formation.h"

class Aircraft;
struct PositionData;
class FlightService;
class AircraftService;
class FormationSettings;
struct FormationWidgetPrivate;

namespace Ui {
    class FormationWidget;
}

class FormationWidget : public QWidget
{
    Q_OBJECT
public:
    FormationWidget(FormationSettings &moduleSettings, QWidget *parent = nullptr) noexcept;
    FormationWidget(const FormationWidget &rhs) = delete;
    FormationWidget(FormationWidget &&rhs) = delete;
    FormationWidget &operator=(const FormationWidget &rhs) = delete;
    FormationWidget &operator=(FormationWidget &&rhs) = delete;
    ~FormationWidget() override;

    Formation::HorizontalDistance getHorizontalDistance() const noexcept;
    Formation::VerticalDistance getVerticalDistance() const noexcept;
    Formation::Bearing getRelativePosition() const noexcept;

private:
    std::unique_ptr<Ui::FormationWidget> ui;
    const std::unique_ptr<FormationWidgetPrivate> d;

    void initUi() noexcept;
    void initTimeOffsetUi() noexcept;
    void frenchConnection() noexcept;

    void updateTable() noexcept;
    void updateInteractiveUi() noexcept;
    void updateAircraftIcons() noexcept;
    void updateReferenceAircraftIcon() noexcept;
    void updateRelativePositionUi() noexcept;
    void updateEditUi() noexcept;
    void updateTimeOffsetUi() noexcept;
    void updateReplayUi() noexcept;
    void updateReplayModeUi(SkyConnectIntf::ReplayMode replayMode) noexcept;
    void updateToolTips() noexcept;

    inline const QTableWidgetItem *createRow(const Aircraft &aircraft, int aircraftIndex) noexcept;
    inline const QTableWidgetItem *initRow(const Aircraft &aircraft, int row, int aircraftIndex) noexcept;
    inline void updateRow(const Aircraft &aircraft, int row, int aircraftIndex) noexcept;

    void updateAndSendUserAircraftPosition() const noexcept;
    void updateUserAircraftPosition(SkyConnectIntf::ReplayMode replayMode) const noexcept;

    int getSelectedRow() const noexcept;
    int getRowBySequenceNumber(int sequenceNumber) const noexcept;
    int getRowByAircraftIndex(int index) const noexcept;

    void updateAircraftCount() const noexcept;
    void updateRelativePosition();
    
private slots:
    void updateUi() noexcept;

    void onUserAircraftChanged() noexcept;
    void onAircraftAdded(const Aircraft &aircraft) noexcept;
    void onAircraftInfoChanged(const Aircraft &aircraft) noexcept;

    void onCellSelected(int row, int column) noexcept;
    void onCellChanged(int row, int column) noexcept;
    void onSelectionChanged() noexcept;

    void onInitialPositionPlacementChanged(bool enable) noexcept;
    void updateUserAircraftIndex() noexcept;
    void deleteAircraft() noexcept;

    void onRelativePositionChanged() noexcept;
    void onHorizontalDistanceChanged() noexcept;
    void onVerticalDistanceChanged() noexcept;
    void onReplayModeSelected() noexcept;
    void onReplayModeChanged(SkyConnectIntf::ReplayMode replayMode);

    void changeTimeOffset(const std::int64_t timeOffset) noexcept;
    void onTimeOffsetValueChanged() noexcept;
    void resetAllTimeOffsets() noexcept;

    // Settings
    void onTableLayoutChanged() noexcept;
    void onModuleSettingsChanged() noexcept;
    QRadioButton &getPositionButtonFromSettings() const noexcept;
    Formation::Bearing bearingFromPositionGroup() const noexcept;
    void restoreDefaultSettings() noexcept;
};

#endif // FORMATIONWIDGET_H
