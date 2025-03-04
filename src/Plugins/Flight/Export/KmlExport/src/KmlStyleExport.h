/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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
#ifndef KMLSTYLEEXPORT_H
#define KMLSTYLEEXPORT_H

#include <memory>
#include <cstdint>

class QIODevice;

#include <Model/SimType.h>
#include "KmlExportSettings.h"

struct KmlStyleExportPrivate;

class KmlStyleExport
{
public:
    enum struct Icon: std::uint8_t {
        Airport,
        Flag
    };

    KmlStyleExport(const KmlExportSettings &pluginSettings) noexcept;
    KmlStyleExport(const KmlStyleExport &rhs) = delete;
    KmlStyleExport(KmlStyleExport &&rhs) = delete;
    KmlStyleExport &operator=(const KmlStyleExport &rhs) = delete;
    KmlStyleExport &operator=(KmlStyleExport &&rhs) = delete;
    ~KmlStyleExport();

    bool exportStyles(QIODevice &io) noexcept;
    QString getNextEngineTypeStyleMap(SimType::EngineType engineType) noexcept;

    static QString getStyleUrl(Icon icon) noexcept;

private:
    const std::unique_ptr<KmlStyleExportPrivate> d;

    void initialiseColorRamps() noexcept;
    bool exportHighlightLineStyle(QIODevice &io) const noexcept;
    bool exportNormalLineStyles(QIODevice &io) const noexcept;
    bool exportLineStyleMaps(QIODevice &io) const noexcept;
    bool exportPlacemarkStyles(QIODevice &io) const noexcept;

    static bool exportNormalLineStylesPerEngineType(SimType::EngineType engineType, std::vector<QRgb> &colorRamp, float lineWidth, QIODevice &io) noexcept;
};

#endif // KMLSTYLEEXPORT_H
