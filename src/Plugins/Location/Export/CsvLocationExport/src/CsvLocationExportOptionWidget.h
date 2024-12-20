/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation ios (the "Software"), to deal in the Software
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
#ifndef CSVLOCATIONEXPORTOPTIONWIDGET_H
#define CSVLOCATIONEXPORTOPTIONWIDGET_H

#include <memory>

#include <QWidget>

namespace Ui {
    class CsvLocationExportOptionWidget;
}

class CsvLocationExportSettings;
struct CsvLocationExportOptionWidgetPrivate;

class CsvLocationExportOptionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CsvLocationExportOptionWidget(CsvLocationExportSettings &pluginSettings, QWidget *parent = nullptr) noexcept;
    CsvLocationExportOptionWidget(const CsvLocationExportOptionWidget &rhs) = delete;
    CsvLocationExportOptionWidget(CsvLocationExportOptionWidget &&rhs) = delete;
    CsvLocationExportOptionWidget &operator=(const CsvLocationExportOptionWidget &rhs) = delete;
    CsvLocationExportOptionWidget &operator=(CsvLocationExportOptionWidget &&rhs) = delete;
    ~CsvLocationExportOptionWidget() override;

private:
    const std::unique_ptr<Ui::CsvLocationExportOptionWidget> ui;
    const std::unique_ptr<CsvLocationExportOptionWidgetPrivate> d;

    void frenchConnection() noexcept;
    void initUi() noexcept;

private slots:
    void updateUi() noexcept;
    void onFormatChanged() noexcept;
};

#endif // CSVLOCATIONEXPORTOPTIONWIDGET_H
