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
#include <algorithm>
#include <memory>
#include <cstdint>
#include <cmath>

#include <QByteArray>
#include <QDoubleValidator>
#include <QWidget>
#include <QTableWidget>
#include <QIcon>
#include <QLabel>
#include <QButtonGroup>
#include <QMessageBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Version.h>
#include <Kernel/Const.h>
#include <Kernel/Convert.h>
#include <Kernel/Unit.h>
#include <Kernel/SkyMath.h>
#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/AircraftType.h>
#include <Model/AircraftInfo.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/AircraftService.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/Connect.h>
#include <Widget/Platform.h>
#include "Formation.h"
#include "FormationWidget.h"
#include "ui_FormationWidget.h"

namespace
{
    constexpr int MinimumTableWidth {120};
    constexpr int InvalidRowIndex {-1};
    constexpr int InvalidColumnIndex {-1};
    constexpr int SequenceNumberColumnIndex {0};

    enum ReplayModeIndex {
        Normal,
        UserAircraftManualControl,
        FlyWithFormation
    };

    // Milliseconds
    constexpr std::int64_t SmallTimeOffset = 100;
    constexpr std::int64_t LargeTimeOffset = 1000;

    // Seconds
    constexpr double TimeOffsetMax = 24.0 * 60.0 * 60.0;
    constexpr double TimeOffsetMin = -TimeOffsetMax;
    constexpr double TimeOffsetDecimalPlaces = 2;
}

struct FormationWidgetPrivate
{
    FormationWidgetPrivate(FlightService &theFlightService, AircraftService &theAircraftService, QWidget &parent) noexcept
        : flightService(theFlightService),
          aircraftService(theAircraftService),
          positionButtonGroup(new QButtonGroup(&parent))
    {
        // We always initialise all icons at once, so checking only for
        // normalAircraftIcon is sufficient
        if (normalAircraftIcon.isNull()) {
            normalAircraftIcon = QIcon(":/img/icons/aircraft-normal.png");
            recordingAircraftIcon = QIcon(":/img/icons/aircraft-record-normal.png");
            referenceAircraftIcon = QIcon(":/img/icons/aircraft-reference-normal.png");
        }
        if (parent.devicePixelRatioF() >= 1.5) {
            userAircraftPixmap.load(":/img/icons/aircraft-normal@2x.png");
            userAircraftPixmap.setDevicePixelRatio(2.0);
            referenceAircraftPixmap.load(":/img/icons/aircraft-reference-normal@2x.png");
            referenceAircraftPixmap.setDevicePixelRatio(2.0);
        } else {
            userAircraftPixmap.load(":/img/icons/aircraft-normal.png");
            userAircraftPixmap.setDevicePixelRatio(1.0);
            referenceAircraftPixmap.load(":/img/icons/aircraft-reference-normal.png");
            referenceAircraftPixmap.setDevicePixelRatio(1.0);
        }
    }

    FlightService &flightService;
    AircraftService &aircraftService;

    int tailNumberColumnIndex {::InvalidColumnIndex};
    int timeOffsetColumnIndex {::InvalidColumnIndex};
    QButtonGroup *positionButtonGroup;
    int selectedRowIndex {::InvalidRowIndex};
    std::int64_t selectedAircraftIndex {Flight::InvalidId};
    QDoubleValidator *timeOffsetValidator {nullptr};
    Unit unit;

    // Only initialise once the PluginManager.qrc resources are available
    static inline QIcon normalAircraftIcon;
    static inline QIcon recordingAircraftIcon;
    static inline QIcon referenceAircraftIcon;

    // QPixmaps can only be created after QApplication (cannot be static)
    QPixmap userAircraftPixmap;
    QPixmap referenceAircraftPixmap;
};

// PUBLIC

FormationWidget::FormationWidget(FlightService &flightService, AircraftService &aircraftService, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::FormationWidget>()),
      d(std::make_unique<FormationWidgetPrivate>(flightService, aircraftService, *this))
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug() << "FormationWidget::FormationWidget: CREATED.";
#endif
}

FormationWidget::~FormationWidget() noexcept
{
    const QByteArray tableState = ui->aircraftTableWidget->horizontalHeader()->saveState();
    Settings::getInstance().setFormationAircraftTableState(tableState);
#ifdef DEBUG
    qDebug() << "FormationWidget::~FormationWidget: DELETED.";
#endif
}

