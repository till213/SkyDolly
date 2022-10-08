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
#ifndef EXPORTPLUGINBASE_H
#define EXPORTPLUGINBASE_H

#include <memory>
#include <vector>
#include <iterator>

#include <QObject>
#include <QtPlugin>
#include <QStringView>

class QIODevice;

#include <Flight/FlightAugmentation.h>
#include <Kernel/Settings.h>
#include "ExportIntf.h"
#include "PluginBase.h"
#include "PluginManagerLib.h"

struct PositionData;
class FLight;
class Aircraft;
class ExportPluginBaseSettings;
struct ExportPluginBasePrivate;

class PLUGINMANAGER_API ExportPluginBase : public PluginBase, public ExportIntf
{
    Q_OBJECT
    Q_INTERFACES(ExportIntf)
public:
    ExportPluginBase() noexcept;
    ExportPluginBase(const ExportPluginBase &rhs) = delete;
    ExportPluginBase(ExportPluginBase &&rhs) = delete;
    ExportPluginBase &operator=(const ExportPluginBase &rhs) = delete;
    ExportPluginBase &operator=(ExportPluginBase &&rhs) = delete;
    ~ExportPluginBase() override;

    QWidget *getParentWidget() const noexcept final
    {
        return PluginBase::getParentWidget();
    }

    void setParentWidget(QWidget *parent) noexcept final
    {
        PluginBase::setParentWidget(parent);
    }

    void storeSettings(const QUuid &pluginUuid) const noexcept final
    {
        PluginBase::storeSettings(pluginUuid);
    }

    void restoreSettings(const QUuid &pluginUuid) noexcept final
    {
        PluginBase::restoreSettings(pluginUuid);
    }

    bool exportFlight(const Flight &flight) noexcept final;

protected:
    // Re-implement
    virtual ExportPluginBaseSettings &getPluginSettings() const noexcept = 0;
    virtual QString getFileSuffix() const noexcept = 0;
    virtual QString getFileFilter() const noexcept = 0;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept = 0;

    /*!
     * Returns whether the plugin (file format) supports export of multiple aircraft into a single file.
     * Examples are the KML or GPX format which both may have multiple \e tracks.
     *
     * \return \c true if the file format supports multiple aircraft tracks; \c false else
     */
    virtual bool hasMultiAircraftSupport() const noexcept = 0;
    virtual bool exportFlight(const Flight &flight, QIODevice &io) noexcept = 0;
    virtual bool exportAircraft(const Flight &flight, const Aircraft &aircraft, QIODevice &io) noexcept = 0;

private:
    const std::unique_ptr<ExportPluginBasePrivate> d;

    bool exportFlight(const Flight &flight, const QString &filePath) noexcept;
    // Exports all aircraft into separate files, given the 'baseFilePath'
    bool exportAllAircraft(const Flight &flight, const QString &baseFilePath) noexcept;

    void addSettings(Settings::KeyValues &keyValues) const noexcept final;
    void addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept final;
    void restoreSettings(Settings::ValuesByKey valuesByKey) noexcept final;
};

#endif // EXPORTPLUGINBASE_H
