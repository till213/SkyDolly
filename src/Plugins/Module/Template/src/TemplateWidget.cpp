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

#include <memory>


#include <QCoreApplication>
#include <QHideEvent>
#ifdef DEBUG
#include <QDebug>
#endif

#include <PluginManager/SkyConnectManager.h>
#include <PluginManager/SkyConnectIntf.h>
#include <PluginManager/AbstractModule.h>
#include "TemplateWidget.h"
#include "ui_TemplateWidget.h"

namespace
{

}

class TemplateWidgetPrivate
{
public:
    TemplateWidgetPrivate() noexcept
    {}
};

// PUBLIC

TemplateWidget::TemplateWidget(QWidget *parent) noexcept
    : QWidget(parent),
      ui(std::make_unique<Ui::TemplateWidget>()),
      d(std::make_unique<TemplateWidgetPrivate>())
{
    ui->setupUi(this);
    initUi();
    updateUi();
    frenchConnection();
#ifdef DEBUG
    qDebug() << "TemplateWidget::TemplateWidget: CREATED.";
#endif
}

TemplateWidget::~TemplateWidget() noexcept
{
#ifdef DEBUG
    qDebug() << "TemplateWidget::~TemplateWidget: DELETED.";
#endif
}


// PRIVATE

void TemplateWidget::initUi() noexcept
{

}

void TemplateWidget::frenchConnection() noexcept
{

}

// PRIVATE SLOTS

void TemplateWidget::updateUi() noexcept
{

}