Formation::HorizontalDistance FormationWidget::getHorizontalDistance() const noexcept
{
    return static_cast<Formation::HorizontalDistance>(ui->horizontalDistanceSlider->value());
}

Formation::VerticalDistance FormationWidget::getVerticalDistance() const noexcept
{
    return static_cast<Formation::VerticalDistance>(ui->verticalDistanceSlider->value());
}

Formation::RelativePosition FormationWidget::getRelativePosition() const noexcept
{
    return static_cast<Formation::RelativePosition>(d->positionButtonGroup->checkedId());
}

// PRIVATE

void FormationWidget::initUi() noexcept
{
    ui->aircraftTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    const QStringList headers {tr("Sequence"), tr("Aircraft"), tr("Engine Type"), tr("Wing Span"), tr("Initial Airspeed"), tr("Initial Altitude"), tr("Duration"), tr("Tail Number"), tr("Time Offset")};
    ui->aircraftTableWidget->setColumnCount(headers.count());
    ui->aircraftTableWidget->setHorizontalHeaderLabels(headers);
    ui->aircraftTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->aircraftTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->aircraftTableWidget->verticalHeader()->hide();
    ui->aircraftTableWidget->setMinimumWidth(::MinimumTableWidth);
    ui->aircraftTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->aircraftTableWidget->sortByColumn(::SequenceNumberColumnIndex, Qt::SortOrder::AscendingOrder);
    ui->aircraftTableWidget->horizontalHeader()->setSectionsMovable(true);
    ui->aircraftTableWidget->setAlternatingRowColors(true);

    QByteArray tableState = Settings::getInstance().getFormationAircraftTableState();
    ui->aircraftTableWidget->horizontalHeader()->restoreState(tableState);

    // Default position is south-east
    ui->sePositionRadioButton->setChecked(true);
    ui->horizontalDistanceSlider->setValue(Formation::HorizontalDistance::Nearby);
    ui->verticalDistanceSlider->setValue(Formation::VerticalDistance::Level);
    ui->relativePositionCheckBox->setChecked(Settings::getInstance().isRelativePositionPlacementEnabled());

    d->positionButtonGroup->addButton(ui->nPositionRadioButton, Formation::RelativePosition::North);
    d->positionButtonGroup->addButton(ui->nnePositionRadioButton, Formation::RelativePosition::NorthNorthEast);
    d->positionButtonGroup->addButton(ui->nePositionRadioButton, Formation::RelativePosition::NorthEast);
    d->positionButtonGroup->addButton(ui->enePositionRadioButton, Formation::RelativePosition::EastNorthEast);
    d->positionButtonGroup->addButton(ui->ePositionRadioButton, Formation::RelativePosition::East);
    d->positionButtonGroup->addButton(ui->esePositionRadioButton, Formation::RelativePosition::EastSouthEast);
    d->positionButtonGroup->addButton(ui->sePositionRadioButton, Formation::RelativePosition::SouthEast);
    d->positionButtonGroup->addButton(ui->ssePositionRadioButton, Formation::RelativePosition::SouthSouthEast);
    d->positionButtonGroup->addButton(ui->sPositionRadioButton, Formation::RelativePosition::South);
    d->positionButtonGroup->addButton(ui->sswPositionRadioButton, Formation::RelativePosition::SouthSouthWest);
    d->positionButtonGroup->addButton(ui->swPositionRadioButton, Formation::RelativePosition::SouthWest);
    d->positionButtonGroup->addButton(ui->wswPositionRadioButton, Formation::RelativePosition::WestSouthWest);
    d->positionButtonGroup->addButton(ui->wPositionRadioButton, Formation::RelativePosition::West);
    d->positionButtonGroup->addButton(ui->wnwPositionRadioButton, Formation::RelativePosition::WestNorthWest);
    d->positionButtonGroup->addButton(ui->nwPositionRadioButton, Formation::RelativePosition::NorthWest);
    d->positionButtonGroup->addButton(ui->nnwPositionRadioButton, Formation::RelativePosition::NorthNorthWest);

    const QString css = QStringLiteral(
"QRadioButton::indicator:unchecked {"
"    image: url(:/img/icons/aircraft-normal-off.png);"
"}"
"QRadioButton::indicator:checked {"
"    image: url(:/img/icons/aircraft-record-normal.png);"
"}");
    ui->nPositionRadioButton->setStyleSheet(css);
    ui->nnePositionRadioButton->setStyleSheet(css);
    ui->nePositionRadioButton->setStyleSheet(css);
    ui->enePositionRadioButton->setStyleSheet(css);
    ui->ePositionRadioButton->setStyleSheet(css);
    ui->esePositionRadioButton->setStyleSheet(css);
    ui->sePositionRadioButton->setStyleSheet(css);
    ui->ssePositionRadioButton->setStyleSheet(css);
    ui->sPositionRadioButton->setStyleSheet(css);
    ui->sswPositionRadioButton->setStyleSheet(css);
    ui->swPositionRadioButton->setStyleSheet(css);
    ui->wswPositionRadioButton->setStyleSheet(css);
    ui->wPositionRadioButton->setStyleSheet(css);
    ui->wnwPositionRadioButton->setStyleSheet(css);
    ui->nwPositionRadioButton->setStyleSheet(css);
    ui->nnwPositionRadioButton->setStyleSheet(css);

    ui->replayModeComboBox->insertItem(ReplayModeIndex::Normal, tr("Formation (Normal)"), Enum::toUnderlyingType(SkyConnectIntf::ReplayMode::Normal));
    ui->replayModeComboBox->insertItem(ReplayModeIndex::UserAircraftManualControl, tr("Take control of recorded user aircraft"), Enum::toUnderlyingType(SkyConnectIntf::ReplayMode::UserAircraftManualControl));
    ui->replayModeComboBox->insertItem(ReplayModeIndex::FlyWithFormation, tr("Fly with formation"), Enum::toUnderlyingType(SkyConnectIntf::ReplayMode::FlyWithFormation));

    initTimeOffsetUi();

    // Default "Delete" key deletes aircraft
    ui->deletePushButton->setShortcut(QKeySequence::Delete);

    ui->timeOffsetGroupBox->setStyleSheet(Platform::getFlatButtonCss());
}

