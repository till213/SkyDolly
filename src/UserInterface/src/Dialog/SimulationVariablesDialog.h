/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef SIMULATIONVARIABLESDIALOG_H
#define SIMULATIONVARIABLESDIALOG_H

#include <memory>

#include <QDialog>

class QShowEvent;
class QHideEvent;

struct SimulationVariablesDialogPrivate;

namespace Ui {
    class SimulationVariablesDialog;
}

class SimulationVariablesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SimulationVariablesDialog(QWidget *parent = nullptr) noexcept;
    SimulationVariablesDialog(const SimulationVariablesDialog &rhs) = delete;
    SimulationVariablesDialog(SimulationVariablesDialog &&rhs) = delete;
    SimulationVariablesDialog &operator=(const SimulationVariablesDialog &rhs) = delete;
    SimulationVariablesDialog &operator=(SimulationVariablesDialog &&rhs) = delete;
    ~SimulationVariablesDialog() override;

signals:
    void visibilityChanged(bool visible);

protected:
    void showEvent(QShowEvent *event) noexcept override;
    void hideEvent(QHideEvent *event) noexcept override;

private:
    const std::unique_ptr<Ui::SimulationVariablesDialog> ui;
    const std::unique_ptr<SimulationVariablesDialogPrivate> d;    

    void initUi() noexcept;
    void updateUi() noexcept;
    void frenchConnection() noexcept;

private slots:    
    void updateTitle() noexcept;
};

#endif // SIMULATIONVARIABLESDIALOG_H
