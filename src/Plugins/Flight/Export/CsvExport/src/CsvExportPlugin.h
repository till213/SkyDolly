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
#ifndef CSVEXPORTPLUGIN_H
#define CSVEXPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QWidget>

class QIODevice;
class QString;
class QDateTime;

#include <Kernel/Settings.h>
#include <PluginManager/Flight/FlightExportIntf.h>
#include <PluginManager/Flight/FlightExportPluginBase.h>

struct FlightData;
class Aircraft;

class FlightExportPluginBaseSettings;
struct CsvExportPluginPrivate;

class CsvExportPlugin : public FlightExportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FLIGHT_EXPORT_INTERFACE_IID FILE "CsvExportPlugin.json")
    Q_INTERFACES(FlightExportIntf)
public:
    CsvExportPlugin() noexcept;
    CsvExportPlugin(const CsvExportPlugin &rhs) = delete;
    CsvExportPlugin(CsvExportPlugin &&rhs) = delete;
    CsvExportPlugin &operator=(const CsvExportPlugin &rhs) = delete;
    CsvExportPlugin &operator=(CsvExportPlugin &&rhs) = delete;
    ~CsvExportPlugin() override;

protected:
    FlightExportPluginBaseSettings &getPluginSettings() const noexcept override;
    QString getFileExtension() const noexcept override;
    QString getFileFilter() const noexcept override;
    std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    bool exportFlightData(const FlightData &flightData, QIODevice &io) const noexcept override;
    bool exportAircraft(const FlightData &flightData, const Aircraft &aircraft, QIODevice &io) const noexcept override;

private:
    const std::unique_ptr<CsvExportPluginPrivate> d;
};

#endif // CSVEXPORTPLUGIN_H
