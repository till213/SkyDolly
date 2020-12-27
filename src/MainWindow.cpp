#include <QByteArray>
#include <QString>
#include <QTime>

#include "Aircraft.h"
#include "SkyConnect.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"

// PUBLIC

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->updateUi();
    this->frenchConnection();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// PRIVATE

void MainWindow::frenchConnection()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    connect(&aircraft, &Aircraft::infoChanged,
            this, &MainWindow::updateInfoUi);
    connect(&aircraft, &Aircraft::positionChanged,
            this, &MainWindow::updatePositionUi);
}

// PRIVATE SLOTS

void MainWindow::on_connectionPushButton_clicked()
{
    qDebug("on_connnectionPushButton_clicked");

    if (m_skyConnect.isConnected()) {
        m_skyConnect.close();
        ui->connectionStatusLineEdit->setText(tr("Disconnected."));
        ui->connectionPushButton->setText(tr("Connect"));
    } else {
        bool res = m_skyConnect.open();
        if (res) {
            this->ui->connectionStatusLineEdit->setText(tr("Connected."));
            ui->connectionPushButton->setText(tr("Disconnect"));
        } else {
            this->ui->connectionStatusLineEdit->setText(tr("Error."));
        }
    }
}

void MainWindow::on_recordPushButton_clicked(bool checked)
{
    qDebug("on_recordPushButton_clicked");

    if (checked) {
        m_skyConnect.startDataSample();
    } else {
        m_skyConnect.stopDataSample();
    }
}

void MainWindow::on_clearPushButton_clicked()
{
    Aircraft &aircraft = m_skyConnect.getAircraft();

    aircraft.clear();
}

void MainWindow::on_replayPushButton_clicked(bool checked)
{
    qDebug("on_replayPushButton_clicked");

    if (checked) {
        m_skyConnect.startReplay();
    } else {
        m_skyConnect.stopReplay()   ;
    }
}

void MainWindow::updateUi()
{
    updateInfoUi();
    updatePositionUi();
}

void MainWindow::updateInfoUi()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    const QByteArray &name = aircraft.getName();

    ui->nameLineEdit->setText(name);
}

void MainWindow::updatePositionUi()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    const Position &position = aircraft.getLastPosition();

    ui->nofPositionsLineEdit->setText(QString::number(aircraft.getPositions().length()));
    ui->latitudeLineEdit->setText(QString::number(position.latitude));
    ui->longitudeLineEdit->setText(QString::number(position.longitude));
    ui->altitudeLineEdit->setText(QString::number(position.altitude));
    ui->pitchLineEdit->setText(QString::number(position.pitch));
    ui->bankLineEdit->setText(QString::number(position.bank));
    ui->headingLineEdit->setText(QString::number(position.heading));
    QTime time = QTime::fromMSecsSinceStartOfDay(position.timestamp);
    ui->timestampTimeEdit->setTime(time);
}


