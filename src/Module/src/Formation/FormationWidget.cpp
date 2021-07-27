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

#include <QCoreApplication>
#include <QDoubleValidator>
#include <QWidget>
#include <QAction>
#include <QTableWidget>
#include <QIcon>
#include <QButtonGroup>
#include <QMessageBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

#include "../../../Kernel/src/Version.h"
#include "../../../Kernel/src/Const.h"
#include "../../../Kernel/src/Unit.h"
#include "../../../Kernel/src/SkyMath.h"
#include "../../../Kernel/src/Enum.h"
#include "../../../Kernel/src/Settings.h"
#include "../../../Model/src/Logbook.h"
#include "../../../Model/src/Flight.h"
#include "../../../Model/src/AircraftType.h"
#include "../../../Model/src/AircraftInfo.h"
#include "../../../Model/src/Position.h"
#include "../../../Model/src/PositionData.h"
#include "../../../Model/src/InitialPosition.h"
#include "../../../Persistence/src/Service/FlightService.h"
#include "../../../Persistence/src/Service/AircraftService.h"
#include "../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../Widget/src/Platform.h"
#include "../AbstractModuleWidget.h"
#include "FormationWidget.h"
#include "ui_FormationWidget.h"

namespace
{
    constexpr int MinimumTableWidth = 600;
    constexpr int InvalidSelection = -1;
    constexpr int InvalidColumn = -1;
    constexpr int SequenceNumberColumn = 0;

    enum HorizontalDistance {
        VeryClose = 0,
        Close,
        Nearby,
        Far,
        VeryFar
    };

    enum VerticalDistance {
        Below = 0,
        JustBelow,
        Level,
        JustAbove,
        Above
    };

    enum RelativePosition {
        North,
        NorthNorthEast,
        NorthEast,
        EastNorthEast,
        East,
        EastSouthEast,
        SouthEast,
        SouthSouthEast,
        South,
        SouthSouthWest,
        SouthWest,
        WestSouthWest,
        West,
        WestNorthWest,
        NorthWest,
        NorthNorthWest
    };

    enum ReplayMode {
        NormalIndex,
        ManualControlUserAircraftIndex,
        FlyWithFormationIndex
    };

    // Milliseconds
    constexpr qint64 TimeOffsetIncrease = 100;
    constexpr qint64 TimeOffsetIncreaseLarge = 1000;
    constexpr qint64 TimeOffsetDecrease = 100;
    constexpr qint64 TimeOffsetDecreaseLarge = 1000;

    // Seconds
    constexpr double TimeOffsetMax = 24.0 * 60.0 * 60.0;
    constexpr double TimeOffsetMin = -TimeOffsetMax;
    constexpr double TimeOffsetDecimalPlaces = 2;
}

class FormationWidgetPrivate
{
public:
    FormationWidgetPrivate(QObject *parent) noexcept
        : tailNumberColumnIndex(InvalidColumn),
          timeOffsetColumnIndex(InvalidColumn),
          positionButtonGroup(new QButtonGroup(parent)),
          moduleAction(nullptr),
          aircraftService(std::make_unique<AircraftService>()),
          selectedRow(InvalidSelection),
          selectedAircraftIndex(Flight::InvalidId),
          timeOffsetValidator(nullptr)
    {}

    int tailNumberColumnIndex;
    int timeOffsetColumnIndex;
    QButtonGroup *positionButtonGroup;
    std::unique_ptr<QAction> moduleAction;
    std::unique_ptr<AircraftService> aircraftService;
    int selectedRow;
    qint64 selectedAircraftIndex;
    QDoubleValidator *timeOffsetValidator;
    Unit unit;
};

// PUBLIC

FormationWidget::FormationWidget(FlightService &flightService, QWidget *parent) noexcept
    : AbstractModuleWidget(flightService, parent),
      ui(std::make_unique<Ui::FormationWidget>()),
      d(std::make_unique<FormationWidgetPrivate>(this))
{
    ui->setupUi(this);
    initUi();
    frenchConnection();
}

FormationWidget::~FormationWidget() noexcept
{
#ifdef DEBUG
    qDebug("FormationWidget::~FormationWidget: DELETED.");
#endif
}

Module::Module FormationWidget::getModuleId() const noexcept
{
    return Module::Module::Formation;
}

const QString FormationWidget::getModuleName() const noexcept
{
    return getName();
}

QAction &FormationWidget::getAction() noexcept
{
    return *d->moduleAction;
}

