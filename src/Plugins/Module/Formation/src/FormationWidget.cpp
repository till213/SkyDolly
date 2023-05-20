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
#include <algorithm>
#include <memory>
#include <cstdint>
#include <cmath>

#include <QByteArray>
#include <QDoubleValidator>
#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QButtonGroup>
#include <QMessageBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QComboBox>
#ifdef DEBUG
#include <QDebug>
#endif

#include <Kernel/Const.h>
#include <Kernel/Convert.h>
#include <Kernel/Enum.h>
#include <Kernel/Unit.h>
#include <Kernel/SkyMath.h>
#include <Kernel/Version.h>
#include <Kernel/Settings.h>
#include <Model/Logbook.h>
#include <Model/Flight.h>
#include <Model/AircraftType.h>
#include <Model/AircraftInfo.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Persistence/PersistenceManager.h>
#include <Persistence/Service/FlightService.h>
#include <Persistence/Service/AircraftService.h>
#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/Connect/SkyConnectIntf.h>
#include <PluginManager/Connect/Connect.h>
#include <Widget/Platform.h>
#include <Widget/UnitWidgetItem.h>
#include "Formation.h"
#include "FormationSettings.h"
#include "FormationWidget.h"
#include "ui_FormationWidget.h"

namespace
{
    constexpr int MinimumTableWidth {120};
    constexpr int InvalidRow {-1};
    constexpr int InvalidColumn {-1};

    // Milliseconds
    constexpr std::int64_t SmallTimeOffset = 100;
    constexpr std::int64_t LargeTimeOffset = 1000;

    // Seconds
    constexpr double DefaultTimeOffsetSec = 0.0;
    constexpr double TimeOffsetMaxSec = 24.0 * 60.0 * 60.0;
    constexpr double TimeOffsetMinSec = -TimeOffsetMaxSec;
}

struct FormationWidgetPrivate
{
    FormationWidgetPrivate(FormationSettings &moduleSettings, QWidget &parent) noexcept
        : moduleSettings(moduleSettings),
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

    FormationSettings &moduleSettings;
    std::unique_ptr<FlightService> flightService {std::make_unique<FlightService>()};
    std::unique_ptr<AircraftService> aircraftService {std::make_unique<AircraftService>()};
    QButtonGroup *positionButtonGroup;
    int selectedAircraftIndex {Const::InvalidIndex};
    Unit unit;

    static inline int sequenceNumberColumn {::InvalidColumn};
    static inline int aircraftTypeColumn {::InvalidColumn};
    static inline int engineTypeColumn {::InvalidColumn};
    static inline int wingSpanColumn {::InvalidColumn};
    static inline int initialAirspeedColumn {::InvalidColumn};
    static inline int initialAltitudeColumn {::InvalidColumn};
    static inline int durationColumn {::InvalidColumn};
    static inline int tailNumberColumn {::InvalidColumn};
    static inline int timeOffsetColumn {::InvalidColumn};

    // Only initialise once the PluginManager.qrc resources are available
    static inline QIcon normalAircraftIcon;
    static inline QIcon recordingAircraftIcon;
    static inline QIcon referenceAircraftIcon;

    // QPixmaps can only be created after QApplication (cannot be static)
    QPixmap userAircraftPixmap;
    QPixmap referenceAircraftPixmap;
};

// PUBLIC

FormationWidget::FormationWidget(FormationSettings &settings, QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::FormationWidget>()),
      d(std::make_unique<FormationWidgetPrivate>(settings, *this))
{
    ui->setupUi(this);
    initUi();
    // The aircraft table is updated once the plugin settings are restored (initiated
    // by FormationPlugin)
    updateInteractiveUi();
    frenchConnection();
}

FormationWidget::~FormationWidget()
{
    const QByteArray tableState = ui->aircraftTableWidget->horizontalHeader()->saveState();
}

Formation::HorizontalDistance FormationWidget::getHorizontalDistance() const noexcept
{
    return static_cast<Formation::HorizontalDistance>(ui->horizontalDistanceSlider->value());
}

Formation::VerticalDistance FormationWidget::getVerticalDistance() const noexcept
{
    return static_cast<Formation::VerticalDistance>(ui->verticalDistanceSlider->value());
}

Formation::Bearing FormationWidget::getRelativePosition() const noexcept
{
    return static_cast<Formation::Bearing>(d->positionButtonGroup->checkedId());
}

// PRIVATE

