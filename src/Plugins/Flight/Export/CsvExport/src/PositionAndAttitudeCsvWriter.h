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
#ifndef POSITIONANDATTITUDECSVWRITER_H
#define POSITIONANDATTITUDECSVWRITER_H

#include <memory>

class QIODevice;

#include "CsvWriterIntf.h"

struct FlightData;
class Aircraft;
class CsvExportSettings;
class PositionData;
struct PositionAndAttitudeCsvWriterPrivate;

class PositionAndAttitudeCsvWriter : public CsvWriterIntf
{
public:
    PositionAndAttitudeCsvWriter(const CsvExportSettings &pluginSettings) noexcept;
    PositionAndAttitudeCsvWriter(const PositionAndAttitudeCsvWriter &rhs) = delete;
    PositionAndAttitudeCsvWriter(PositionAndAttitudeCsvWriter &&rhs) = delete;
    PositionAndAttitudeCsvWriter &operator=(const PositionAndAttitudeCsvWriter &rhs) = delete;
    PositionAndAttitudeCsvWriter &operator=(PositionAndAttitudeCsvWriter &&rhs) = delete;
    ~PositionAndAttitudeCsvWriter() override;

    bool write(const FlightData &flightData, const Aircraft &aircraft, QIODevice &ioDevice) const noexcept override;

private:
    const std::unique_ptr<PositionAndAttitudeCsvWriterPrivate> d;
};

#endif // POSITIONANDATTITUDECSVWRITER_H