void FormationWidget::initTimeOffsetUi() noexcept
{
    // Validation
    d->timeOffsetValidator = new QDoubleValidator(ui->timeOffsetLineEdit);
    d->timeOffsetValidator->setRange(::TimeOffsetMin, ::TimeOffsetMax, ::TimeOffsetDecimalPlaces);
}

void FormationWidget::frenchConnection() noexcept
{
    // Flight
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::userAircraftChanged,
            this, &FormationWidget::onUserAircraftChanged);
    connect(&flight, &Flight::aircraftRemoved,
            this, &FormationWidget::updateUi);
    connect(&flight, &Flight::flightStored,
            this, &FormationWidget::updateUi);
    connect(&flight, &Flight::aircraftInfoChanged,
            this, &FormationWidget::onAircraftInfoChanged);

    // Connection
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &FormationWidget::updateUi);
    connect(&skyConnectManager, &SkyConnectManager::replayModeChanged,
            this, &FormationWidget::onReplayModeChanged);

    // Aircraft table
    connect(ui->aircraftTableWidget, &QTableWidget::itemSelectionChanged,
            this, &FormationWidget::onSelectionChanged);
    connect(ui->aircraftTableWidget, &QTableWidget::cellDoubleClicked,
            this, &FormationWidget::onCellSelected);
    connect(ui->aircraftTableWidget, &QTableWidget::cellChanged,
            this, &FormationWidget::onCellChanged);
    connect(ui->userAircraftPushButton, &QPushButton::clicked,
            this, &FormationWidget::updateUserAircraftIndex);
    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &FormationWidget::deleteAircraft);
    connect(ui->relativePositionCheckBox, &QCheckBox::stateChanged,
            this, &FormationWidget::onInitialPositionPlacementChanged);

    // Relative position, replay mode
    connect(ui->horizontalDistanceSlider, &QSlider::valueChanged,
            this, &FormationWidget::onRelativeDistanceChanged);
    connect(ui->verticalDistanceSlider, &QSlider::valueChanged,
            this, &FormationWidget::onRelativeDistanceChanged);
    connect(d->positionButtonGroup, &QButtonGroup::idClicked,
            this, &FormationWidget::onRelativePositionChanged);
    connect(ui->replayModeComboBox, &QComboBox::activated,
            this, &FormationWidget::updateReplayMode);

    // Time offset
    connect(ui->fastBackwardOffsetPushButton, &QPushButton::clicked,
            this, [&] { changeTimeOffset(- ::LargeTimeOffset);});
    connect(ui->backwardOffsetPushButton, &QPushButton::clicked,
            this, [&] { changeTimeOffset(- ::SmallTimeOffset);});
    connect(ui->forwardOffsetPushButton, &QPushButton::clicked,
            this, [&] { changeTimeOffset(+ ::SmallTimeOffset);});
    connect(ui->fastForwardOffsetPushButton, &QPushButton::clicked,
            this, [&] { changeTimeOffset(+ ::LargeTimeOffset);});
    connect(ui->timeOffsetLineEdit, &QLineEdit::editingFinished,
            this, &FormationWidget::onTimeOffsetEditingFinished);
    connect(ui->resetAllTimeOffsetPushButton, &QPushButton::clicked,
            this, &FormationWidget::resetAllTimeOffsets);
}