// PROTECTED

void FormationWidget::showEvent(QShowEvent *event) noexcept
{
    AbstractModuleWidget::showEvent(event);

    // Deselect when showing module
    d->selectedRow = InvalidSelection;
    d->selectedAircraftIndex = Flight::InvalidId;

    Flight &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::userAircraftChanged,
            this, &FormationWidget::handleUserAircraftChanged);
    connect(&flight, &Flight::aircraftDeleted,
            this, &FormationWidget::updateUi);
    connect(&flight, &Flight::flightStored,
            this, &FormationWidget::updateUi);
    connect(&flight, &Flight::aircraftInfoChanged,
            this, &FormationWidget::updateUi);
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    connect(&skyConnectManager, &SkyConnectManager::stateChanged,
            this, &FormationWidget::updateUi);

    // Also updates the UI
    handleUserAircraftChanged(flight.getUserAircraft());
    handleSelectionChanged();    
}

void FormationWidget::hideEvent(QHideEvent *event) noexcept
{
    AbstractModuleWidget::hideEvent(event);

    Flight &flight = Logbook::getInstance().getCurrentFlight();
    disconnect(&flight, &Flight::userAircraftChanged,
               this, &FormationWidget::handleUserAircraftChanged);
    disconnect(&flight, &Flight::aircraftDeleted,
               this, &FormationWidget::updateUi);
    disconnect(&flight, &Flight::flightStored,
               this, &FormationWidget::updateUi);
    disconnect(&flight, &Flight::aircraftInfoChanged,
               this, &FormationWidget::updateUi);
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    disconnect(&skyConnectManager, &SkyConnectManager::stateChanged,
               this, &FormationWidget::updateUi);
}

void FormationWidget::onStartRecording() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        // The initial recording position is calculated for timestamp = 0 ("at the beginning")
        const InitialPosition initialPosition = calculateRelativeInitialPositionToUserAircraft(0);
        skyConnect->get().startRecording(SkyConnectIntf::RecordingMode::AddToFormation, initialPosition);
    }
}

void FormationWidget::onStartReplay() noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnectOptional = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnectOptional) {
        SkyConnectIntf &skyConnect = skyConnectOptional->get();
        const bool fromStart = skyConnect.isAtEnd();
        const qint64 timestamp = fromStart ? 0 : skyConnect.getCurrentTimestamp();
        const InitialPosition initialPosition = calculateRelativeInitialPositionToUserAircraft(timestamp);
        skyConnect.startReplay(fromStart, initialPosition);
    }
}

// PROTECTED SLOTS

void FormationWidget::handleRecordingStopped() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    const int count = flight.count();
    if (count > 1) {
        // Sequence starts at 1
        d->aircraftService->store(flight.getId(), count, flight[count - 1]);
    } else {
        AbstractModuleWidget::handleRecordingStopped();
    }
    updateRelativePosition();
}

// PRIVATE

