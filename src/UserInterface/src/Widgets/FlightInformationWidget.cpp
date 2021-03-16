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

#include <QDialog>

#include "../../../Kernel/src/Const.h"
#include "../../../Kernel/src/Aircraft.h"
#include "../../../Kernel/src/AircraftInfo.h"
#include "../../../SkyConnect/src/SkyConnectIntf.h"
#include "../../../SkyConnect/src/Connect.h"
#include "FlightInformationWidget.h"
#include "ui_FlightInformationWidget.h"

class FlightInformationWidgetPrivate
{
public:
    FlightInformationWidgetPrivate(SkyConnectIntf &theSkyConnect)
        : skyConnect(theSkyConnect)
    {}

    SkyConnectIntf &skyConnect;
    static const QString WindowTitle;
};

const QString FlightInformationWidgetPrivate::WindowTitle = QT_TRANSLATE_NOOP("FlightInformationWidget", "Simulation Variables");

// PUBLIC

FlightInformationWidget::FlightInformationWidget(SkyConnectIntf &skyConnect, QWidget *parent) :
    QDialog(parent),
    d(std::make_unique<FlightInformationWidgetPrivate>(skyConnect)),
    ui(new Ui::FlightInformationWidget)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;
    setWindowFlags(flags);

    initUi();

}

FlightInformationWidget::~FlightInformationWidget()
{
    delete ui;
}

// PROTECTED

void FlightInformationWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    updateUi();

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    connect(&aircraft, &Aircraft::infoChanged,
            this, &FlightInformationWidget::updateInfoUi);
}

void FlightInformationWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    const Aircraft &aircraft = d->skyConnect.getAircraft();
    disconnect(&aircraft, &Aircraft::infoChanged,
            this, &FlightInformationWidget::updateInfoUi);
}

// PRIVATE

void FlightInformationWidget::initUi()
{
    // Make the "on ground" checkbox checkable, but not for the user
    ui->startOnGroundCheckBox->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->startOnGroundCheckBox->setFocusPolicy(Qt::NoFocus);
}

void FlightInformationWidget::updateUi()
{
    updateInfoUi();
}

// PRIVATE SLOTS

void FlightInformationWidget::updateInfoUi()
{
    const Aircraft &aircraft = d->skyConnect.getAircraft();
    const AircraftInfo &aircraftInfo = aircraft.getAircraftInfo();

    ui->nameLineEdit->setText(aircraftInfo.name);
    ui->tailNumberLineEdit->setText(aircraftInfo.atcId);
    ui->airlineLineEdit->setText(aircraftInfo.atcAirline);
    ui->flightLineEdit->setText(aircraftInfo.atcFlightNumber);
    ui->categoryLineEdit->setText(aircraftInfo.category);

    ui->initialAirspeedLineEdit->setText(QString::number(aircraftInfo.initialAirspeed));
    ui->surfaceTypeLineEdit->setText(SimTypes::surfaceTypeToString(aircraftInfo.surfaceType));
    ui->wingSpanLineEdit->setText(QString::number(aircraftInfo.wingSpan));
    ui->engineTypeLineEdit->setText(SimTypes::engineTypeToString(aircraftInfo.engineType));
    ui->numberOfEnginesLineEdit->setText(QString::number(aircraftInfo.numberOfEngines));
    ui->startOnGroundCheckBox->setChecked(aircraftInfo.startOnGround);
}
