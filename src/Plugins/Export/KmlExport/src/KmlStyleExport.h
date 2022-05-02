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
#ifndef KMLSTYLEEXPORT_H
#define KMLSTYLEEXPORT_H

#include <memory>
#include <unordered_map>
#include <utility>

class QIODevice;

#include "../../../../Model/src/SimType.h"
#include "KmlExportSettings.h"

class KmlStyleExportPrivate;

class KmlStyleExport
{
public:
    enum struct Icon {
        Airport,
        Flag
    };

    KmlStyleExport(const KmlExportSettings &settings) noexcept;
    ~KmlStyleExport() noexcept;

    bool exportStyles(QIODevice &io) noexcept;
    QString getNextEngineTypeStyleMap(SimType::EngineType engineType) noexcept;

    static QString getStyleUrl(Icon icon) noexcept;

private:
    std::unique_ptr<KmlStyleExportPrivate> d;

    void initialiseColorRamps() noexcept;
    bool exportHighlightLineStyle(QIODevice &io) const noexcept;
    bool exportNormalLineStyles(QIODevice &io) const noexcept;
    bool exportLineStyleMaps(QIODevice &io) const noexcept;
    bool exportPlacemarkStyles(QIODevice &io) const noexcept;

    static bool exportNormalLineStylesPerEngineType(SimType::EngineType engineType, std::vector<QRgb> &colorRamp, float lineWidth, QIODevice &io) noexcept;
};

#endif // KMLSTYLEEXPORT_H