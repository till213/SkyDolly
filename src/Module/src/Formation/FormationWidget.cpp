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

#include <QWidget>
#include <QAction>
#include <QTableWidget>
#include <QIcon>
#include <QButtonGroup>
#include <QMessageBox>
#include <QCheckBox>

#include "../../../Kernel/src/Version.h"
#include "../../../Kernel/src/Unit.h"
#include "../../../Kernel/src/SkyMath.h"
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
#include "../AbstractModuleWidget.h"
#include "FormationWidget.h"
#include "ui_FormationWidget.h"

namespace
{
    constexpr int MinimumTableWidth = 600;
    constexpr int InvalidSelection = -1;
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
}

class FormationWidgetPrivate
{
public:
    FormationWidgetPrivate(QObject *parent) noexcept
        : positionButtonGroup(new QButtonGroup(parent)),
          moduleAction(nullptr),
          aircraftService(std::make_unique<AircraftService>()),
          selectedRow(InvalidSelection),
          selectedAircraftIndex(Flight::InvalidId)
    {}

    QButtonGroup *positionButtonGroup;
    QMetaObject::Connection aircraftInfoChangedConnection;
    std::unique_ptr<QAction> moduleAction;
    std::unique_ptr<AircraftService> aircraftService;
    int selectedRow;
    qint64 selectedAircraftIndex;
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

    Settings &settings = Settings::getInstance();
    int previewInfoCount = settings.getPreviewInfoDialogCount();
    if (previewInfoCount > 0) {
        --previewInfoCount;
        QMessageBox::information(this, "Preview",
            QString("%1 %2 supports multiple aircrafts per flight (\"formation\"). While in the Fomation module the record button switches its symbol: the plus (+) sign indicates that "
                    "recorded aircrafts are being added to the current flight. Additional aircrafts are initially positioned relative to the last user aircraft.\n\n"
                    "Each newly recorded aircraft is automatically stored into a database (the logbook). As new features are being added and developed the database format will change.\n\n"
                    "During the preview phase older databases will automatically be migrated to the current data format. As a matter of fact at the time you see this message any existing "
                    "logbook from the previous 0.6 version has already been converted to the current format.\n\n"
                    "However take note that the first release version 1.0.0 will consolidate all migration steps into the final database format, making logbooks generated with preview "
                    "versions (such as this one) unreadable!\n\n"
                    "From that point onwards databases (logbooks) will of course again be migrated to the format of the next release version.\n\n"
                    "This dialog will be shown %3 more times.").arg(Version::getApplicationName(), Version::getApplicationVersion()).arg(previewInfoCount),
            QMessageBox::StandardButton::Ok);
        settings.setPreviewInfoDialogCount(previewInfoCount);
    }

    // Deselect when showing module
    d->selectedRow = InvalidSelection;

    Flight &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::userAircraftChanged,
            this, &FormationWidget::handleUserAircraftChanged);
    connect(&flight, &Flight::aircraftDeleted,
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
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    disconnect(&skyConnectManager, &SkyConnectManager::stateChanged,
               this, &FormationWidget::updateUi);
    QObject::disconnect(d->aircraftInfoChangedConnection);
}

// PROTECTED SLOTS

void FormationWidget::handleRecordingStopped() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    int count = flight.count();
    if (count > 1) {
        // Sequence starts at 1
        d->aircraftService->store(flight.getId(), count, flight[count - 1]);
    } else {
        AbstractModuleWidget::handleRecordingStopped();
    }
    updateInitialPosition();
}

// PRIVATE

void FormationWidget::initUi() noexcept
{
    d->moduleAction = std::make_unique<QAction>(getName());
    d->moduleAction->setCheckable(true);

    ui->aircraftTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    const QStringList headers {tr("Sequence"), tr("Type"), tr("Category"), tr("Wing Span"), tr("Initial Airspeed"), tr("Initial Altitude Above Ground"), tr("Tail Number")};
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

    const auto skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    ui->manualUserAircraftCheckBox->setChecked(skyConnect && skyConnect->get().isUserAircraftManualControl());
}

