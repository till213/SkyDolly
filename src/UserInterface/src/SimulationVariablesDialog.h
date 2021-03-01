#ifndef SIMULATIONVARIABLESDIALOG_H
#define SIMULATIONVARIABLESDIALOG_H

#include <QDialog>

class QShowEvent;
class QHideEvent;

class SkyConnect;
class SimulationVariablesDialogPrivate;

namespace Ui {
class SimulationVariablesDialog;
}

class SimulationVariablesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimulationVariablesDialog(SkyConnect &skyConnect, QWidget *parent = nullptr);
    virtual ~SimulationVariablesDialog();

private:
    SimulationVariablesDialogPrivate *d;
    Ui::SimulationVariablesDialog *ui;

    void frenchConnection();

protected:
    void showEvent (QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void updateInfoUi();
    void updateAircraftDataUi();
    void updateTitle();

private:
    const AircraftData &getAircraftData() const;
};

#endif // SIMULATIONVARIABLESDIALOG_H
