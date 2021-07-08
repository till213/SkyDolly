/**
 * Sky Dolly - The black sheep for your flight recordings
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

#include <QObject>
#include <QtPlugin>

class Aircraft;
class Engine;
class PrimaryFlightControl;
class SecondaryFlightControl;
class AircraftHandle;
class Light;
class FlightService;
class CSVExportPrivate;

#include "../../../ImportIntf.h"

class CSVImportPlugin : public QObject, public ImportIntf
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IMPORT_INTERFACE_IID FILE "CSVImportPlugin.json")
    Q_INTERFACES(ImportIntf)
public:
    CSVImportPlugin() noexcept;
    virtual ~CSVImportPlugin() noexcept;

    virtual bool importData(FlightService &flightService) const noexcept override;

private:
    static bool importPositionData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Aircraft &aircraft) noexcept;
    static bool importEngineData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Engine &engine) noexcept;
    static bool importPrimaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, PrimaryFlightControl &primaryFlightControl) noexcept;
    static bool importSecondaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, SecondaryFlightControl &secondaryFlightControl) noexcept;
    static bool importAircraftHandleData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, AircraftHandle &aircraftHandle) noexcept;
    static bool importLightData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Light &light) noexcept;
    static bool importTimestamp(const QList<QByteArray> &values, int columnIndex, bool firstRow, qint64 &timestamp, qint64 &timestampDelta);
};

#endif // CSVIMPORTPLUGIN_H