void FormationWidget::initUi() noexcept
{
    ui->aircraftTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    const QStringList headers {
        tr("Sequence"), tr("Aircraft"), tr("Engine Type"),
        tr("Wing Span"), tr("Initial Airspeed"), tr("Initial Altitude"),
        tr("Duration"), tr("Tail Number"), tr("Time Offset")
    };
    FormationWidgetPrivate::sequenceNumberColumn = static_cast<int>(headers.indexOf(tr("Sequence")));
    FormationWidgetPrivate::aircraftTypeColumn = static_cast<int>(headers.indexOf(tr("Aircraft")));
    FormationWidgetPrivate::engineTypeColumn = static_cast<int>(headers.indexOf(tr("Engine Type")));
    FormationWidgetPrivate::wingSpanColumn = static_cast<int>(headers.indexOf(tr("Wing Span")));
    FormationWidgetPrivate::initialAirspeedColumn = static_cast<int>(headers.indexOf(tr("Initial Airspeed")));
    FormationWidgetPrivate::initialAltitudeColumn = static_cast<int>(headers.indexOf(tr("Initial Altitude")));
    FormationWidgetPrivate::durationColumn = static_cast<int>(headers.indexOf(tr("Duration")));
    FormationWidgetPrivate::tailNumberColumn = static_cast<int>(headers.indexOf(tr("Tail Number")));
    FormationWidgetPrivate::timeOffsetColumn = static_cast<int>(headers.indexOf(tr("Time Offset")));

    ui->aircraftTableWidget->setColumnCount(static_cast<int>(headers.count()));
    ui->aircraftTableWidget->setHorizontalHeaderLabels(headers);
    ui->aircraftTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->aircraftTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->aircraftTableWidget->verticalHeader()->hide();
    ui->aircraftTableWidget->setMinimumWidth(::MinimumTableWidth);
    ui->aircraftTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->aircraftTableWidget->sortByColumn(FormationWidgetPrivate::sequenceNumberColumn, Qt::SortOrder::AscendingOrder);
    ui->aircraftTableWidget->horizontalHeader()->setSectionsMovable(true);
    ui->aircraftTableWidget->setAlternatingRowColors(true);

    d->positionButtonGroup->addButton(ui->nPositionRadioButton, Formation::Bearing::North);
    d->positionButtonGroup->addButton(ui->nnePositionRadioButton, Formation::Bearing::NorthNorthEast);
    d->positionButtonGroup->addButton(ui->nePositionRadioButton, Formation::Bearing::NorthEast);
    d->positionButtonGroup->addButton(ui->enePositionRadioButton, Formation::Bearing::EastNorthEast);
    d->positionButtonGroup->addButton(ui->ePositionRadioButton, Formation::Bearing::East);
    d->positionButtonGroup->addButton(ui->esePositionRadioButton, Formation::Bearing::EastSouthEast);
    d->positionButtonGroup->addButton(ui->sePositionRadioButton, Formation::Bearing::SouthEast);
    d->positionButtonGroup->addButton(ui->ssePositionRadioButton, Formation::Bearing::SouthSouthEast);
    d->positionButtonGroup->addButton(ui->sPositionRadioButton, Formation::Bearing::South);
    d->positionButtonGroup->addButton(ui->sswPositionRadioButton, Formation::Bearing::SouthSouthWest);
    d->positionButtonGroup->addButton(ui->swPositionRadioButton, Formation::Bearing::SouthWest);
    d->positionButtonGroup->addButton(ui->wswPositionRadioButton, Formation::Bearing::WestSouthWest);
    d->positionButtonGroup->addButton(ui->wPositionRadioButton, Formation::Bearing::West);
    d->positionButtonGroup->addButton(ui->wnwPositionRadioButton, Formation::Bearing::WestNorthWest);
    d->positionButtonGroup->addButton(ui->nwPositionRadioButton, Formation::Bearing::NorthWest);
    d->positionButtonGroup->addButton(ui->nnwPositionRadioButton, Formation::Bearing::NorthNorthWest);

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

    ui->replayModeComboBox->addItem(tr("Formation (Normal)"), Enum::underly(SkyConnectIntf::ReplayMode::Normal));
    ui->replayModeComboBox->addItem(tr("Take control of recorded user aircraft"), Enum::underly(SkyConnectIntf::ReplayMode::UserAircraftManualControl));
    ui->replayModeComboBox->addItem(tr("Fly with formation"), Enum::underly(SkyConnectIntf::ReplayMode::FlyWithFormation));

    // Default "Delete" key deletes aircraft
    ui->deletePushButton->setShortcut(QKeySequence::Delete);

    initTimeOffsetUi(); 
}

void FormationWidget::initTimeOffsetUi() noexcept
{
    ui->timeOffsetGroupBox->setStyleSheet(Platform::getFlatButtonCss());

    // Validation
    ui->timeOffsetSpinBox->setRange(::TimeOffsetMinSec, ::TimeOffsetMaxSec);
    ui->timeOffsetSpinBox->setSuffix(tr(" s"));
}

