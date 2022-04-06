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
#ifndef EXPORTPLUGINBASE_H
#define EXPORTPLUGINBASE_H

#include <memory>
#include <vector>
#include <iterator>

#include <QObject>
#include <QtPlugin>
#include <QStringView>

class QIODevice;

#include "../../Kernel/src/Settings.h"
#include "../../Flight/src/FlightAugmentation.h"
#include "ExportIntf.h"
#include "PluginBase.h"
#include "PluginLib.h"

struct PositionData;
class FLight;
class Aircraft;
class ExportPluginBaseSettings;
class ExportPluginBasePrivate;

class PLUGIN_API ExportPluginBase : public PluginBase, public ExportIntf
{
    Q_OBJECT
    Q_INTERFACES(ExportIntf)
public:
    ExportPluginBase() noexcept;
    virtual ~ExportPluginBase() noexcept;

    virtual QWidget *getParentWidget() const noexcept override final
    {
        return PluginBase::getParentWidget();
    }

    virtual void setParentWidget(QWidget *parent) noexcept override final
    {
        PluginBase::setParentWidget(parent);
    }

    virtual void storeSettings(const QUuid &pluginUuid) const noexcept override final
    {
        PluginBase::storeSettings(pluginUuid);
    }

    virtual void restoreSettings(const QUuid &pluginUuid) noexcept override final
    {
        PluginBase::restoreSettings(pluginUuid);
    }

    virtual bool exportFlight(const Flight &flight) noexcept override final;

protected:
    void resamplePositionDataForExport(const Aircraft &aircraft, std::back_insert_iterator<std::vector<PositionData>> backInsertIterator) const noexcept;

    // Re-implement
    virtual ExportPluginBaseSettings &getSettings() const noexcept = 0;
    virtual QString getFileExtension() const noexcept = 0;
    virtual QString getFileFilter() const noexcept = 0;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept = 0;
    virtual bool exportFlight(const Flight &flight, QIODevice &io) noexcept = 0;

protected slots:
    virtual void onRestoreDefaultSettings() noexcept = 0;

private:
    std::unique_ptr<ExportPluginBasePrivate> d;

    bool exportFlight(const Flight &flight, const QString &filePath) noexcept;
    virtual void addSettings(Settings::KeyValues &keyValues) const noexcept override final;
    virtual void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override final;
    virtual void restoreSettings(Settings::ValuesByKey valuesByKey) noexcept override final;
};

#endif // EXPORTPLUGINBASE_H
