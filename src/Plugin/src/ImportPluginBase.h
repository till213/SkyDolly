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
#ifndef IMPORTPLUGINBASE_H
#define IMPORTPLUGINBASE_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QStringView>

class QFile;

#include "ImportIntf.h"
#include "PluginBase.h"
#include "PluginLib.h"

class FlightService;
struct AircraftType;
struct AircraftInfo;
class ImportPluginBasePrivate;

class PLUGIN_API ImportPluginBase : public PluginBase, public ImportIntf
{
    Q_OBJECT
    Q_INTERFACES(ImportIntf)
public:
    ImportPluginBase() noexcept;
    virtual ~ImportPluginBase() noexcept;

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

    virtual bool import(FlightService &flightService) noexcept override final;

protected:

    AircraftType &getSelectedAircraftType() const noexcept;

    // Re-implement
    virtual QString getFileFilter() const noexcept = 0;
    virtual QWidget *createOptionWidget() const noexcept = 0;

    virtual bool readFile(QFile &file) noexcept = 0;
    virtual QDateTime getStartDateTimeUtc() noexcept = 0;
    virtual void updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept = 0;
    virtual void updateFlight(const QFile &file) noexcept = 0;

private:
    std::unique_ptr<ImportPluginBasePrivate> d;

    bool importFile(const QString &filePath, FlightService &flightService) noexcept;
    void updateAircraftInfo() noexcept;
};

#endif // IMPORTPLUGINBASE_H
