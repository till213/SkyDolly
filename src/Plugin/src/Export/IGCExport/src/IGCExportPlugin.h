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
class QDateTime;

#include "../../../ExportIntf.h"
#include "../../../PluginBase.h"

class Aircraft;

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

    bool exportIGCFile(const Aircraft &aircraft, QIODevice &io) const noexcept;
    inline bool exportARecord(QIODevice &io) const noexcept;
    inline bool exportHRecord(const Aircraft &aircraft, QIODevice &io) const noexcept;
    inline bool exportIRecord(QIODevice &io) const noexcept;
    inline bool exportBRecord(const Aircraft &aircraft, QIODevice &io) const noexcept;
    inline bool exportGRecord(QIODevice &io) const noexcept;

    inline QByteArray formatDate(const QDateTime &date) const noexcept;
    inline QByteArray formatTime(const QDateTime &date) const noexcept;
    inline QByteArray formatNumber(int value, int padding) const noexcept;
    inline QByteArray formatLatitude(double latitude) const noexcept;
    inline QByteArray formatLongitude(double longitude) const noexcept;
};

#endif // IGCEXPORTPLUGIN_H