void FormationWidget::updateAircraftTable() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();

    ui->aircraftTableWidget->blockSignals(true);
    ui->aircraftTableWidget->setSortingEnabled(false);
    ui->aircraftTableWidget->clearContents();
    ui->aircraftTableWidget->setRowCount(flight.count());

    int rowIndex {0};
    for (const auto &aircraft : flight) {
        updateAircraftRow(*aircraft, rowIndex);
        rowIndex++;
    }

    ui->aircraftTableWidget->setSortingEnabled(true);
    ui->aircraftTableWidget->resizeColumnsToContents();
    ui->aircraftTableWidget->blockSignals(false);

    if (d->selectedRowIndex != ::InvalidRowIndex) {
        d->selectedRowIndex = std::min(d->selectedRowIndex, ui->aircraftTableWidget->rowCount() - 1);
        ui->aircraftTableWidget->selectRow(d->selectedRowIndex);
    }

    updateAircraftIcons();
}

void FormationWidget::updateAircraftIcons() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const int userAircraftIndex = flight.getUserAircraftIndex();
    const SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const bool recording = skyConnectManager.isInRecordingState();
    const SkyConnectIntf::ReplayMode replayMode = skyConnectManager.getReplayMode();

    for (int row = 0; row < ui->aircraftTableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, ::SequenceNumberColumnIndex);
        if (row == userAircraftIndex) {
            if (recording) {
                item->setIcon(FormationWidgetPrivate::recordingAircraftIcon);
            } else if (replayMode == SkyConnectIntf::ReplayMode::FlyWithFormation) {
                item->setIcon(FormationWidgetPrivate::referenceAircraftIcon);
            } else {
                item->setIcon(FormationWidgetPrivate::normalAircraftIcon);
            }
            if (replayMode == SkyConnectIntf::ReplayMode::FlyWithFormation) {
                ui->referenceAircraftLabel->setPixmap(d->referenceAircraftPixmap);
            } else {
                ui->referenceAircraftLabel->setPixmap(d->userAircraftPixmap);
            }
        } else {
            item->setIcon(QIcon());
        }
    }
}

void FormationWidget::updateRelativePositionUi() noexcept
{
    switch (ui->horizontalDistanceSlider->value()) {
    case Formation::HorizontalDistance::VeryClose:
        ui->horizontalDistanceTextLabel->setText(tr("Very close"));
        break;
    case Formation::HorizontalDistance::Close:
        ui->horizontalDistanceTextLabel->setText(tr("Close"));
        break;
    case Formation::HorizontalDistance::Nearby:
        ui->horizontalDistanceTextLabel->setText(tr("Nearby"));
        break;
    case Formation::HorizontalDistance::Far:
        ui->horizontalDistanceTextLabel->setText(tr("Far"));
        break;
    case Formation::HorizontalDistance::VeryFar:
        ui->horizontalDistanceTextLabel->setText(tr("Very far"));
        break;
    }

    switch (ui->verticalDistanceSlider->value()) {
    case Formation::VerticalDistance::Below:
        ui->verticalDistanceTextLabel->setText(tr("Below"));
        break;
    case Formation::VerticalDistance::JustBelow:
        ui->verticalDistanceTextLabel->setText(tr("Just below"));
        break;
    case Formation::VerticalDistance::Level:
        ui->verticalDistanceTextLabel->setText(tr("Level"));
        break;
    case Formation::VerticalDistance::JustAbove:
        ui->verticalDistanceTextLabel->setText(tr("Just above"));
        break;
    case Formation::VerticalDistance::Above:
        ui->verticalDistanceTextLabel->setText(tr("Above"));
        break;
    }
}

