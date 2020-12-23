#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "SkyConnect.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    SkyConnect m_skyConnect;

    void frenchConnection();

private slots:
    void on_connectionPushButton_clicked();
    void on_recordPushButton_clicked(bool checked = false);
    void on_clearPushButton_clicked();
    void on_replayPushButton_clicked(bool checked = false);

    void updateUi();
    void updateInfoUi();
    void updatePositionUi();
};
#endif // MAINWINDOW_H
