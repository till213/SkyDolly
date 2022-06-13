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
#ifndef FORMATIONWIDGET_H
#define FORMATIONWIDGET_H

#include <memory>
#include <cstdint>

#include <QWidget>

class QShowEvent;
class QHideEvent;
class QAction;

#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/ModuleIntf.h>
#include <PluginManager/ModulePluginBase.h>
#include "Formation.h"

class Aircraft;
struct PositionData;

namespace Ui {
    class FormationWidget;
}

class FlightService;
class AircraftService;
struct FormationWidgetPrivate;

class FormationWidget : public QWidget
{
    Q_OBJECT
public:
    FormationWidget(FlightService &flightService, AircraftService &aircraftService, QWidget *parent = nullptr) noexcept;
    ~FormationWidget() noexcept override;

    Formation::HorizontalDistance getHorizontalDistance() const noexcept;
    Formation::VerticalDistance getVerticalDistance() const noexcept;
    Formation::RelativePosition getRelativePosition() const noexcept;

private:
    Q_DISABLE_COPY(FormationWidget)
    std::unique_ptr<Ui::FormationWidget> ui;
    std::unique_ptr<FormationWidgetPrivate> d;

    void initUi() noexcept;
    void initTimeOffsetUi() noexcept;
    void frenchConnection() noexcept;

    void updateAircraftTable() noexcept;
    void updateAircraftIcons() noexcept;
    void updateRelativePositionUi() noexcept;
    void updateEditUi() noexcept;
    void updateTimeOffsetUi() noexcept;
    void updateReplayUi() noexcept;
    void updateToolTips() noexcept;

    void addAircraft(const Aircraft &aircraft, int rowIndex) noexcept;
    void updateAndSendUserAircraftPosition() const noexcept;
    void updateUserAircraftPosition(SkyConnectIntf::ReplayMode replayMode) const noexcept;

private slots:
    void updateUi() noexcept;

    void onRelativePositionChanged() noexcept;
    void onUserAircraftChanged() noexcept;
    void onAircraftInfoChanged() noexcept;

    void onCellSelected(int row, int column) noexcept;
    void onCellChanged(int row, int column) noexcept;
    void onSelectionChanged() noexcept;

    void onInitialPositionPlacementChanged(bool enable) noexcept;
    void updateUserAircraftIndex() noexcept;
    void deleteAircraft() noexcept;

    void onRelativeDistanceChanged() noexcept;
    void updateReplayMode(int index) noexcept;
    void onReplayModeChanged(SkyConnectIntf::ReplayMode replayMode);

    void changeTimeOffset(const std::int64_t timeOffset) noexcept;
    void onTimeOffsetEditingFinished() noexcept;
    void resetAllTimeOffsets() noexcept;
};

#endif // FORMATIONWIDGET_H
