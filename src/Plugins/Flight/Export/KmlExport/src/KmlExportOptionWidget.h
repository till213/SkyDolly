/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef KMLEXPORTOPTIONWIDGET_H
#define KMLEXPORTOPTIONWIDGET_H

#include <memory>

#include <QWidget>

namespace Ui {
    class KmlExportOptionWidget;
}

class KmlExportSettings;
struct KmlExportOptionWidgetPrivate;

class KmlExportOptionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KmlExportOptionWidget(KmlExportSettings &pluginSettings, QWidget *parent = nullptr) noexcept;
    KmlExportOptionWidget(const KmlExportOptionWidget &rhs) = delete;
    KmlExportOptionWidget(KmlExportOptionWidget &&rhs) = delete;
    KmlExportOptionWidget &operator=(const KmlExportOptionWidget &rhs) = delete;
    KmlExportOptionWidget &operator=(KmlExportOptionWidget &&rhs) = delete;
    ~KmlExportOptionWidget() override;

private:
    const std::unique_ptr<Ui::KmlExportOptionWidget> ui;
    const std::unique_ptr<KmlExportOptionWidgetPrivate> d;

    void frenchConnection() noexcept;
    void initUi() noexcept;

private slots:
    void updateUi() noexcept;
    void selectColor(int id) noexcept;
    void onColorStyleChanged() noexcept;
};

#endif // KMLEXPORTOPTIONWIDGET_H
