#include "SkyConnect.h"
#include "MainWindow.h"
#include "./ui_MainWindow.h"

// PUBLIC

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
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