void FormationWidget::initUi() noexcept
{
    d->moduleAction = std::make_unique<QAction>(getName());
    d->moduleAction->setCheckable(true);

    ui->aircraftTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    const QStringList headers {tr("Sequence"), tr("Aircraft"), tr("Engine Type"), tr("Wing Span"), tr("Initial Airspeed"), tr("Initial Altitude"), tr("Duration"), tr("Tail Number"), tr("Time Offset")};
    ui->aircraftTableWidget->setColumnCount(headers.count());
    ui->aircraftTableWidget->setHorizontalHeaderLabels(headers);
    ui->aircraftTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->aircraftTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->aircraftTableWidget->verticalHeader()->hide();
    ui->aircraftTableWidget->setMinimumWidth(MinimumTableWidth);
    ui->aircraftTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->aircraftTableWidget->sortByColumn(SequenceNumberColumn, Qt::SortOrder::AscendingOrder);

    // Default position is south-east
    ui->sePositionRadioButton->setChecked(true);
    ui->horizontalDistanceSlider->setValue(HorizontalDistance::Nearby);
    ui->verticalDistanceSlider->setValue(VerticalDistance::Level);

    d->positionButtonGroup->addButton(ui->nPositionRadioButton, RelativePosition::North);
    d->positionButtonGroup->addButton(ui->nnePositionRadioButton, RelativePosition::NorthNorthEast);
    d->positionButtonGroup->addButton(ui->nePositionRadioButton, RelativePosition::NorthEast);
    d->positionButtonGroup->addButton(ui->enePositionRadioButton, RelativePosition::EastNorthEast);
    d->positionButtonGroup->addButton(ui->ePositionRadioButton, RelativePosition::East);
    d->positionButtonGroup->addButton(ui->esePositionRadioButton, RelativePosition::EastSouthEast);
    d->positionButtonGroup->addButton(ui->sePositionRadioButton, RelativePosition::SouthEast);
    d->positionButtonGroup->addButton(ui->ssePositionRadioButton, RelativePosition::SouthSouthEast);
    d->positionButtonGroup->addButton(ui->sPositionRadioButton, RelativePosition::South);
    d->positionButtonGroup->addButton(ui->sswPositionRadioButton, RelativePosition::SouthSouthWest);
    d->positionButtonGroup->addButton(ui->swPositionRadioButton, RelativePosition::SouthWest);
    d->positionButtonGroup->addButton(ui->wswPositionRadioButton, RelativePosition::WestSouthWest);
    d->positionButtonGroup->addButton(ui->wPositionRadioButton, RelativePosition::West);
    d->positionButtonGroup->addButton(ui->wnwPositionRadioButton, RelativePosition::WestNorthWest);
    d->positionButtonGroup->addButton(ui->nwPositionRadioButton, RelativePosition::NorthWest);
    d->positionButtonGroup->addButton(ui->nnwPositionRadioButton, RelativePosition::NorthNorthWest);

    const QString css =
"QRadioButton::indicator:unchecked {"
"    image: url(:/img/icons/aircraft-normal-off.png);"
"}"
"QRadioButton::indicator:checked {"
"    image: url(:/img/icons/aircraft-record-normal.png);"
"}";
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

    ui->replayModeComboBox->insertItem(ReplayMode::NormalIndex, tr("Formation (Normal)"), Enum::toUnderlyingType(SkyConnectIntf::ReplayMode::Normal));
    ui->replayModeComboBox->insertItem(ReplayMode::ManualControlUserAircraftIndex, tr("Take control of recorded user aircraft"), Enum::toUnderlyingType(SkyConnectIntf::ReplayMode::UserAircraftManualControl));
    ui->replayModeComboBox->insertItem(ReplayMode::FlyWithFormationIndex, tr("Fly with formation"), Enum::toUnderlyingType(SkyConnectIntf::ReplayMode::FlyWithFormation));

    initTimeOffsetUi();
}

void FormationWidget::initTimeOffsetUi() noexcept
{
    // Validation
    d->timeOffsetValidator = new QDoubleValidator(ui->timeOffsetLineEdit);
    d->timeOffsetValidator->setRange(TimeOffsetMin, TimeOffsetMax, TimeOffsetDecimalPlaces);
}

void FormationWidget::frenchConnection() noexcept
{
    connect(ui->aircraftTableWidget, &QTableWidget::itemSelectionChanged,
            this, &FormationWidget::handleSelectionChanged);
    connect(ui->aircraftTableWidget, &QTableWidget::cellDoubleClicked,
            this, &FormationWidget::handleCellSelected);
    connect(ui->aircraftTableWidget, &QTableWidget::cellChanged,
            this, &FormationWidget::handleCellChanged);
    connect(ui->userAircraftPushButton, &QPushButton::clicked,
            this, &FormationWidget::updateUserAircraftIndex);
    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &FormationWidget::deleteAircraft);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(d->positionButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &FormationWidget::updateRelativePosition);
#else
    connect(d->positionButtonGroup, &QButtonGroup::idClicked,
            this, &FormationWidget::updateRelativePosition);
#endif
}

void FormationWidget::updateInitialPositionUi() noexcept
{
    switch (ui->horizontalDistanceSlider->value()) {
    case HorizontalDistance::VeryClose:
        ui->horizontalDistanceTextLabel->setText(tr("Very close"));
        break;
    case HorizontalDistance::Close:
        ui->horizontalDistanceTextLabel->setText(tr("Close"));
        break;
    case HorizontalDistance::Nearby:
        ui->horizontalDistanceTextLabel->setText(tr("Nearby"));
        break;
    case HorizontalDistance::Far:
        ui->horizontalDistanceTextLabel->setText(tr("Far"));
        break;
    default:
        ui->horizontalDistanceTextLabel->setText(tr("Very far"));
        break;
    }

    switch (ui->verticalDistanceSlider->value()) {
    case VerticalDistance::Below:
        ui->verticalDistanceTextLabel->setText(tr("Below"));
        break;
    case VerticalDistance::JustBelow:
        ui->verticalDistanceTextLabel->setText(tr("Just below"));
        break;
    case VerticalDistance::Level:
        ui->verticalDistanceTextLabel->setText(tr("Level"));
        break;
    case VerticalDistance::JustAbove:
        ui->verticalDistanceTextLabel->setText(tr("Just above"));
        break;
    default:
        ui->verticalDistanceTextLabel->setText(tr("Above"));
        break;
    }
}

