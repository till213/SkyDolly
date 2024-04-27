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
#include <memory>

#include <QObject>
#include <QCoreApplication>

#include <PluginManager/Module/ModuleBaseSettings.h>
#include "TemplateWidget.h"
#include "TemplateSettings.h"
#include "TemplatePlugin.h"

namespace {
    constexpr const char *TemplateModuleUuid {"2c77a56a-2c20-49f7-83ea-55da2dfd22f0"};
}

struct TemplatePluginPrivate
{
    TemplatePluginPrivate()
        : locationWidget(std::make_unique<TemplateWidget>())
    {}

    std::unique_ptr<TemplateWidget> locationWidget;
    TemplateSettings moduleSettings;
};

// PUBLIC

TemplatePlugin::TemplatePlugin(QObject *parent) noexcept
    : AbstractModule(parent),
      d(std::make_unique<TemplatePluginPrivate>())
{
    restoreSettings(QUuid(::TemplateModuleUuid));
}

TemplatePlugin::~TemplatePlugin()
{
    storeSettings(QUuid(::TemplateModuleUuid));
};

QUuid TemplatePlugin::getUuid() const noexcept
{
    static const QUuid uuid {::TemplateModuleUuid};
    return uuid;
}

QString TemplatePlugin::getModuleName() const noexcept
{
    return QCoreApplication::translate("TemplatePlugin", "Template");
}

QWidget *TemplatePlugin::getWidget() const noexcept
{
    return d->locationWidget.get();
}

// PROTECTED

ModuleBaseSettings &TemplatePlugin::getModuleSettings() const noexcept
{
    return d->moduleSettings;
}
