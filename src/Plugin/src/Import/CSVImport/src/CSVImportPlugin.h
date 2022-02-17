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
#ifndef CSVIMPORTPLUGIN_H
#define CSVIMPORTPLUGIN_H

#include <memory>
#include <cstdint>

#include <QObject>
#include <QtPlugin>

class QFile;

#include "../../../../../Flight/src/FlightAugmentation.h"
#include "../../../ImportIntf.h"
#include "../../../ImportPluginBase.h"

class Aircraft;
struct AircraftType;
class Engine;
class PrimaryFlightControl;
class SecondaryFlightControl;
class AircraftHandle;
class Light;
class FlightService;
class CSVImportPluginPrivate;

class CSVImportPlugin : public ImportPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IMPORT_INTERFACE_IID FILE "CSVImportPlugin.json")
    Q_INTERFACES(ImportIntf)
public:
    CSVImportPlugin() noexcept;
    virtual ~CSVImportPlugin() noexcept;

protected:
    // PluginBase
    virtual Settings::PluginSettings getSettings() const noexcept override;
    virtual Settings::KeysWithDefaults getKeyWithDefaults() const noexcept override;
    virtual void setSettings(Settings::ValuesByKey) noexcept override;

    // ImportPluginBase
    virtual QString getFileFilter() const noexcept override;
    virtual std::unique_ptr<QWidget> createOptionWidget() const noexcept override;

    virtual bool readFile(QFile &file) noexcept override;

    virtual FlightAugmentation::Procedures getProcedures() const noexcept override;
    virtual FlightAugmentation::Aspects getAspects() const noexcept override;
    virtual QDateTime getStartDateTimeUtc() noexcept override;
    virtual void updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept override;
    virtual void updateFlight(const QFile &file) noexcept override;

protected slots:
    virtual void onRestoreDefaultSettings() noexcept override;

private:
    std::unique_ptr<CSVImportPluginPrivate> d;

    bool getAircraftType(const QString &type, AircraftType &aircraftType) noexcept;

    static inline bool importPositionData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Aircraft &aircraft) noexcept;
    static inline bool importEngineData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Engine &engine) noexcept;
    static inline bool importPrimaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, PrimaryFlightControl &primaryFlightControl) noexcept;
    static inline bool importSecondaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, SecondaryFlightControl &secondaryFlightControl) noexcept;
    static inline bool importAircraftHandleData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, AircraftHandle &aircraftHandle) noexcept;
    static inline bool importLightData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Light &light) noexcept;
    static inline bool importTimestamp(const QList<QByteArray> &values, int columnIndex, bool firstRow, std::int64_t &timestamp, std::int64_t &timestampDelta);
};

#endif // CSVIMPORTPLUGIN_H