void FormationWidget::frenchConnection() noexcept
{
    connect(ui->aircraftTableWidget, &QTableWidget::itemSelectionChanged,
            this, &FormationWidget::handleSelectionChanged);
    connect(ui->aircraftTableWidget, &QTableWidget::cellDoubleClicked,
            this, &FormationWidget::handleCellSelected);
    connect(ui->userAircraftPushButton, &QPushButton::clicked,
            this, &FormationWidget::updateUserAircraftIndex);
    connect(ui->deletePushButton, &QPushButton::clicked,
            this, &FormationWidget::deleteAircraft);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(d->positionButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &FormationWidget::updateInitialPosition);
#else
    connect(d->positionButtonGroup, &QButtonGroup::idClicked,
            this, &FormationWidget::updateInitialPosition);
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

const QString FormationWidget::getName()
{
    return QString(QT_TRANSLATE_NOOP("LogbookWidget", "Formation"));
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
    const auto skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
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

        // Type
        newItem = new QTableWidgetItem(aircraftInfo.aircraftType.type);
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        ++columnIndex;

        // Category
        newItem = new QTableWidgetItem(aircraftInfo.aircraftType.category);
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
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
        newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ++columnIndex;

        // Tail number
        newItem = new QTableWidgetItem(aircraftInfo.tailNumber);
        ui->aircraftTableWidget->setItem(rowIndex, columnIndex, newItem);
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
}

void FormationWidget::updateEditUi() noexcept
{
    const auto skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    const bool inRecordingMode = skyConnect && skyConnect->get().inRecordingMode();
    const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
    bool userAircraftIndex = d->selectedAircraftIndex == flight.getUserAircraftIndex();
    ui->userAircraftPushButton->setEnabled(d->selectedAircraftIndex != Flight::InvalidId && !userAircraftIndex);
    const bool formation = flight.count() > 1;
    ui->deletePushButton->setEnabled(formation && !inRecordingMode && d->selectedAircraftIndex != Flight::InvalidId);
}

void FormationWidget::updateInitialPosition() noexcept
{
    InitialPosition initialPosition;
    QList<QAbstractButton *> buttons = d->positionButtonGroup->buttons();
    for (QAbstractButton *button : buttons) {
        if (button->isChecked()) {
            button->setToolTip(tr("Selected aircraft position for next recording"));
        } else {
            button->setToolTip(tr("Select aircraft position"));
        }
    }

    const Flight &flight = Logbook::getInstance().getCurrentFlightConst();
    const Aircraft &userAircraft = flight.getUserAircraftConst();
    const PositionData &positionData = userAircraft.getPositionConst().getFirst();
    if (!positionData.isNull()) {
        const AircraftInfo &aircraftInfo = userAircraft.getAircraftInfoConst();
        const AircraftType &aircraftType = aircraftInfo.aircraftType;

        initialPosition.fromPositionData(positionData);

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
        initialPosition.fromAircraftInfo(aircraftInfo);
        auto skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
        if (skyConnect) {
            skyConnect->get().setInitialRecordingPosition(initialPosition);
        }
    }
}

void FormationWidget::handleUserAircraftChanged(Aircraft &aircraft) noexcept
{
    QObject::disconnect(d->aircraftInfoChangedConnection);
    d->aircraftInfoChangedConnection = connect(&aircraft, &Aircraft::infoChanged,
                                               this, &FormationWidget::handleAircraftInfoChanged);
    updateInitialPosition();
    updateUi();
}

void FormationWidget::handleAircraftInfoChanged() noexcept
{
    updateInitialPosition();
    updateUi();
}

void FormationWidget::handleCellSelected(int row, int column) noexcept
{
    Q_UNUSED(column)
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    if (row != flight.getUserAircraftIndex()) {
        getFlightService().updateUserAircraftIndex(flight, row);
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
}

void FormationWidget::updateUserAircraftIndex() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    getFlightService().updateUserAircraftIndex(flight, d->selectedRow);
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
    updateInitialPosition();
}

void FormationWidget::on_verticalDistanceSlider_valueChanged(int value) noexcept
{
    Q_UNUSED(value)
    updateInitialPositionUi();
    updateInitialPosition();
}

void FormationWidget::on_manualUserAircraftCheckBox_toggled(bool enable) noexcept
{
    auto skyConnect = SkyConnectManager::getInstance().getCurrentSkyConnect();
    if (skyConnect) {
        skyConnect->get().setUserAircraftManualControl(enable);
    }
}