void FormationWidget::frenchConnection() noexcept
{
    // Logbook
    connect(&PersistenceManager::getInstance(), &PersistenceManager::connectionChanged,
            this, &FormationWidget::updateUi);

    // Flight
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    connect(&flight, &Flight::flightRestored,
            this, &FormationWidget::updateUi);
    connect(&flight, &Flight::userAircraftChanged,
            this, &FormationWidget::onUserAircraftChanged);
    connect(&flight, &Flight::aircraftAdded,
            this, &FormationWidget::onAircraftAdded);
    connect(&flight, &Flight::aircraftRemoved,
            this, &FormationWidget::updateUi);
    connect(&flight, &Flight::aircraftInfoChanged,
            this, &FormationWidget::onAircraftInfoChanged);
    connect(&flight, &Flight::cleared,
            this, &FormationWidget::updateUi);

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

    // Relative position, replay mode
    connect(ui->horizontalDistanceSlider, &QSlider::valueChanged,
            this, &FormationWidget::onHorizontalDistanceChanged);
    connect(ui->verticalDistanceSlider, &QSlider::valueChanged,
            this, &FormationWidget::onVerticalDistanceChanged);
    connect(d->positionButtonGroup, &QButtonGroup::idClicked,
            this, &FormationWidget::onRelativePositionChanged);
    connect(ui->relativePositionCheckBox, &QCheckBox::stateChanged,
            this, &FormationWidget::onInitialPositionPlacementChanged);
    connect(ui->replayModeComboBox, &QComboBox::activated,
            this, &FormationWidget::onReplayModeSelected);
    connect(ui->restoreDefaultsPushButton, &QPushButton::clicked,
            this, &FormationWidget::restoreDefaultSettings);

    // Time offset
    connect(ui->fastBackwardOffsetPushButton, &QPushButton::clicked,
            this, [&] { changeTimeOffset(- ::LargeTimeOffset);});
    connect(ui->backwardOffsetPushButton, &QPushButton::clicked,
            this, [&] { changeTimeOffset(- ::SmallTimeOffset);});
    connect(ui->forwardOffsetPushButton, &QPushButton::clicked,
            this, [&] { changeTimeOffset(+ ::SmallTimeOffset);});
    connect(ui->fastForwardOffsetPushButton, &QPushButton::clicked,
            this, [&] { changeTimeOffset(+ ::LargeTimeOffset);});
    connect(ui->timeOffsetSpinBox, &QDoubleSpinBox::valueChanged,
            this, &FormationWidget::onTimeOffsetValueChanged);
    connect(ui->resetAllTimeOffsetPushButton, &QPushButton::clicked,
            this, &FormationWidget::resetAllTimeOffsets);

    // Module settings
    connect(ui->aircraftTableWidget->horizontalHeader(), &QHeaderView::sectionMoved,
            this, &FormationWidget::onTableLayoutChanged);
    connect(ui->aircraftTableWidget->horizontalHeader(), &QHeaderView::sectionResized,
            this, &FormationWidget::onTableLayoutChanged);
    connect(&d->moduleSettings, &ModuleBaseSettings::changed,
            this, &FormationWidget::onModuleSettingsChanged);
}

void FormationWidget::updateTable() noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();

    ui->aircraftTableWidget->blockSignals(true);
    ui->aircraftTableWidget->setSortingEnabled(false);
    ui->aircraftTableWidget->clearContents();
    ui->aircraftTableWidget->setRowCount(static_cast<int>(flight.count()));

    int row {0};
    int aircraftIndex {0};
    for (const auto &aircraft : flight) {
        initRow(aircraft, row, aircraftIndex);
        ++row;
        ++aircraftIndex;
    }

    ui->aircraftTableWidget->setSortingEnabled(true);

    QByteArray tableState = d->moduleSettings.getFormationAircraftTableState();
    if (!tableState.isEmpty()) {
        ui->aircraftTableWidget->horizontalHeader()->blockSignals(true);
        ui->aircraftTableWidget->horizontalHeader()->restoreState(tableState);
        ui->aircraftTableWidget->horizontalHeader()->blockSignals(false);
    } else {
        ui->aircraftTableWidget->resizeColumnsToContents();
    }
    d->selectedAircraftIndex = Const::InvalidIndex;
    ui->aircraftTableWidget->blockSignals(false);

    updateAircraftCount();
}

void FormationWidget::updateInteractiveUi() noexcept
{
    updateRelativePositionUi();
    updateEditUi();
    updateTimeOffsetUi();
    updateReplayUi();
    updateToolTips();
}

void FormationWidget::updateAircraftIcons() noexcept
{
    const SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const SkyConnectIntf::ReplayMode replayMode = skyConnectManager.getReplayMode();
    if (replayMode == SkyConnectIntf::ReplayMode::FlyWithFormation) {
        ui->referenceAircraftLabel->setPixmap(d->referenceAircraftPixmap);
    } else {
        ui->referenceAircraftLabel->setPixmap(d->userAircraftPixmap);
    }

    ui->aircraftTableWidget->blockSignals(true);

    // Reset all icons
    for (int row = 0; row < ui->aircraftTableWidget->rowCount(); ++row) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::sequenceNumberColumn);
        item->setIcon(QIcon());
    }
    // Update user aircraft icon
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const int userAircraftIndex = flight.getUserAircraftIndex();
    const int row = getRowByAircraftIndex(userAircraftIndex);
    if (row != ::InvalidRow) {
        updateRow(flight.getUserAircraft(), row, userAircraftIndex);
    }

    ui->aircraftTableWidget->blockSignals(false);
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

    updateToolTips();
}

