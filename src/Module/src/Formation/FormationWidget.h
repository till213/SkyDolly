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

class QShowEvent;
class QHideEvent;
class QAction;

#include <Model/InitialPosition.h>
#include <ModuleIntf.h>
#include <AbstractModuleWidget.h>

class Aircraft;
struct PositionData;

namespace Ui {
    class FormationWidget;
}

class FlightService;
class AircraftService;
class FormationWidgetPrivate;

class FormationWidget : public AbstractModuleWidget
{
    Q_OBJECT
public:
    explicit FormationWidget(FlightService &flightService, QWidget *parent = nullptr) noexcept;
    virtual ~FormationWidget() noexcept;

    virtual Module::Module getModuleId() const noexcept override;
    virtual const QString getModuleName() const noexcept override;
    virtual QAction &getAction() noexcept override;

protected:
    virtual void showEvent(QShowEvent *event) noexcept override;
    virtual void hideEvent(QHideEvent *event) noexcept override;

    virtual void onStartRecording() noexcept override;
    virtual void onStartReplay() noexcept override;

protected slots:
    virtual void handleRecordingStopped() noexcept override;

private:
    Q_DISABLE_COPY(FormationWidget)
    std::unique_ptr<Ui::FormationWidget> ui;
    std::unique_ptr<FormationWidgetPrivate> d;

    void initUi() noexcept;
    void initTimeOffsetUi() noexcept;
    void frenchConnection() noexcept;
    void updateInitialPositionUi() noexcept;

    InitialPosition calculateRelativeInitialPositionToUserAircraft(std::int64_t timestamp) const noexcept;
    PositionData calculateRelativePositionToUserAircraft(std::int64_t timestamp) const noexcept;

    static const QString getName();

private slots:
    void updateUi() noexcept;
    void updateEditUi() noexcept;
    void updateRelativePosition() noexcept;
    void updateTimeOffsetUi() noexcept;
    void updateReplayUi() noexcept;
    void updateToolTips() noexcept;

    void handleUserAircraftChanged() noexcept;
    void handleAircraftInfoChanged() noexcept;
    void handleCellSelected(int row, int column) noexcept;
    void handleCellChanged(int row, int column) noexcept;
    void onSelectionChanged() noexcept;
    void updateUserAircraftIndex() noexcept;
    void deleteAircraft() noexcept;

    void updateRelativeDistance() noexcept;
    void updateReplayMode(int index) noexcept;
    void changeTimeOffset(const std::int64_t timeOffset) noexcept;
    void onTimeOffsetEditingFinished() noexcept;
    void resetAllTimeOffsets() noexcept;
};

#endif // FORMATIONWIDGET_H