InitialPosition FormationWidget::calculateRelativeInitialPositionToUserAircraft(qint64 timestamp) const noexcept
{
    InitialPosition initialPosition;
    const PositionData &relativePositionData = calculateRelativePositionToUserAircraft(timestamp);

    initialPosition.fromPositionData(relativePositionData);
    if (timestamp == 0) {
        const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
        const Aircraft &userAircraft = flight.getUserAircraftConst();
        const AircraftInfo &aircraftInfo = userAircraft.getAircraftInfoConst();
        initialPosition.fromAircraftInfo(aircraftInfo);
    } else {
        initialPosition.airspeed = qRound(relativePositionData.velocityBodyZ);
        initialPosition.onGround = false;
    }

    return initialPosition;
}

PositionData FormationWidget::calculateRelativePositionToUserAircraft(qint64 timestamp) const noexcept
{
    PositionData initialPosition;

    const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
    const Aircraft &userAircraft = flight.getUserAircraftConst();
    const Position &position = userAircraft.getPositionConst();
    const PositionData &positionData = timestamp == 0 ? position.getFirst() : position.interpolate(timestamp, TimeVariableData::Access::Seek);
    if (!positionData.isNull()) {

        const AircraftInfo &aircraftInfo = userAircraft.getAircraftInfoConst();
        const AircraftType &aircraftType = aircraftInfo.aircraftType;

        // Copy pitch, bank and heading
        initialPosition = positionData;

        // Horizontal distance [feet]
        double distance;
        switch (ui->horizontalDistanceSlider->value()) {
        case HorizontalDistance::VeryClose:
            // Aircrafts one wing apart
            distance = 1.5 * aircraftType.wingSpan;
            break;
        case HorizontalDistance::Close:
            // Aircrafts one wingspan
            distance = 2.0 * aircraftType.wingSpan;
            break;
        case HorizontalDistance::Nearby:
            // Aircrafts two wingspans
            distance = 3.0 * aircraftType.wingSpan;
            break;
        case HorizontalDistance::Far:
            // Aircrafts three wingspans apart
            distance = 4.0 * aircraftType.wingSpan;
            break;
        default:
            // Aircrafts four wingspans apart
            distance = 5.0 * aircraftType.wingSpan;
            break;
        }

        // Vertical distance [feet]
        const std::pair sourcePosition(positionData.latitude, positionData.longitude);
        double deltaAltitude;
        switch (ui->verticalDistanceSlider->value()) {
        case VerticalDistance::Below:
            deltaAltitude = -distance;
            break;
        case VerticalDistance::JustBelow:
            deltaAltitude = -distance / 2.0;
            break;
        case VerticalDistance::JustAbove:
            deltaAltitude = +distance / 2.0;
            break;
        case VerticalDistance::Above:
            deltaAltitude = +distance;
            break;
        default:
            deltaAltitude = 0.0;
            break;
        }
        const double altitude = positionData.altitude + deltaAltitude;

        // Degrees
        double bearing;
        switch (d->positionButtonGroup->checkedId()) {
        case RelativePosition::North:
            bearing = 0.0;
            break;
        case RelativePosition::NorthNorthEast:
            bearing = 22.5;
            break;
        case RelativePosition::NorthEast:
            bearing = 45.0;
            break;
        case RelativePosition::EastNorthEast:
            bearing = 67.5;
            break;
        case RelativePosition::East:
            bearing = 90.0;
            break;
        case RelativePosition::EastSouthEast:
            bearing = 112.5;
            break;
        case RelativePosition::SouthEast:
            bearing = 135.0;
            break;
        case RelativePosition::SouthSouthEast:
            bearing = 157.5;
            break;
        case RelativePosition::South:
            bearing = 180.0;
            break;
        case RelativePosition::SouthSouthWest:
            bearing = 202.5;
            break;
        case RelativePosition::SouthWest:
            bearing = 225.0;
            break;
        case RelativePosition::WestSouthWest:
            bearing = 247.5;
            break;
        case RelativePosition::West:
            bearing = 270.0;
            break;
        case RelativePosition::WestNorthWest:
            bearing = 292.5;
            break;
        case RelativePosition::NorthWest:
            bearing = 315.0;
            break;
        case RelativePosition::NorthNorthWest:
            bearing = 337.5;
            break;
        default:
            bearing = 0.0;
            break;
        }
        bearing += positionData.heading;
        std::pair initial = SkyMath::relativePosition(sourcePosition, SkyMath::feetToMeters(altitude), bearing, SkyMath::feetToMeters(distance));

        initialPosition.latitude = initial.first;
        initialPosition.longitude = initial.second;
        initialPosition.altitude = altitude;

    } // positionData is not null

    return initialPosition;
}

