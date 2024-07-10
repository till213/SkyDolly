/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
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
#ifndef KMLIMPORTOPTIONWIDGET_H
#define KMLIMPORTOPTIONWIDGET_H

#include <memory>

#include <QWidget>

namespace Ui {
    class KmlImportOptionWidget;
}

class KmlImportSettings;
struct KmlImportOptionWidgetPrivate;

class KmlImportOptionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KmlImportOptionWidget(KmlImportSettings &pluginSettings, QWidget *parent = nullptr) noexcept;
    KmlImportOptionWidget(const KmlImportOptionWidget &rhs) = delete;
    KmlImportOptionWidget(KmlImportOptionWidget &&rhs) = delete;
    KmlImportOptionWidget &operator=(const KmlImportOptionWidget &rhs) = delete;
    KmlImportOptionWidget &operator=(KmlImportOptionWidget &&rhs) = delete;
    ~KmlImportOptionWidget() override;

private:
    const std::unique_ptr<Ui::KmlImportOptionWidget> ui;
    const std::unique_ptr<KmlImportOptionWidgetPrivate> d;

    void frenchConnection() noexcept;
    void initUi() noexcept;

private slots:
    void updateUi() noexcept;
    void onFormatChanged() noexcept;
};

#endif // KMLIMPORTOPTIONWIDGET_H