void FormationWidget::updateEditUi() noexcept
{
    const bool inRecordingState = SkyConnectManager::getInstance().isInRecordingState();
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    bool userAircraftIndex = d->selectedAircraftIndex == flight.getUserAircraftIndex();
    ui->userAircraftPushButton->setEnabled(d->selectedAircraftIndex != Flight::InvalidId && !inRecordingState && !userAircraftIndex);
    const bool formation = flight.count() > 1;
    ui->deletePushButton->setEnabled(formation && !inRecordingState && d->selectedAircraftIndex != Flight::InvalidId);
}

void FormationWidget::updateTimeOffsetUi() noexcept
{
    const bool enabled = d->selectedAircraftIndex != Flight::InvalidId;

    ui->fastBackwardOffsetPushButton->setEnabled(enabled);
    ui->backwardOffsetPushButton->setEnabled(enabled);
    ui->timeOffsetLineEdit->setEnabled(enabled);
    ui->forwardOffsetPushButton->setEnabled(enabled);
    ui->fastBackwardOffsetPushButton->setEnabled(enabled);

    if (enabled) {
        const Flight &flight = Logbook::getInstance().getCurrentFlight();
        const Aircraft &aircraft = flight[d->selectedAircraftIndex];
        const std::int64_t timeOffset = aircraft.getAircraftInfo().timeOffset;
        const double timeOffsetSec = static_cast<double>(timeOffset) / 1000.0;
        QString offsetString = d->unit.formatNumber(timeOffsetSec, TimeOffsetDecimalPlaces);
        ui->timeOffsetLineEdit->setText(offsetString);
    } else {
        ui->timeOffsetLineEdit->setText("");
    }
}

void FormationWidget::updateReplayUi() noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    switch (SkyConnectManager::getInstance().getReplayMode()) {
    case SkyConnectIntf::ReplayMode::Normal:
        ui->replayModeComboBox->setCurrentIndex(ReplayModeIndex::Normal);
        break;
    case SkyConnectIntf::ReplayMode::UserAircraftManualControl:
        ui->replayModeComboBox->setCurrentIndex(ReplayModeIndex::UserAircraftManualControl);
        break;
    case SkyConnectIntf::ReplayMode::FlyWithFormation:
        ui->replayModeComboBox->setCurrentIndex(ReplayModeIndex::FlyWithFormation);
        break;
    }
    ui->replayModeComboBox->setEnabled(!skyConnectManager.isInRecordingState());
}

void FormationWidget::updateToolTips() noexcept
{
    // Relative positions
    QList<QAbstractButton *> buttons = d->positionButtonGroup->buttons();
    for (QAbstractButton *button : buttons) {
        if (button->isChecked()) {
            button->setToolTip(tr("Selected aircraft position for next recording."));
        } else {
            button->setToolTip(tr("Select aircraft position."));
        }
    }

    // Time offset
    if (d->selectedAircraftIndex != Flight::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const std::int64_t timeOffset = aircraft.getTimeOffset();
        if (timeOffset < 0) {
            ui->timeOffsetLineEdit->setToolTip(tr("The aircraft is %1 behind its recorded schedule.").arg(d->unit.formatElapsedTime(timeOffset)));
        } else if (timeOffset > 0) {
            ui->timeOffsetLineEdit->setToolTip(tr("The aircraft is %1 ahead its recorded schedule.").arg(d->unit.formatElapsedTime(timeOffset)));
        } else {
            ui->timeOffsetLineEdit->setToolTip(tr("Positive values [seconds] put the aircraft ahead, negative values put the aircraft behind its recorded schedule."));
        }
    }

    // Replay mode
    switch (ui->replayModeComboBox->currentIndex()) {
    case ReplayModeIndex::Normal:
        ui->replayModeComboBox->setToolTip(tr("%1 controls all recorded aircraft.").arg(Version::getApplicationName()));
        break;
    case ReplayModeIndex::UserAircraftManualControl:
        ui->replayModeComboBox->setToolTip(tr("Take control of the recorded user aircraft of the formation.\n"
                                              "The user aircraft (marked in blue) can be changed during replay."));
        break;
    case ReplayModeIndex::FlyWithFormation:
        ui->replayModeComboBox->setToolTip(tr("Fly with the currently loaded aircraft along with the entire formation.\n"
                                              "Reposition your user aircraft at any time, by either changing its relative position\n"
                                              "or choose another reference aircraft (marked in green) in the formation."));
        break;
    }
}

// PRIVATE SLOTS