void FormationWidget::updateEditUi() noexcept
{
    const bool inRecordingState = SkyConnectManager::getInstance().isInRecordingState();
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    bool userAircraftIndex = d->selectedAircraftIndex == flight.getUserAircraftIndex();
    ui->userAircraftPushButton->setEnabled(d->selectedAircraftIndex != Const::InvalidIndex && !inRecordingState && !userAircraftIndex);
    const bool formation = flight.count() > 1;
    ui->deletePushButton->setEnabled(formation && !inRecordingState && d->selectedAircraftIndex != Const::InvalidIndex);
}

void FormationWidget::updateTimeOffsetUi() noexcept
{
    const bool enabled = d->selectedAircraftIndex != Const::InvalidIndex;

    ui->fastBackwardOffsetPushButton->setEnabled(enabled);
    ui->backwardOffsetPushButton->setEnabled(enabled);
    ui->timeOffsetSpinBox->setEnabled(enabled);
    ui->forwardOffsetPushButton->setEnabled(enabled);
    ui->fastForwardOffsetPushButton->setEnabled(enabled);

    ui->timeOffsetSpinBox->blockSignals(true);
    if (enabled) {
        const Flight &flight = Logbook::getInstance().getCurrentFlight();
        const Aircraft &aircraft = flight[d->selectedAircraftIndex];
        const std::int64_t timeOffset = aircraft.getAircraftInfo().timeOffset;
        const double timeOffsetSec = static_cast<double>(timeOffset) / 1000.0;
        ui->timeOffsetSpinBox->setValue(timeOffsetSec);
    } else {
        ui->timeOffsetSpinBox->setValue(::DefaultTimeOffsetSec);
    }
    ui->timeOffsetSpinBox->blockSignals(false);
}

void FormationWidget::updateReplayUi() noexcept
{
    SkyConnectManager &skyConnectManager {SkyConnectManager::getInstance()};
    updateReplayModeUi(skyConnectManager.getReplayMode());
    ui->replayModeComboBox->setEnabled(!skyConnectManager.isInRecordingState());
}

void FormationWidget::updateReplayModeUi(SkyConnectIntf::ReplayMode replayMode) noexcept
{
    ui->replayModeComboBox->blockSignals(true);
    for (int index = 0; index < ui->replayModeComboBox->count(); ++index) {
        if (static_cast<SkyConnectIntf::ReplayMode>(ui->replayModeComboBox->itemData(index).toInt()) == replayMode) {
            ui->replayModeComboBox->setCurrentIndex(index);
            break;
        }
    }
    ui->replayModeComboBox->blockSignals(false);
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
    if (d->selectedAircraftIndex != Const::InvalidId) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const std::int64_t timeOffset = aircraft.getTimeOffset();
        if (timeOffset < 0) {
            ui->timeOffsetSpinBox->setToolTip(tr("The aircraft is %1 behind its recorded schedule.").arg(d->unit.formatElapsedTime(timeOffset)));
        } else if (timeOffset > 0) {
            ui->timeOffsetSpinBox->setToolTip(tr("The aircraft is %1 ahead its recorded schedule.").arg(d->unit.formatElapsedTime(timeOffset)));
        } else {
            ui->timeOffsetSpinBox->setToolTip(tr("Positive values [seconds] put the aircraft ahead, negative values put the aircraft behind its recorded schedule."));
        }
    }

    // Replay mode
    switch (static_cast<SkyConnectIntf::ReplayMode>(ui->replayModeComboBox->currentData().toInt())) {
    case SkyConnectIntf::ReplayMode::Normal:
        ui->replayModeComboBox->setToolTip(tr("%1 controls all recorded aircraft.").arg(Version::getApplicationName()));
        break;
    case SkyConnectIntf::ReplayMode::UserAircraftManualControl:
        ui->replayModeComboBox->setToolTip(tr("Take control of the recorded user aircraft of the formation.\n"
                                              "The user aircraft (marked in blue) can be changed during replay."));
        break;
    case SkyConnectIntf::ReplayMode::FlyWithFormation:
        ui->replayModeComboBox->setToolTip(tr("Fly with the currently loaded aircraft along with the entire formation.\n"
                                              "Reposition your user aircraft at any time, by either changing its relative position\n"
                                              "or choose another reference aircraft (marked in green) in the formation."));
        break;
    }
}

inline const QTableWidgetItem *FormationWidget::createRow(const Aircraft &aircraft, int aircraftIndex) noexcept
{
    const int row = ui->aircraftTableWidget->rowCount();
    ui->aircraftTableWidget->insertRow(row);
    return initRow(aircraft, row, aircraftIndex);
}