const QString FormationWidget::getName()
{
    return QCoreApplication::translate("FormationWidget", "Formation");
}

// PRIVATE SLOTS

void FormationWidget::updateUi() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();

    ui->aircraftTableWidget->blockSignals(true);
    ui->aircraftTableWidget->setSortingEnabled(false);
    ui->aircraftTableWidget->clearContents();
    ui->aircraftTableWidget->setRowCount(flight.count());
    int rowIndex = 0;
    const int userAircraftIndex = flight.getUserAircraftIndex();
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const bool recording = skyConnect && skyConnect->get().isRecording();
    const QString tooltip = tr("Double-click to change user aircraft");
    for (const auto &aircraft : flight) {

        const AircraftInfo &aircraftInfo = aircraft->getAircraftInfoConst();
        int columnIndex = 0;

        // Sequence
        QTableWidgetItem *newItem = new QTableWidgetItem();
        if (rowIndex == userAircraftIndex) {
            QIcon icon;
            if (recording) {
                icon = QIcon(":/img/icons/aircraft-record-normal.png");
            } else {
                icon = QIcon(":/img/icons/aircraft-normal.png");
            }
            newItem->setIcon(icon);
        }
        // Sequence numbers start at 1
        newItem->setData(Qt::DisplayRole, rowIndex + 1);
        newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        newItem->setToolTip(tooltip);
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Aircraft type
        newItem = new QTableWidgetItem(aircraftInfo.aircraftType.type);
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Engine type
        newItem = new QTableWidgetItem(SimType::engineTypeToString(aircraftInfo.aircraftType.engineType));
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Wing span
        newItem = new QTableWidgetItem(d->unit.formatFeet(aircraftInfo.aircraftType.wingSpan));
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ++columnIndex;

        // Initial airspeed
        newItem = new QTableWidgetItem(d->unit.formatKnots(aircraftInfo.initialAirspeed));
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ++columnIndex;

        // Initial altitude above ground
        newItem = new QTableWidgetItem(d->unit.formatFeet(aircraftInfo.altitudeAboveGround));
        newItem->setToolTip(tr("Altitude above ground"));
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ++columnIndex;

        // Duration
        newItem = new QTableWidgetItem();
        newItem->setData(Qt::DisplayRole, Unit::formatHHMMSS(aircraft->getDurationMSec()));
        newItem->setToolTip(tr("Hours:Minutes:Seconds"));
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Tail number
        newItem = new QTableWidgetItem(aircraftInfo.tailNumber);
        newItem->setToolTip(tr("Double-click to edit tail number."));
        newItem->setBackground(Platform::getEditableTableCellBGColor());
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        d->tailNumberColumnIndex = columnIndex;
        ++columnIndex;

        // Time offset
        const double timeOffsetSec = static_cast<double>(aircraftInfo.timeOffset) / 1000.0;
        newItem = new QTableWidgetItem(d->unit.formatNumber(timeOffsetSec, TimeOffsetDecimalPlaces));
        newItem->setToolTip(tr("Double-click to edit time offset [seconds]."));
        newItem->setBackground(Platform::getEditableTableCellBGColor());
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        d->timeOffsetColumnIndex = columnIndex;
        ++columnIndex;

        ++rowIndex;
    }

    ui->aircraftTableWidget->setSortingEnabled(true);
    ui->aircraftTableWidget->resizeColumnsToContents();
    ui->aircraftTableWidget->blockSignals(false);

    if (d->selectedRow != InvalidSelection) {
        d->selectedRow = qMin(d->selectedRow, ui->aircraftTableWidget->rowCount() - 1);
        ui->aircraftTableWidget->selectRow(d->selectedRow);
    }

    updateEditUi();
    updateInitialPositionUi();
    updateTimeOffsetUi();
    updateReplayUi();
}