void FormationWidget::updateAircraftRow(const Aircraft &aircraft, int rowIndex) noexcept
{
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();
    const QString tooltip = tr("Double-click to change user aircraft.");
    int columnIndex = 0;

    // Sequence
    std::unique_ptr<QTableWidgetItem> newItem = std::make_unique<QTableWidgetItem>();

    // Sequence numbers start at 1
    newItem->setData(Qt::DisplayRole, rowIndex + 1);
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newItem->setToolTip(tooltip);
    // Transfer ownership of newItem to table widget
    ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Aircraft type
    newItem = std::make_unique<QTableWidgetItem>(aircraftInfo.aircraftType.type);
    ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Engine type
    newItem = std::make_unique<QTableWidgetItem>(SimType::engineTypeToString(aircraftInfo.aircraftType.engineType));
    ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Wing span
    newItem = std::make_unique<QTableWidgetItem>(d->unit.formatFeet(aircraftInfo.aircraftType.wingSpan));
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Initial airspeed
    newItem = std::make_unique<QTableWidgetItem>(d->unit.formatKnots(aircraftInfo.initialAirspeed));
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Initial altitude above ground
    newItem = std::make_unique<QTableWidgetItem>(d->unit.formatFeet(aircraftInfo.altitudeAboveGround));
    newItem->setToolTip(tr("Altitude above ground."));
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Duration
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setData(Qt::DisplayRole, Unit::formatHHMMSS(aircraft.getDurationMSec()));
    newItem->setToolTip(tr("Recording duration."));
    ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    ++columnIndex;

    // Tail number
    newItem = std::make_unique<QTableWidgetItem>(aircraftInfo.tailNumber);
    newItem->setToolTip(tr("Double-click to edit tail number."));
    newItem->setBackground(Platform::getEditableTableCellBGColor());
    ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    d->tailNumberColumnIndex = columnIndex;
    ++columnIndex;

    // Time offset
    const double timeOffsetSec = static_cast<double>(aircraftInfo.timeOffset) / 1000.0;
    newItem = std::make_unique<QTableWidgetItem>(d->unit.formatNumber(timeOffsetSec, ::TimeOffsetDecimalPlaces));
    newItem->setToolTip(tr("Double-click to edit time offset [seconds]."));
    newItem->setBackground(Platform::getEditableTableCellBGColor());
    ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem.release());
    d->timeOffsetColumnIndex = columnIndex;
    ++columnIndex;

    ++rowIndex;
}

void FormationWidget::updateAndSendUserAircraftPosition() const noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();

    switch (skyConnectManager.getReplayMode())
    {
    case SkyConnectIntf::ReplayMode::Normal:
        break;
    case SkyConnectIntf::ReplayMode::UserAircraftManualControl:
    {
        if (!skyConnectManager.isInRecordingState()) {
            Flight &flight = Logbook::getInstance().getCurrentFlight();
            // Also update the manually flown user aircraft position
            const Aircraft &aircraft = flight.getUserAircraft();
            Position &position = aircraft.getPosition();
            const PositionData positionData = position.interpolate(skyConnectManager.getCurrentTimestamp(), TimeVariableData::Access::Seek);
            skyConnectManager.setUserAircraftPosition(positionData);
        }
        break;
    }
    case SkyConnectIntf::ReplayMode::FlyWithFormation:
        if (!skyConnectManager.isInRecordingState() && Settings::getInstance().isRelativePositionPlacementEnabled()) {
            const Formation::HorizontalDistance horizontalDistance {getHorizontalDistance()};
            const Formation::VerticalDistance verticalDistance {getVerticalDistance()};
            const Formation::RelativePosition relativePosition {getRelativePosition()};
            const PositionData positionData = Formation::calculateRelativePositionToUserAircraft(horizontalDistance,
                                                                                                 verticalDistance,
                                                                                                 relativePosition,
                                                                                                 skyConnectManager.getCurrentTimestamp());
            skyConnectManager.setUserAircraftPosition(positionData);
        }
        break;
    }
}

