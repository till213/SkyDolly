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
#ifndef CSVEXPORTPLUGIN_H
#define CSVEXPORTPLUGIN_H

#include <memory>

#include <QObject>
#include <QtPlugin>
#include <QWidget>

class QIODevice;
class QString;
class QDateTime;

#include "../../../../../Kernel/src/Settings.h"
#include "../../../ExportIntf.h"
#include "../../../ExportPluginBase.h"

class Aircraft;
struct PositionData;
struct EngineData;
struct PrimaryFlightControlData;
struct SecondaryFlightControlData;
struct AircraftHandleData;
struct LightData;
class CSVExportPluginPrivate;

class CSVExportPlugin : public ExportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID EXPORT_INTERFACE_IID FILE "CSVExportPlugin.json")
    Q_INTERFACES(ExportIntf)
public:
    CSVExportPlugin() noexcept;
    virtual ~CSVExportPlugin() noexcept;

protected:
    virtual void addSettingsExtn(Settings::KeyValues &keyValues) const noexcept override;
    virtual void addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept override;
    virtual void restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept override;
    virtual ExportPluginBaseSettings &getSettings() const noexcept override;
    virtual QString getFileExtension() const noexcept override;
    virtual QString getFileFilter() const noexcept override;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept override;
    virtual bool writeFile(QIODevice &io) noexcept override;

protected slots:
    virtual void onRestoreDefaultSettings() noexcept override;

private:
    std::unique_ptr<CSVExportPluginPrivate> d;

    static QString getPositionHeader() noexcept;
    static QString getPositionData(const PositionData &data) noexcept;

    static QString getEngineHeader() noexcept;
    static QString getEngineData(const EngineData &data) noexcept;

    static QString getPrimaryFlightControlHeader() noexcept;
    static QString getPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept;

    static QString getSecondaryFlightControlHeader() noexcept;
    static QString getSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept;

    static QString getAircraftHandleHeader() noexcept;
    static QString getAircraftHandleData(const AircraftHandleData &data) noexcept;

    static QString getLightHeader() noexcept;
    static QString getLightData(const LightData &data) noexcept;

    static inline bool writeLine(QChar type,
                                 const PositionData &positionData,
                                 const EngineData &engineData,
                                 const PrimaryFlightControlData &flightControlData,
                                 const SecondaryFlightControlData &secondaryFlightControlData,
                                 const AircraftHandleData &aircraftHandleData,
                                 const LightData &lightData,
                                 std::int64_t timestamp,
                                 QIODevice &io) noexcept;
};

#endif // CSVEXPORTPLUGIN_H