inline const QTableWidgetItem *FormationWidget::initRow(const Aircraft &aircraft, int row, int aircraftIndex) noexcept
{
    int column {0};

    // Sequence number
    std::unique_ptr<QTableWidgetItem> newItem = std::make_unique<QTableWidgetItem>();
    const QTableWidgetItem *firstItem = newItem.get();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    newItem->setToolTip(tr("Double-click to change user aircraft."));
    ui->aircraftTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Aircraft type
    newItem = std::make_unique<QTableWidgetItem>();
    ui->aircraftTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Engine type
    newItem = std::make_unique<QTableWidgetItem>();
    ui->aircraftTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Wing span
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->aircraftTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Initial airspeed
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->aircraftTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Initial altitude above ground
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setToolTip(tr("Altitude above ground."));
    newItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->aircraftTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Duration
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setToolTip(tr("Recording duration."));
    ui->aircraftTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Tail number
    newItem = std::make_unique<QTableWidgetItem>();
    newItem->setToolTip(tr("Double-click to edit tail number."));
    newItem->setBackground(Platform::getEditableTableCellBGColor());
    ui->aircraftTableWidget->setItem(row, column, newItem.release());
    ++column;

    // Time offset
    newItem = std::make_unique<UnitWidgetItem>(d->unit, Unit::Name::Second);
    newItem->setToolTip(tr("Double-click to edit time offset [seconds]."));
    newItem->setBackground(Platform::getEditableTableCellBGColor());
    ui->aircraftTableWidget->setItem(row, column, newItem.release());

    updateRow(aircraft, row, aircraftIndex);

    return firstItem;
}

inline void FormationWidget::updateRow(const Aircraft &aircraft, int row, int aircraftIndex) noexcept
{
    const SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();

    // Sequence number
    QTableWidgetItem *item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::sequenceNumberColumn);
    // Sequence numbers start at 1
    item->setData(Qt::DisplayRole, aircraftIndex + 1);
    // Icon
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const int userAircraftIndex = flight.getUserAircraftIndex();
    const bool recording = skyConnectManager.isInRecordingState();
    const SkyConnectIntf::ReplayMode replayMode = skyConnectManager.getReplayMode();
    if (aircraftIndex == userAircraftIndex) {
        if (recording) {
            item->setIcon(FormationWidgetPrivate::recordingAircraftIcon);
        } else if (replayMode == SkyConnectIntf::ReplayMode::FlyWithFormation) {
            item->setIcon(FormationWidgetPrivate::referenceAircraftIcon);
        } else {
            item->setIcon(FormationWidgetPrivate::normalAircraftIcon);
        }
    } else {
        item->setIcon(QIcon());
    }

    // Aircraft type
    item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::aircraftTypeColumn);
    item->setData(Qt::DisplayRole, aircraftInfo.aircraftType.type);

    // Engine type 
    item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::engineTypeColumn);
    item->setData(Qt::DisplayRole, SimType::engineTypeToString(aircraftInfo.aircraftType.engineType));

    // Wing span
    item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::wingSpanColumn);
    item->setData(Qt::DisplayRole, d->unit.formatFeet(aircraftInfo.aircraftType.wingSpan));

    // Initial airspeed 
    item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::initialAirspeedColumn);
    item->setData(Qt::DisplayRole, d->unit.formatKnots(aircraftInfo.initialAirspeed));

    // Initial altitude above ground
    item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::initialAltitudeColumn);
    item->setData(Qt::DisplayRole, d->unit.formatFeet(aircraftInfo.altitudeAboveGround));

    // Duration
    item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::durationColumn);
    item->setData(Qt::DisplayRole, Unit::formatHHMMSS(aircraft.getDurationMSec()));

    // Tail number 
    item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::tailNumberColumn);
    item->setData(Qt::DisplayRole, aircraftInfo.tailNumber);

    // Time offset
    const double timeOffsetSec = static_cast<double>(aircraftInfo.timeOffset) / 1000.0;    
    item = ui->aircraftTableWidget->item(row, FormationWidgetPrivate::timeOffsetColumn);
    item->setData(Qt::EditRole, timeOffsetSec);
}

// PRIVATE SLOTS

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
            const PositionData &positionData = position.interpolate(skyConnectManager.getCurrentTimestamp(), TimeVariableData::Access::DiscreteSeek);
            skyConnectManager.setUserAircraftPosition(positionData);
        }
        break;
    }
    case SkyConnectIntf::ReplayMode::FlyWithFormation:
        if (!skyConnectManager.isInRecordingState() && d->moduleSettings.isRelativePositionPlacementEnabled()) {
            const Formation::HorizontalDistance horizontalDistance {getHorizontalDistance()};
            const Formation::VerticalDistance verticalDistance {getVerticalDistance()};
            const Formation::Bearing relativePosition {getRelativePosition()};
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
    if (d->moduleSettings.isRelativePositionPlacementEnabled()) {
        switch(replayMode) {
        case SkyConnectIntf::ReplayMode::Normal:
            break;
        case SkyConnectIntf::ReplayMode::UserAircraftManualControl:
        {
            Flight &flight = Logbook::getInstance().getCurrentFlight();
            const Aircraft &aircraft = flight.getUserAircraft();
            Position &position = aircraft.getPosition();
            const PositionData &positionData = position.interpolate(skyConnectManager.getCurrentTimestamp(), TimeVariableData::Access::DiscreteSeek);
            skyConnectManager.setUserAircraftPosition(positionData);
            break;
        }
        case SkyConnectIntf::ReplayMode::FlyWithFormation:
            const Formation::HorizontalDistance horizontalDistance {getHorizontalDistance()};
            const Formation::VerticalDistance verticalDistance {getVerticalDistance()};
            const Formation::Bearing relativePosition {getRelativePosition()};
            const PositionData positionData = Formation::calculateRelativePositionToUserAircraft(horizontalDistance,
                                                                                                 verticalDistance,
                                                                                                 relativePosition,
                                                                                                 skyConnectManager.getCurrentTimestamp());
            skyConnectManager.setUserAircraftPosition(positionData);
            break;
        }
    }
}