void FormationWidget::updateUserAircraftPosition(SkyConnectIntf::ReplayMode replayMode) const noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    if (Settings::getInstance().isRelativePositionPlacementEnabled()) {
        switch(replayMode) {
        case SkyConnectIntf::ReplayMode::Normal:
            break;
        case SkyConnectIntf::ReplayMode::UserAircraftManualControl:
        {
            Flight &flight = Logbook::getInstance().getCurrentFlight();
            const Aircraft &aircraft = flight.getUserAircraft();
            Position &position = aircraft.getPosition();
            const PositionData positionData = position.interpolate(skyConnectManager.getCurrentTimestamp(), TimeVariableData::Access::Seek);
            skyConnectManager.setUserAircraftPosition(positionData);
            break;
        }
        case SkyConnectIntf::ReplayMode::FlyWithFormation:
            const Formation::HorizontalDistance horizontalDistance {getHorizontalDistance()};
            const Formation::VerticalDistance verticalDistance {getVerticalDistance()};
            const Formation::RelativePosition relativePosition {getRelativePosition()};
            const PositionData positionData = Formation::calculateRelativePositionToUserAircraft(horizontalDistance,
                                                                                                 verticalDistance,
                                                                                                 relativePosition,
                                                                                                 skyConnectManager.getCurrentTimestamp());
            skyConnectManager.setUserAircraftPosition(positionData);
            break;
        }
    }
}

// PRIVATE SLOTS

void FormationWidget::updateUi() noexcept
{
    updateAircraftTable();
    updateRelativePositionUi();
    updateEditUi();    
    updateTimeOffsetUi();
    updateReplayUi();
    updateToolTips();
}

void FormationWidget::onRelativePositionChanged() noexcept
{
    updateToolTips();
    updateAndSendUserAircraftPosition();
}

void FormationWidget::onUserAircraftChanged() noexcept
{
    updateAircraftIcons();
    updateEditUi();
    updateAndSendUserAircraftPosition();
}

void FormationWidget::onAircraftInfoChanged() noexcept
{
    updateAircraftTable();
}

void FormationWidget::onCellSelected(int row, [[maybe_unused]] int column) noexcept
{
    if (column == d->tailNumberColumnIndex || column == d->timeOffsetColumnIndex) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, column);
        ui->aircraftTableWidget->editItem(item);
    } else {
        updateUserAircraftIndex();
    }
}

void FormationWidget::onCellChanged(int row, int column) noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Aircraft &aircraft = flight[d->selectedAircraftIndex];
    if (column == d->tailNumberColumnIndex) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, column);
        const QString tailNumber = item->data(Qt::EditRole).toString();
        d->aircraftService.changeTailNumber(aircraft, tailNumber);
    } else if (column == d->timeOffsetColumnIndex) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, column);
        bool ok {false};
        const double timeOffsetSec = item->data(Qt::EditRole).toDouble(&ok);
        if (ok) {
            const std::int64_t timeOffset = static_cast<std::int64_t>(std::round(timeOffsetSec * 1000.0));
            d->aircraftService.changeTimeOffset(aircraft, timeOffset);
        }
    }
}

void FormationWidget::onSelectionChanged() noexcept
{
    QItemSelectionModel *select = ui->aircraftTableWidget->selectionModel();
    QModelIndexList modelIndices = select->selectedRows(::SequenceNumberColumnIndex);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        d->selectedRowIndex = modelIndex.row();
        // Index starts at 0
        d->selectedAircraftIndex = ui->aircraftTableWidget->model()->data(modelIndex).toInt() - 1;
    } else {
        d->selectedRowIndex = ::InvalidRowIndex;
        d->selectedAircraftIndex = Flight::InvalidId;
    }
    updateEditUi();
    updateTimeOffsetUi();
    updateToolTips();
}

void FormationWidget::onInitialPositionPlacementChanged(bool enable) noexcept
{
    Settings::getInstance().setRelativePositionPlacementEnabled(enable);
}

void FormationWidget::updateUserAircraftIndex() noexcept
{
    if (!SkyConnectManager::getInstance().isInRecordingState()) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        if (d->selectedRowIndex != flight.getUserAircraftIndex()) {
            d->flightService.updateUserAircraftIndex(flight, d->selectedRowIndex);
        }
    }
}

void FormationWidget::deleteAircraft() noexcept
{
    Settings &settings = Settings::getInstance();
    bool doDelete {true};
    if (settings.isDeleteAircraftConfirmationEnabled()) {
        std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
        QCheckBox *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), messageBox.get());

        // Sequence numbers start at 1
        messageBox->setWindowTitle(tr("Delete Aircraft"));
        messageBox->setText(tr("The aircraft with sequence number %1 is about to be deleted. Do you want to delete the aircraft?").arg(d->selectedRowIndex + 1));
        messageBox->setInformativeText(tr("Deletion cannot be undone."));
        QPushButton *deleteButton = messageBox->addButton(tr("&Delete"), QMessageBox::AcceptRole);
        QPushButton *keepButton = messageBox->addButton(tr("&Keep"), QMessageBox::RejectRole);
        messageBox->setDefaultButton(keepButton);
        messageBox->setCheckBox(dontAskAgainCheckBox);
        messageBox->setIcon(QMessageBox::Icon::Question);

        messageBox->exec();
        doDelete = messageBox->clickedButton() == deleteButton;
        settings.setDeleteAircraftConfirmationEnabled(!dontAskAgainCheckBox->isChecked());
    }

    if (doDelete) {
        d->aircraftService.deleteByIndex(d->selectedRowIndex);
        ui->aircraftTableWidget->setFocus(Qt::NoFocusReason);
    }
}

