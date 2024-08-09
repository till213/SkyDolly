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
#ifndef KMLPLACEMARKPARSER_H
#define KMLPLACEMARKPARSER_H

#include <memory>
#include <vector>

#include <QDateTime>
#include <QString>

class QXmlStreamReader;

#include "KmlParserIntf.h"

struct Location;
struct KmlPlacemarkParserPrivate;

class KmlPlacemarkParser : public KmlParserIntf
{
public:
    KmlPlacemarkParser() noexcept;
    KmlPlacemarkParser(const KmlPlacemarkParser &rhs) = delete;
    KmlPlacemarkParser(KmlPlacemarkParser &&rhs) = delete;
    KmlPlacemarkParser &operator=(const KmlPlacemarkParser &rhs) = delete;
    KmlPlacemarkParser &operator=(KmlPlacemarkParser &&rhs) = delete;
    ~KmlPlacemarkParser() override;

    std::vector<Location> parse(QXmlStreamReader &xmlStreamReader) noexcept override;

private:
    const std::unique_ptr<KmlPlacemarkParserPrivate> d;

    virtual std::vector<Location> parseKML() noexcept;
    virtual void parseDocument(Location &location) noexcept;
    virtual void parseFolder(Location &location) noexcept;
    virtual void parsePlacemark(Location &location) noexcept;
    virtual void parseDocumentName(Location &location) noexcept;
};

#endif // KMLPLACEMARKPARSER_H