int FormationWidget::getSelectedRow() const noexcept
{
    int selectedRow {::InvalidRow};
    const QItemSelectionModel *select = ui->aircraftTableWidget->selectionModel();
    const QModelIndexList modelIndices = select->selectedRows(FormationWidgetPrivate::sequenceNumberColumn);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        selectedRow = modelIndex.row();
    }
    return selectedRow;
}

int FormationWidget::getRowBySequenceNumber(int sequence) const noexcept
{
    int row {::InvalidRow};
    int currentRow {0};
    while (row == ::InvalidRow && currentRow < ui->aircraftTableWidget->rowCount()) {
        const QTableWidgetItem *item = ui->aircraftTableWidget->item(currentRow, FormationWidgetPrivate::sequenceNumberColumn);
        if (sequence == item->data(Qt::EditRole).toInt()) {
            row = currentRow;
        } else {
            ++currentRow;
        }
    }
    return row;
}

int FormationWidget::getRowByAircraftIndex(int index) const noexcept
{
    // Sequence number starts at 1
    return getRowBySequenceNumber(index + 1);
}

void FormationWidget::updateAircraftCount() const noexcept
{
    const int aircraftCount = ui->aircraftTableWidget->rowCount();
    ui->aircraftCountLabel->setText(tr("%1 aircraft", "Number of aircraft in the formation flight", aircraftCount).arg(aircraftCount));
}

void FormationWidget::updateRelativePosition()
{
    updateAndSendUserAircraftPosition();
}

// PRIVATE SLOTS

void FormationWidget::updateUi() noexcept
{
    updateTable();
    updateInteractiveUi();
}

void FormationWidget::onUserAircraftChanged() noexcept
{
    updateAircraftIcons();
    updateEditUi();
    updateAndSendUserAircraftPosition();
}

void FormationWidget::onAircraftAdded(const Aircraft &aircraft) noexcept
{
    const Flight &flight = Logbook::getInstance().getCurrentFlight();
    const int aircraftIndex = flight.getAircraftIndex(aircraft);

    ui->aircraftTableWidget->blockSignals(true);
    ui->aircraftTableWidget->setSortingEnabled(false);
    const QTableWidgetItem *firstItem = createRow(aircraft, aircraftIndex);
    ui->aircraftTableWidget->blockSignals(false);
    ui->aircraftTableWidget->setSortingEnabled(true);
    ui->aircraftTableWidget->scrollToItem(firstItem);
    updateTimeOffsetUi();
    updateAircraftCount();
}

void FormationWidget::onAircraftInfoChanged(const Aircraft &aircraft) noexcept
{
    int row {::InvalidRow};
    const int aircraftIndex = Logbook::getInstance().getCurrentFlight().getAircraftIndex(aircraft);
    if (d->selectedAircraftIndex == aircraftIndex) {
        row = getSelectedRow();
    } else {
        // Sequence number starts at 1
        row = getRowByAircraftIndex(aircraftIndex);
    }
    // Update aircraft table
    if (row != ::InvalidRow) {
        ui->aircraftTableWidget->blockSignals(true);
        ui->aircraftTableWidget->setSortingEnabled(false);
        updateRow(aircraft, row, aircraftIndex);
        ui->aircraftTableWidget->blockSignals(false);
        ui->aircraftTableWidget->setSortingEnabled(true);
    }
    updateTimeOffsetUi();
}

void FormationWidget::onCellSelected(int row, [[maybe_unused]] int column) noexcept
{
    if (column == FormationWidgetPrivate::tailNumberColumn || column == FormationWidgetPrivate::timeOffsetColumn) {
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
    if (column == FormationWidgetPrivate::tailNumberColumn) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, column);
        const QString tailNumber = item->data(Qt::EditRole).toString();
        d->aircraftService->changeTailNumber(aircraft, tailNumber);
    } else if (column == FormationWidgetPrivate::timeOffsetColumn) {
        QTableWidgetItem *item = ui->aircraftTableWidget->item(row, column);
        bool ok {false};
        const double timeOffsetSec = item->data(Qt::EditRole).toDouble(&ok);
        if (ok) {
            const std::int64_t timeOffset = static_cast<std::int64_t>(std::round(timeOffsetSec * 1000.0));
            d->aircraftService->changeTimeOffset(aircraft, timeOffset);
        }
    }
}

