#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    virtual ~SettingsDialog();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    Ui::SettingsDialog *ui;

    void initUi();
    void frenchConnection();

private slots:
    void updateUi();
    void handleAccepted();
};

#endif // SETTINGSDIALOG_H
