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
#ifndef LITTLENAVMAPCSVLOCATIONWRITER_H
#define LITTLENAVMAPCSVLOCATIONWRITER_H

#include <memory>
#include <vector>
#include <cstdint>

#include <QString>
#include <QStringView>

class QIODevice;

#include "CsvLocationWriterIntf.h"

struct Location;
class CsvLocationExportSettings;
struct LittleNavmapCsvLocationWriterPrivate;

class LittleNavmapCsvLocationWriter : public CsvLocationWriterIntf
{
public:
    LittleNavmapCsvLocationWriter(const CsvLocationExportSettings &pluginSettings) noexcept;
    LittleNavmapCsvLocationWriter(const LittleNavmapCsvLocationWriter &rhs) = delete;
    LittleNavmapCsvLocationWriter(LittleNavmapCsvLocationWriter &&rhs) = delete;
    LittleNavmapCsvLocationWriter &operator=(const LittleNavmapCsvLocationWriter &rhs) = delete;
    LittleNavmapCsvLocationWriter &operator=(LittleNavmapCsvLocationWriter &&rhs) = delete;
    ~LittleNavmapCsvLocationWriter() override;

    bool write(const std::vector<Location> &locations, QIODevice &ioDevice) noexcept override;

private:
    const std::unique_ptr<LittleNavmapCsvLocationWriterPrivate> d;

    inline QString mapCategorySymIdToType(const QString &categorySymId) const noexcept;
};

#endif // LITTLENAVMAPCSVLOCATIONWRITER_H
