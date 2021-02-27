#include <QObject>

#include "../../Kernel/src/Aircraft.h"
#include "../../Kernel/src/AircraftInfo.h"
#include "../../SkyConnect/src/SkyConnect.h"
#include "SimulationVariablesDialog.h"
#include "ui_SimulationVariablesDialog.h"

class SimulationVariablesDialogPrivate
{
public:
    SimulationVariablesDialogPrivate(SkyConnect &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnect &skyConnect;
};

// PUBLIC

SimulationVariablesDialog::SimulationVariablesDialog(SkyConnect &skyConnect, QWidget *parent) :
    QDialog(parent),
    d(new SimulationVariablesDialogPrivate(skyConnect)),
    ui(new Ui::SimulationVariablesDialog)
{
    ui->setupUi(this);
}

SimulationVariablesDialog::~SimulationVariablesDialog()
{
    delete ui;
    delete d;
}

// PROTECTED

void SimulationVariablesDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    frenchConnection();
}

void SimulationVariablesDialog::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    d->skyConnect.getAircraft().disconnect(this);
}

// PRIVATE

void SimulationVariablesDialog::frenchConnection()
{
    const Aircraft &aircraft = d->skyConnect.getAircraft();
    connect(&aircraft, &Aircraft::infoChanged,
            this, &SimulationVariablesDialog::updateInfoUi);
    connect(&aircraft, &Aircraft::dataChanged,
            this, &SimulationVariablesDialog::updateAircraftDataUi);
}

// PRIVATE SLOTS

void SimulationVariablesDialog::updateInfoUi()
{
    const Aircraft &aircraft = d->skyConnect.getAircraft();
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();

    ui->nameLineEdit->setText(aircraftInfo.name);
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);
    ui->initialAirspeedLineEdit->setText(QString::number(aircraftInfo.initialAirspeed));
}

void SimulationVariablesDialog::updateAircraftDataUi()
{
    const Aircraft &aircraft = d->skyConnect.getAircraft();
    const AircraftData &aircraftData = aircraft.getLastAircraftData();

    ui->latitudeLineEdit->setText(QString::number(aircraftData.latitude));
    ui->longitudeLineEdit->setText(QString::number(aircraftData.longitude));
    ui->altitudeLineEdit->setText(QString::number(aircraftData.altitude));
    ui->pitchLineEdit->setText(QString::number(aircraftData.pitch));
    ui->bankLineEdit->setText(QString::number(aircraftData.bank));
    ui->headingLineEdit->setText(QString::number(aircraftData.heading));

    ui->yokeXLineEdit->setText(QString::number(aircraftData.yokeXPosition));
    ui->yokeYLineEdit->setText(QString::number(aircraftData.yokeYPosition));
    ui->rudderLineEdit->setText(QString::number(aircraftData.rudderPosition));
    ui->elevatorLineEdit->setText(QString::number(aircraftData.elevatorPosition));
    ui->aileronLineEdit->setText(QString::number(aircraftData.aileronPosition));
    ui->throttle1LineEdit->setText(QString::number(aircraftData.throttleLeverPosition1));
    ui->throttle2LineEdit->setText(QString::number(aircraftData.throttleLeverPosition2));
    ui->throttle3LineEdit->setText(QString::number(aircraftData.throttleLeverPosition3));
    ui->throttle4LineEdit->setText(QString::number(aircraftData.throttleLeverPosition4));
    ui->spoilerLineEdit->setText(QString::number(aircraftData.spoilersHandlePosition));
    ui->flapsPositionLineEdit->setText(QString::number(aircraftData.flapsHandleIndex));
}
