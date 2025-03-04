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
#ifndef LOCATIONPLUGIN_H
#define LOCATIONPLUGIN_H

#include <QObject>
#include <QString>

class QWidget;

#include <PluginManager/Module/AbstractModule.h>

class ModuleBaseSettings;
struct TemplatePluginPrivate;

class TemplatePlugin : public AbstractModule
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MODULE_INTERFACE_IID FILE "TemplatePlugin.json")
    Q_INTERFACES(ModuleIntf)
public:
    explicit TemplatePlugin(QObject *parent = nullptr) noexcept;
    TemplatePlugin(const TemplatePlugin &rhs) = delete;
    TemplatePlugin(TemplatePlugin &&rhs) = delete;
    TemplatePlugin &operator=(const TemplatePlugin &rhs) = delete;
    TemplatePlugin &operator=(TemplatePlugin &&rhs) = delete;
    ~TemplatePlugin() override;

    QUuid getUuid() const noexcept override;
    QString getModuleName() const noexcept override;
    QWidget *getWidget() const noexcept override;

protected:
    ModuleBaseSettings &getModuleSettings() const noexcept override;

private:
    const std::unique_ptr<TemplatePluginPrivate> d;
};

#endif // LOCATIONPLUGIN_H