void FormationWidget::onSelectionChanged() noexcept
{
    QItemSelectionModel *select = ui->aircraftTableWidget->selectionModel();
    QModelIndexList modelIndices = select->selectedRows(d->sequenceNumberColumn);
    if (modelIndices.count() > 0) {
        QModelIndex modelIndex = modelIndices.at(0);
        // Index starts at 0
        d->selectedAircraftIndex = ui->aircraftTableWidget->model()->data(modelIndex).toInt() - 1;
    } else {
        d->selectedAircraftIndex = Const::InvalidIndex;
    }
    updateEditUi();
    updateTimeOffsetUi();
    updateToolTips();
}

void FormationWidget::onInitialPositionPlacementChanged(bool enable) noexcept
{
    d->moduleSettings.setRelativePositionPlacementEnabled(enable);
}

void FormationWidget::updateUserAircraftIndex() noexcept
{
    if (!SkyConnectManager::getInstance().isInRecordingState()) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        if (d->selectedAircraftIndex != flight.getUserAircraftIndex()) {
            d->flightService->updateUserAircraftIndex(flight, d->selectedAircraftIndex);
        }
    }
}

void FormationWidget::deleteAircraft() noexcept
{
    if (d->selectedAircraftIndex != ::Const::InvalidIndex) {
        Settings &settings = Settings::getInstance();
        bool doDelete {true};
        if (settings.isDeleteAircraftConfirmationEnabled()) {
            std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
            auto *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), messageBox.get());

            // Sequence numbers start at 1
            messageBox->setWindowTitle(tr("Delete Aircraft"));
            messageBox->setText(tr("The aircraft with sequence number %1 is about to be deleted. Do you want to delete the aircraft?").arg(d->selectedAircraftIndex + 1));
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
            const int lastSelectedRow = getSelectedRow();
            d->aircraftService->deleteByIndex(d->selectedAircraftIndex);
            const int selectedRow = std::min(lastSelectedRow, ui->aircraftTableWidget->rowCount() - 1);
            ui->aircraftTableWidget->selectRow(selectedRow);
            ui->aircraftTableWidget->setFocus(Qt::NoFocusReason);
        }
    }
}

void FormationWidget::onRelativePositionChanged() noexcept
{
    Formation::Bearing bearing = bearingFromPositionGroup();
    d->moduleSettings.setBearing(bearing);
    updateRelativePositionUi();
    updateRelativePosition();
}

void FormationWidget::onHorizontalDistanceChanged() noexcept
{
    d->moduleSettings.setHorizontalDistance(static_cast<Formation::HorizontalDistance>(ui->horizontalDistanceSlider->value()));
    updateRelativePositionUi();
    updateRelativePosition();
}

void FormationWidget::onVerticalDistanceChanged() noexcept
{
    d->moduleSettings.setVerticalDistance(static_cast<Formation::VerticalDistance>(ui->verticalDistanceSlider->value()));
    updateRelativePositionUi();
    onRelativePositionChanged();
}

void FormationWidget::onReplayModeSelected() noexcept
{
    SkyConnectManager &skyConnectManager = SkyConnectManager::getInstance();
    SkyConnectIntf::ReplayMode replayMode {static_cast<SkyConnectIntf::ReplayMode>(ui->replayModeComboBox->currentData().toInt())};
    skyConnectManager.setReplayMode(replayMode);
    d->moduleSettings.setReplayMode(replayMode);
    updateUi();
}

void FormationWidget::onReplayModeChanged(SkyConnectIntf::ReplayMode replayMode)
{
    updateReplayModeUi(replayMode);
    updateUserAircraftPosition(replayMode);
}

void FormationWidget::changeTimeOffset(const std::int64_t timeOffset) noexcept
{
    if (d->selectedAircraftIndex != Const::InvalidIndex) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const std::int64_t newTimeOffset = aircraft.getTimeOffset() + timeOffset;
        d->aircraftService->changeTimeOffset(aircraft, newTimeOffset);
        updateToolTips();
    }
}

void FormationWidget::onTimeOffsetValueChanged() noexcept
{
    if (d->selectedAircraftIndex != Const::InvalidIndex) {
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        Aircraft &aircraft = flight[d->selectedAircraftIndex];

        const double timeOffsetSec = ui->timeOffsetSpinBox->value();
        const std::int64_t timeOffset = static_cast<std::int64_t>(std::round(timeOffsetSec * 1000.0));
        d->aircraftService->changeTimeOffset(aircraft, timeOffset);
        updateToolTips();
    }
}

void FormationWidget::resetAllTimeOffsets() noexcept
{
    Settings &settings = Settings::getInstance();
    bool doReset {true};
    if (settings.isResetTimeOffsetConfirmationEnabled()) {
        std::unique_ptr<QMessageBox> messageBox = std::make_unique<QMessageBox>(this);
        auto *dontAskAgainCheckBox = new QCheckBox(tr("Do not ask again."), messageBox.get());

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
        bool ok {true};
        for (auto &aircraft : flight) {
            ok = d->aircraftService->changeTimeOffset(aircraft, 0);
            if (!ok) {
                break;
            }
        }
    }
}