void FormationWidget::updateEditUi() noexcept
{
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const bool inRecordingMode = skyConnect && skyConnect->get().inRecordingMode();
    const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
    bool userAircraftIndex = d->selectedAircraftIndex == flight.getUserAircraftIndex();
    ui->userAircraftPushButton->setEnabled(d->selectedAircraftIndex != Flight::InvalidId && !userAircraftIndex);
    const bool formation = flight.count() > 1;
    ui->deletePushButton->setEnabled(formation && !inRecordingMode && d->selectedAircraftIndex != Flight::InvalidId);
}

void FormationWidget::updateRelativePosition() noexcept
{
    QList<QAbstractButton *> buttons = d->positionButtonGroup->buttons();
    for (QAbstractButton *button : buttons) {
        if (button->isChecked()) {
            button->setToolTip(tr("Selected aircraft position for next recording."));
        } else {
            button->setToolTip(tr("Select aircraft position."));
        }
    }

    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnectOptional = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnectOptional) {
        SkyConnectIntf &skyConnect = skyConnectOptional->get();
        // When "Fly with formation" is paused also update the manually flown user aircraft position ("at the current timestamp")
        if (skyConnect.getReplayMode() == SkyConnectIntf::ReplayMode::FlyWithFormation && skyConnect.getState() == Connect::State::ReplayPaused) {
            const InitialPosition initialPosition = calculateRelativeInitialPositionToUserAircraft(skyConnect.getCurrentTimestamp());
            skyConnect.setUserAircraftInitialPosition(initialPosition);
        }
    }
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
        const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
        const Aircraft &aircraft = flight[d->selectedAircraftIndex];
        const qint64 timeOffset = aircraft.getAircraftInfoConst().timeOffset;
        const double timeOffsetSec = static_cast<double>(timeOffset) / 1000.0;
        QString offsetString = d->unit.formatNumber(timeOffsetSec, TimeOffsetDecimalPlaces);
        ui->timeOffsetLineEdit->setText(offsetString);
    } else {
        ui->timeOffsetLineEdit->setText("");
    }
}

void FormationWidget::updateReplayUi() noexcept
{
    const std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        switch (skyConnect->get().getReplayMode()) {
        case SkyConnectIntf::ReplayMode::Normal:
            ui->replayModeComboBox->setCurrentIndex(ReplayMode::NormalIndex);
            break;
        case SkyConnectIntf::ReplayMode::UserAircraftManualControl:
            ui->replayModeComboBox->setCurrentIndex(ReplayMode::ManualControlUserAircraftIndex);
            break;
        case SkyConnectIntf::ReplayMode::FlyWithFormation:
            ui->replayModeComboBox->setCurrentIndex(ReplayMode::FlyWithFormationIndex);
            break;
        default:
            ui->replayModeComboBox->setCurrentIndex(ReplayMode::NormalIndex);
            break;
        }
    } else {
        ui->replayModeComboBox->setCurrentIndex(ReplayMode::NormalIndex);
    }
    updateToolTips();
}

void FormationWidget::updateToolTips() noexcept
{
    if (d->selectedAircraftIndex != Flight::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const qint64 timeOffset = aircraft.getTimeOffset();
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
    case ReplayMode::NormalIndex:
        ui->replayModeComboBox->setToolTip(tr("%1 controls all recorded aircrafts.").arg(Version::getApplicationName()));
        break;
    case ReplayMode::ManualControlUserAircraftIndex:
        ui->replayModeComboBox->setToolTip(tr("Take control of the recorded user aircraft of the formation. The user aircraft can be changed during replay."));
        break;
    case ReplayMode::FlyWithFormationIndex:
        ui->replayModeComboBox->setToolTip(tr("Fly with the currently loaded aircraft along with the formation. Pause the replay to reposition your aircraft again in relation to the recorded user aircraft of the formation."));
        break;
    default:
        break;
    }
}

void FormationWidget::handleUserAircraftChanged(Aircraft &aircraft) noexcept
{
    Q_UNUSED(aircraft)
    updateRelativePosition();
    updateUi();
}

void FormationWidget::handleAircraftInfoChanged() noexcept
{
    updateRelativePosition();
    updateUi();
}

void FormationWidget::handleCellSelected(int row, int column) noexcept
{
    Q_UNUSED(column)
    if (column == d->tailNumberColumnIndex || column == d->timeOffsetColumnIndex) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, column);
        ui->aircraftTableWidget->editItem(item);
    } else {
        updateUserAircraftIndex();
    }
}

