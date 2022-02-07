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
#ifndef IGCEXPORTPLUGIN_H
#define IGCEXPORTPLUGIN_H

#include <QObject>
#include <QtPlugin>

class QString;

#include "../../../ExportIntf.h"
#include "../../../PluginBase.h"

class IGCExportPluginPrivate;

class IGCExportPlugin : public PluginBase, public ExportIntf
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXPORT_INTERFACE_IID FILE "IGCExportPlugin.json")
    Q_INTERFACES(ExportIntf)
public:
    IGCExportPlugin() noexcept;
    virtual ~IGCExportPlugin() noexcept;

    virtual QWidget *getParentWidget() const noexcept override
    {
        return PluginBase::getParentWidget();
    }

    virtual void setParentWidget(QWidget *parent) noexcept override
    {
        PluginBase::setParentWidget(parent);
    }

    virtual void storeSettings(const QUuid &pluginUuid) const noexcept override
    {
        PluginBase::storeSettings(pluginUuid);
    }

    virtual void restoreSettings(const QUuid &pluginUuid) noexcept override
    {
        PluginBase::restoreSettings(pluginUuid);
    }

    virtual bool exportData() noexcept override;

private:
    std::unique_ptr<IGCExportPluginPrivate> d;
};

#endif // IGCEXPORTPLUGIN_H