void FormationWidget::onTableLayoutChanged() noexcept
{
    QByteArray tableState = ui->aircraftTableWidget->horizontalHeader()->saveState();
    d->moduleSettings.setFormationAircraftTableState(std::move(tableState));
}

void FormationWidget::onModuleSettingsChanged() noexcept
{
    QRadioButton &button = getPositionButtonFromSettings();
    button.blockSignals(true);
    button.setChecked(true);
    button.blockSignals(false);

    ui->horizontalDistanceSlider->blockSignals(true);
    ui->horizontalDistanceSlider->setValue(d->moduleSettings.getHorizontalDistance());
    ui->horizontalDistanceSlider->blockSignals(false);

    ui->verticalDistanceSlider->blockSignals(true);
    ui->verticalDistanceSlider->setValue(d->moduleSettings.getVerticalDistance());
    ui->verticalDistanceSlider->blockSignals(false);

    ui->relativePositionCheckBox->blockSignals(true);
    ui->relativePositionCheckBox->setChecked(d->moduleSettings.isRelativePositionPlacementEnabled());
    ui->relativePositionCheckBox->blockSignals(false);

    updateTable();
    updateReplayModeUi(d->moduleSettings.getReplayMode());
}

QRadioButton &FormationWidget::getPositionButtonFromSettings() const noexcept
{
    QRadioButton *button {nullptr};
    const Formation::Bearing bearing = d->moduleSettings.getBearing();
    switch (bearing) {
    case Formation::Bearing::North:
        button = ui->nPositionRadioButton;
        break;
    case Formation::Bearing::NorthNorthEast:
        button = ui->nnePositionRadioButton;
        break;
    case Formation::Bearing::NorthEast:
        button = ui->nePositionRadioButton;
        break;
    case Formation::Bearing::EastNorthEast:
        button = ui->enePositionRadioButton;
        break;
    case Formation::Bearing::East:
        button = ui->ePositionRadioButton;
        break;
    case Formation::Bearing::EastSouthEast:
        button = ui->esePositionRadioButton;
        break;
    case Formation::Bearing::SouthEast:
        button = ui->sePositionRadioButton;
        break;
    case Formation::Bearing::SouthSouthEast:
        button = ui->ssePositionRadioButton;
        break;
    case Formation::Bearing::South:
        button = ui->sPositionRadioButton;
        break;
    case Formation::Bearing::SouthSouthWest:
        button = ui->sswPositionRadioButton;
        break;
    case Formation::Bearing::SouthWest:
        button = ui->swPositionRadioButton;
        break;
    case Formation::Bearing::WestSouthWest:
        button = ui->wswPositionRadioButton;
        break;
    case Formation::Bearing::West:
        button = ui->wPositionRadioButton;
        break;
    case Formation::Bearing::WestNorthWest:
        button = ui->wnwPositionRadioButton;
        break;
    case Formation::Bearing::NorthWest:
        button = ui->nwPositionRadioButton;
        break;
    case Formation::Bearing::NorthNorthWest:
        button = ui->nnwPositionRadioButton;
        break;
    }
    return *button;
}

Formation::Bearing FormationWidget::bearingFromPositionGroup() const noexcept
{
    QAbstractButton *button = d->positionButtonGroup->checkedButton();
    Formation::Bearing bearing;
    if (button == ui->nPositionRadioButton) {
        bearing = Formation::Bearing::North;
    } else if (button == ui->nnePositionRadioButton) {
        bearing = Formation::Bearing::NorthNorthEast;
    } else if (button == ui->nePositionRadioButton) {
        bearing = Formation::Bearing::NorthEast;
    } else if (button == ui->enePositionRadioButton) {
        bearing = Formation::Bearing::EastNorthEast;
    } else if (button == ui->ePositionRadioButton) {
        bearing = Formation::Bearing::East;
    } else if (button == ui->esePositionRadioButton) {
        bearing = Formation::Bearing::EastSouthEast;
    } else if (button == ui->sePositionRadioButton) {
        bearing = Formation::Bearing::SouthEast;
    } else if (button == ui->ssePositionRadioButton) {
        bearing = Formation::Bearing::SouthSouthEast;
    } else if (button == ui->sPositionRadioButton) {
        bearing = Formation::Bearing::South;
    } else if (button == ui->sswPositionRadioButton) {
        bearing = Formation::Bearing::SouthSouthWest;
    } else if (button == ui->swPositionRadioButton) {
        bearing = Formation::Bearing::SouthWest;
    } else if (button == ui->wswPositionRadioButton) {
        bearing = Formation::Bearing::WestSouthWest;
    } else if (button == ui->wPositionRadioButton) {
        bearing = Formation::Bearing::West;
    } else if (button == ui->wnwPositionRadioButton) {
        bearing = Formation::Bearing::WestNorthWest;
    } else if (button == ui->nwPositionRadioButton) {
        bearing = Formation::Bearing::NorthWest;
    } else {
        bearing = Formation::Bearing::NorthNorthWest;
    }

    return bearing;
}

void FormationWidget::restoreDefaultSettings() noexcept {
    d->moduleSettings.restoreDefaults();
}
