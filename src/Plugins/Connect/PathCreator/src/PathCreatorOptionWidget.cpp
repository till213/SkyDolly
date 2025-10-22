/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2025 Oliver Knoll
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

#include <QComboBox>
#ifdef DEBUG
#include <QDebug>
#endif

#include <PluginManager/Connect/ConnectPluginBaseSettings.h>
#include "PathCreatorSettings.h"
#include "PathCreatorOptionWidget.h"
#include "ui_PathCreatorOptionWidget.h"

struct PathCreatorOptionWidgetPrivate
{
    PathCreatorOptionWidgetPrivate(PathCreatorSettings &pluginSettings)
        : pluginSettings {pluginSettings}
    {}

    PathCreatorSettings &pluginSettings;
};

// PUBLIC

PathCreatorOptionWidget::PathCreatorOptionWidget(PathCreatorSettings &pluginSettings, QWidget *parent)
    : OptionWidgetIntf {parent},
      ui {std::make_unique<Ui::PathCreatorOptionWidget>()},
      d {std::make_unique<PathCreatorOptionWidgetPrivate>(pluginSettings)}
{
    ui->setupUi(this);
    updateUi();
}

PathCreatorOptionWidget::~PathCreatorOptionWidget()
{
#ifdef DEBUG
    qDebug() << "PathCreatorOptionWidget::~PathCreatorOptionWidget: DELETED";
#endif
}

void PathCreatorOptionWidget::accept() noexcept
{
    switch (ui->optionComboBox->currentIndex())
    {
    case 0:
        d->pluginSettings.setOption(PathCreatorSettings::Option::A);
        break;
    case 1:
        d->pluginSettings.setOption(PathCreatorSettings::Option::B);
        break;
    case 2:
        d->pluginSettings.setOption(PathCreatorSettings::Option::C);
        break;
    default:
        break;
    }

#ifdef DEBUG
    qDebug() << "PathCreatorOptionWidget::accept: CALLED";
#endif
}

// PRIVATE

void PathCreatorOptionWidget::updateUi() noexcept
{
    switch (d->pluginSettings.getOption())
    {
    case PathCreatorSettings::Option::A:
        ui->optionComboBox->setCurrentIndex(0);
        break;
    case PathCreatorSettings::Option::B:
        ui->optionComboBox->setCurrentIndex(1);
        break;
    case PathCreatorSettings::Option::C:
        ui->optionComboBox->setCurrentIndex(2);
        break;
    default:
        break;
    }
}
