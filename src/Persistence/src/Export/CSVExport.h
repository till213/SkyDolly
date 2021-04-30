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
#ifndef CSVEXPORT_H
#define CSVEXPORT_H

class QString;
class QIODevice;

class Aircraft;
class AircraftData;
class EngineData;
class PrimaryFlightControlData;
class SecondaryFlightControlData;
class AircraftHandleData;
class LightData;
class CSVExportPrivate;

#include "../PersistenceLib.h"

/*!
 * Exports the Aircraft data as comma-separated values (CSV).
 */
class CSVExport
{
public:
    PERSISTENCE_API static bool exportData(const Aircraft &aircraft, QIODevice &io) noexcept;

private:
    CSVExportPrivate *d;

    static QString getAircraftHeader() noexcept;
    static QString getAircraftData(const AircraftData &data) noexcept;

    static QString getEngineHeader() noexcept;
    static QString getEngineData(const EngineData &data) noexcept;

    static QString getPrimaryFlightControlHeader() noexcept;
    static QString getPrimaryFlightControlData(const PrimaryFlightControlData &data) noexcept;

    static QString getSecondaryFlightControlHeader() noexcept;
    static QString getSecondaryFlightControlData(const SecondaryFlightControlData &data) noexcept;

    static QString getAircraftHandleHeader() noexcept;
    static QString getAircraftHandleData(const AircraftHandleData &data) noexcept;

    static QString getLighteHeader() noexcept;
    static QString getLightData(const LightData &data) noexcept;
};

#endif // CSVEXPORT_H