void FormationWidget::handleCellChanged(int row, int column) noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    Aircraft &aircraft = flight[d->selectedAircraftIndex];
    if (column == d->tailNumberColumnIndex) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, column);
        const QString tailNumber = item->data(Qt::EditRole).toString();
        d->aircraftService->changeTailNumber(aircraft, tailNumber);
    } else if (column == d->timeOffsetColumnIndex) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, column);
        bool ok;
        const double timeOffsetSec = item->data(Qt::EditRole).toDouble(&ok);
        if (ok) {
            const qint64 timeOffset = static_cast<qint64>(qRound(timeOffsetSec * 1000.0));
            d->aircraftService->changeTimeOffset(aircraft, timeOffset);
        }
    }
}

void FormationWidget::handleSelectionChanged() noexcept
{
    QItemSelectionModel *select = ui->aircraftTableWidget->selectionModel();
    QModelIndexList modelIndices = select->selectedRows(SequenceNumberColumn);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        d->selectedRow = modelIndex.row();
        // Index starts at 0
        d->selectedAircraftIndex = ui->aircraftTableWidget->model()->data(modelIndex).toInt() - 1;
    } else {
        d->selectedRow = InvalidSelection;
        d->selectedAircraftIndex = Flight::InvalidId;
    }
    updateEditUi();
    updateTimeOffsetUi();
    updateToolTips();
}

void FormationWidget::updateUserAircraftIndex() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    if (d->selectedRow != flight.getUserAircraftIndex()) {
        getFlightService().updateUserAircraftIndex(flight, d->selectedRow);
        std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnectOptional = SkyConnectManager::getInstance().getCurrentSkyConnect();
        if (skyConnectOptional) {
            SkyConnectIntf &skyConnect = skyConnectOptional->get();
            // Also update the manually flown user aircraft's position
            if (skyConnect.getReplayMode() == SkyConnectIntf::ReplayMode::UserAircraftManualControl) {
                const Aircraft &aircraft = flight.getUserAircraft();
                const Position &position = aircraft.getPosition();
                const PositionData positionData = position.interpolate(skyConnect.getCurrentTimestamp(), TimeVariableData::Access::Seek);
                skyConnect.setUserAircraftPosition(positionData);
            }
        }
    }
}

void FormationWidget::deleteAircraft() noexcept
{
    Settings &settings = Settings::getInstance();
    bool doDelete;
    if (settings.isDeleteAircraftConfirmationEnabled()) {
        QMessageBox messageBox(this);
        QCheckBox *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), &messageBox);

        // Sequence numbers start at 1
        messageBox.setText(tr("The aircraft with sequence number %1 is about to be deleted. Deletion cannot be undone.").arg(d->selectedRow + 1));
        messageBox.setInformativeText(tr("Do you want to delete the aircraft?"));
        QPushButton *deleteButton = messageBox.addButton(tr("Delete"), QMessageBox::AcceptRole);
        QPushButton *keepButton = messageBox.addButton(tr("Keep"), QMessageBox::RejectRole);
        messageBox.setDefaultButton(keepButton);
        messageBox.setCheckBox(dontAskAgainCheckBox);
        messageBox.setIcon(QMessageBox::Icon::Question);

        messageBox.exec();
        doDelete = messageBox.clickedButton() == deleteButton;
        settings.setDeleteAircraftConfirmationEnabled(!dontAskAgainCheckBox->isChecked());
    } else {
        doDelete = true;
    }
    if (doDelete) {
        d->aircraftService->deleteByIndex(d->selectedRow);
    }
}

void FormationWidget::on_horizontalDistanceSlider_valueChanged(int value) noexcept
{
    Q_UNUSED(value)
    updateInitialPositionUi();
    updateRelativePosition();
}

void FormationWidget::on_verticalDistanceSlider_valueChanged(int value) noexcept
{
    Q_UNUSED(value)
    updateInitialPositionUi();
    updateRelativePosition();
}