void FormationWidget::onRelativeDistanceChanged() noexcept
{
    updateRelativePositionUi();
    onRelativePositionChanged();
}

void FormationWidget::updateReplayMode(int index) noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    switch(index) {
    case ReplayModeIndex::Normal:
        skyConnectManager.setReplayMode(SkyConnectIntf::ReplayMode::Normal);
        break;
    case ReplayModeIndex::UserAircraftManualControl:
        skyConnectManager.setReplayMode(SkyConnectIntf::ReplayMode::UserAircraftManualControl);
        break;
    case ReplayModeIndex::FlyWithFormation:
        skyConnectManager.setReplayMode(SkyConnectIntf::ReplayMode::FlyWithFormation);
        break;
    }
    updateUserAircraftPosition(skyConnectManager.getReplayMode());
    updateUi();
}

void FormationWidget::onReplayModeChanged(SkyConnectIntf::ReplayMode replayMode)
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    switch(replayMode) {
    case SkyConnectIntf::ReplayMode::Normal:
        ui->replayModeComboBox->setCurrentIndex(ReplayModeIndex::Normal);
        break;
    case SkyConnectIntf::ReplayMode::UserAircraftManualControl:
    {
        ui->replayModeComboBox->setCurrentIndex(ReplayModeIndex::UserAircraftManualControl);
        break;
    }
    case SkyConnectIntf::ReplayMode::FlyWithFormation:
        ui->replayModeComboBox->setCurrentIndex(ReplayModeIndex::FlyWithFormation);
        break;
    }
    updateUserAircraftPosition(replayMode);
}

void FormationWidget::changeTimeOffset(const std::int64_t timeOffset) noexcept
{
    if (d->selectedAircraftIndex != Flight::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const std::int64_t newTimeOffset = aircraft.getTimeOffset() + timeOffset;
        d->aircraftService.changeTimeOffset(aircraft, newTimeOffset);
        updateToolTips();
    }
}

void FormationWidget::onTimeOffsetEditingFinished() noexcept
{
    if (d->selectedAircraftIndex != Flight::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        bool ok {false};
        const double timeOffsetSec = ui->timeOffsetLineEdit->text().toDouble(&ok);
        if (ok) {
            const std::int64_t timeOffset = static_cast<std::int64_t>(std::round(timeOffsetSec * 1000.0));
            d->aircraftService.changeTimeOffset(aircraft, timeOffset);
            updateToolTips();
        }
    }
}

void FormationWidget::resetAllTimeOffsets() noexcept
{
    Settings &settings = Settings::getInstance();
    bool doReset {true};
    if (settings.isResetTimeOffsetConfirmationEnabled()) {
        std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
        QCheckBox *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), messageBox.get());

        messageBox->setWindowTitle(tr("Reset Time Offsets"));
        messageBox->setText(tr("Do you want to reset all time offsets to 0?"));
        messageBox->setInformativeText(tr("The time offsets of all aircraft in this formation will be changed."));
        QPushButton *resetButton = messageBox->addButton(tr("&Reset Time Offsets"), QMessageBox::AcceptRole);
        QPushButton *doNotChangeButon = messageBox->addButton(tr("Do &Not Change"), QMessageBox::RejectRole);
        messageBox->setDefaultButton(doNotChangeButon);
        messageBox->setCheckBox(dontAskAgainCheckBox);
        messageBox->setIcon(QMessageBox::Icon::Question);

        messageBox->exec();
        doReset = messageBox->clickedButton() == resetButton;
        settings.setResetTimeOffsetConfirmationEnabled(!dontAskAgainCheckBox->isChecked());
    }
    if (doReset) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        bool ok = true;
        for (auto &aircraft : flight) {
            ok = d->aircraftService.changeTimeOffset(*aircraft.get(), 0);
            if (!ok) {
                break;
            }
        }
    }
}
