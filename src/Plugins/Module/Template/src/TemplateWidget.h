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
#ifndef LOCATIONWIDGET_H
#define LOCATIONWIDGET_H

#include <memory>

#include <QWidget>

class QTreeWidgetItem;
class QString;

#include <PluginManager/Module/AbstractModule.h>

struct TemplateWidgetPrivate;

namespace Ui {
    class TemplateWidget;
}

class TemplateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TemplateWidget(QWidget *parent = nullptr) noexcept;
    TemplateWidget(const TemplateWidget &rhs) = delete;
    TemplateWidget(TemplateWidget &&rhs) = delete;
    TemplateWidget &operator=(const TemplateWidget &rhs) = delete;
    TemplateWidget &operator=(TemplateWidget &&rhs) = delete;
    ~TemplateWidget() override;

private:
    const std::unique_ptr<Ui::TemplateWidget> ui;
    const std::unique_ptr<TemplateWidgetPrivate> d;

    void initUi() noexcept;
    void frenchConnection() noexcept;

private slots:
    void updateUi() noexcept;
};

#endif // LOCATIONWIDGET_H
