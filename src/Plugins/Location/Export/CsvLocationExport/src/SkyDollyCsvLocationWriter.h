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
#ifndef SKYDOLLYCSVLOCATIONWRITER_H
#define SKYDOLLYCSVLOCATIONWRITER_H

#include <memory>
#include <vector>

class QIODevice;

#include "CsvLocationWriterIntf.h"

struct Location;
class CsvLocationExportSettings;
struct SkyDollyCsvLocationWriterPrivate;

class SkyDollyCsvLocationWriter : public CsvLocationWriterIntf
{
public:
    SkyDollyCsvLocationWriter(const CsvLocationExportSettings &pluginSettings) noexcept;
    SkyDollyCsvLocationWriter(const SkyDollyCsvLocationWriter &rhs) = delete;
    SkyDollyCsvLocationWriter(SkyDollyCsvLocationWriter &&rhs) = delete;
    SkyDollyCsvLocationWriter &operator=(const SkyDollyCsvLocationWriter &rhs) = delete;
    SkyDollyCsvLocationWriter &operator=(SkyDollyCsvLocationWriter &&rhs) = delete;
    ~SkyDollyCsvLocationWriter() override;

    bool write(const std::vector<Location> &locations, QIODevice &ioDevice) noexcept override;

private:
    const std::unique_ptr<SkyDollyCsvLocationWriterPrivate> d;
};

#endif // SKYDOLLYCSVLOCATIONWRITER_H
