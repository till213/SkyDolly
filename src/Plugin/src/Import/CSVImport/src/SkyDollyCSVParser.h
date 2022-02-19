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
#ifndef SKYDOLLYCSVPARSER_H
#define SKYDOLLYCSVPARSER_H

#include <memory>

#include <QByteArray>
#include <QList>

class QDateTime;
class QString;
class QFile;

#include "CSVParserIntf.h"

class Aircraft;
struct AircraftType;
class Engine;
class PrimaryFlightControl;
class SecondaryFlightControl;
class AircraftHandle;
class Light;
class FlightService;
class CSVImportPluginPrivate;

class SkyDollyCSVParser : public CSVParserIntf
{
public:
    SkyDollyCSVParser() noexcept;
    virtual ~SkyDollyCSVParser() noexcept;

    virtual bool parse(QFile &file) noexcept override;

private:
    static inline bool importPositionData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Aircraft &aircraft) noexcept;
    static inline bool importEngineData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Engine &engine) noexcept;
    static inline bool importPrimaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, PrimaryFlightControl &primaryFlightControl) noexcept;
    static inline bool importSecondaryFlightControlData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, SecondaryFlightControl &secondaryFlightControl) noexcept;
    static inline bool importAircraftHandleData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, AircraftHandle &aircraftHandle) noexcept;
    static inline bool importLightData(const QList<QByteArray> &headers, const QList<QByteArray> &values, bool firstRow, Light &light) noexcept;
    static inline bool importTimestamp(const QList<QByteArray> &values, int columnIndex, bool firstRow, std::int64_t &timestamp, std::int64_t &timestampDelta);
};

#endif // SKYDOLLYCSVPARSER_H
