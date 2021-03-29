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

#include "../ModelLib.h"

/*!
 * Exports the Aircraft data as comma-separated values (CSV).
 */
class CSVExport
{
public:
    MODEL_API static bool exportData(const Aircraft &aircraft, QIODevice &io) noexcept;

private:
    CSVExportPrivate *d;

    static void appendAircraftHeader(QString &header) noexcept;
    static void appendAircraftData(const AircraftData &data, QString &csv) noexcept;

    static void appendEngineHeader(QString &header) noexcept;
    static void appendEngineData(const EngineData &data, QString &csv) noexcept;

    static void appendPrimaryFlightControlHeader(QString &header) noexcept;
    static void appendPrimaryFlightControlData(const PrimaryFlightControlData &data, QString &csv) noexcept;

    static void appendSecondaryFlightControlHeader(QString &header) noexcept;
    static void appendSecondaryFlightControlData(const SecondaryFlightControlData &data, QString &csv) noexcept;

    static void appendAircraftHandleHeader(QString &header) noexcept;
    static void appendAircraftHandleData(const AircraftHandleData &data, QString &csv) noexcept;

    static void appendLighteHeader(QString &header) noexcept;
    static void appendLightData(const LightData &data, QString &csv) noexcept;
};

#endif // CSVEXPORT_H
