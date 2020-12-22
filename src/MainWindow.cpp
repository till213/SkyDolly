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
    connect(&m_skyConnect, &SkyConnect::aircraftChanged,
            this, &MainWindow::updateUi);
}


// PRIVATE SLOTS

void MainWindow::on_connectionPushButton_clicked() {
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

void MainWindow::on_recordPushButton_clicked(bool checked) {
    qDebug("on_recordPushButton_clicked");

    if (checked) {
        m_skyConnect.startDataSample();
    } else {
        m_skyConnect.stopDataSample();
    }
}

void MainWindow::updateUi()
{
    const Aircraft &aircraft = m_skyConnect.getAircraft();
    ui->latitudeLineEdit->setText(QString::number(aircraft.getLatitude()));
    ui->longitudeLineEdit->setText(QString::number(aircraft.getLongitude()));
    ui->altitudeLineEdit->setText(QString::number(aircraft.getAltitude()));
}

