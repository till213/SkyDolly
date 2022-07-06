/**
 * Sky Dolly - The Black Sheep for your Flight Recordings
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
#ifndef LOCATIONWIDGET_H
#define LOCATIONWIDGET_H

#include <memory>
#include <forward_list>
#include <cstdint>

#include <QWidget>

class QKeyEvent;

#include <PluginManager/ModuleIntf.h>
#include <PluginManager/AbstractModule.h>

#include <Model/Location.h>
class LocationWidgetPrivate;

namespace Ui {
    class LocationWidget;
}

class LocationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LocationWidget(QWidget *parent = nullptr) noexcept;
    ~LocationWidget() noexcept override;

    void addLocation(Location location);

    void keyPressEvent(QKeyEvent *event) noexcept override;

signals:
    void captureLocation();
    void teleportTo(Location location);

private:
    std::unique_ptr<Ui::LocationWidget> ui;
    std::unique_ptr<LocationWidgetPrivate> d;

    void initUi() noexcept;
    void frenchConnection() noexcept;
    void updateInfoUi() noexcept;

    void updateLocationTable() noexcept;
    inline void updateLocationItems(const Location &location, int rowIndex) noexcept;

    void teleportToLocation(int row) noexcept;
    Location getLocationByRow(int row) const noexcept;

    void tryPasteLocation() noexcept;

private slots:
    void updateUi() noexcept;
    void updateEditUi() noexcept;

    void onCellSelected(int row, int column) noexcept;
    void onCellChanged(int row, int column) noexcept;
    void onSelectionChanged() noexcept;

    void onAddLocation() noexcept;
    void onCaptureLocation() noexcept;
    void onTeleportToSelectedLocation() noexcept;
    void onDeleteLocation() noexcept;

    void onDescriptionChanged() noexcept;
};

#endif // LOCATIONWIDGET_H