void FormationWidget::on_replayModeComboBox_currentIndexChanged(int index) noexcept
{
    std::optional<std::reference_wrapper<SkyConnectIntf>> skyConnectOptional = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnectOptional) {
        SkyConnectIntf &skyConnect = skyConnectOptional->get();
        switch(index) {
        case ReplayMode::NormalIndex:
            skyConnect.setReplayMode(SkyConnectIntf::ReplayMode::Normal);
            break;
        case ReplayMode::ManualControlUserAircraftIndex:
        {
            skyConnect.setReplayMode(SkyConnectIntf::ReplayMode::UserAircraftManualControl);

            Flight &flight = Logbook::getInstance().getCurrentFlight();
            const Aircraft &aircraft = flight.getUserAircraft();
            const Position &position = aircraft.getPosition();   
            const PositionData positionData = position.interpolate(skyConnect.getCurrentTimestamp(), TimeVariableData::Access::Seek);
            skyConnect.setUserAircraftPosition(positionData);
            break;
        }
        case ReplayMode::FlyWithFormationIndex:
        {
            skyConnect.setReplayMode(SkyConnectIntf::ReplayMode::FlyWithFormation);

            const PositionData positionData = calculateRelativePositionToUserAircraft(skyConnect.getCurrentTimestamp());
            skyConnect.setUserAircraftPosition(positionData);
            break;
        }
        default:
            skyConnect.setReplayMode(SkyConnectIntf::ReplayMode::Normal);
            break;
        }
    }
    updateToolTips();
}

void FormationWidget::on_fastForwardOffsetPushButton_clicked() noexcept
{
    if (d->selectedAircraftIndex != Flight::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const qint64 newTimeOffset = aircraft.getTimeOffset() + TimeOffsetIncreaseLarge;
        d->aircraftService->changeTimeOffset(aircraft, newTimeOffset);
        updateToolTips();
    }
}

void FormationWidget::on_forwardOffsetPushButton_clicked() noexcept
{
    if (d->selectedAircraftIndex != Flight::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const qint64 newTimeOffset = aircraft.getTimeOffset() + TimeOffsetIncrease;
        d->aircraftService->changeTimeOffset(aircraft, newTimeOffset);
        updateToolTips();
    }
}

void FormationWidget::on_backwardOffsetPushButton_clicked() noexcept
{
    if (d->selectedAircraftIndex != Flight::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const qint64 newTimeOffset = aircraft.getTimeOffset() - TimeOffsetDecreaseLarge;
        d->aircraftService->changeTimeOffset(aircraft, newTimeOffset);
        updateToolTips();
    }
}

void FormationWidget::on_fastBackwardOffsetPushButton_clicked() noexcept
{
    if (d->selectedAircraftIndex != Flight::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const qint64 newTimeOffset = aircraft.getTimeOffset() - TimeOffsetDecrease;
        d->aircraftService->changeTimeOffset(aircraft, newTimeOffset);
        updateToolTips();
    }
}

void FormationWidget::on_timeOffsetLineEdit_editingFinished() noexcept
{
    if (d->selectedAircraftIndex != Flight::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        bool ok;
        const double timeOffsetSec = ui->timeOffsetLineEdit->text().toDouble(&ok);
        if (ok) {
            const qint64 timeOffset = static_cast<qint64>(qRound(timeOffsetSec * 1000.0));
            d->aircraftService->changeTimeOffset(aircraft, timeOffset);
            updateToolTips();
        }
    }
}

void FormationWidget::on_resetAllTimeOffsetPushButton_clicked() noexcept
{
    Settings &settings = Settings::getInstance();
    bool doReset;
    if (settings.isResetTimeOffsetConfirmationEnabled()) {
        QMessageBox messageBox(this);
        QCheckBox *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), &messageBox);

        messageBox.setText(tr("The time offsets of all aircrafts in this formation will be changed."));
        messageBox.setInformativeText(tr("Do you want to reset all time offsets to 0?"));
        QPushButton *resetButton = messageBox.addButton(tr("Reset Time Offsets"), QMessageBox::AcceptRole);
        QPushButton *doNotChangeButon = messageBox.addButton(tr("Do Not Change"), QMessageBox::RejectRole);
        messageBox.setDefaultButton(doNotChangeButon);
        messageBox.setCheckBox(dontAskAgainCheckBox);
        messageBox.setIcon(QMessageBox::Icon::Question);

        messageBox.exec();
        doReset = messageBox.clickedButton() == resetButton;
        settings.setResetTimeOffsetConfirmationEnabled(!dontAskAgainCheckBox->isChecked());
    } else {
        doReset = true;
    }
    if (doReset) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        bool ok = true;
        for (auto &aircraft : flight) {
            ok = d->aircraftService->changeTimeOffset(*aircraft.get(), 0);
            if (!ok) {
                break;
            }
        }
    }
}
