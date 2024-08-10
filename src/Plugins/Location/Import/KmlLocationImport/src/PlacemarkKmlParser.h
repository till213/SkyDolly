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
#ifndef PLACEMARKKMLPARSER_H
#define PLACEMARKKMLPARSER_H

#include <vector>

#include <QDateTime>
#include <QString>

class QXmlStreamReader;

#include "AbstractKmlParser.h"

struct Location;
class KmlLocationImportSettings;
struct PlacemarkKmlParserPrivate;

class PlacemarkKmlParser final : public AbstractKmlParser
{
public:
    explicit PlacemarkKmlParser(const KmlLocationImportSettings &pluginSettings) noexcept;
    PlacemarkKmlParser(const PlacemarkKmlParser &rhs) = delete;
    PlacemarkKmlParser(PlacemarkKmlParser &&rhs) = delete;
    PlacemarkKmlParser &operator=(const PlacemarkKmlParser &rhs) = delete;
    PlacemarkKmlParser &operator=(PlacemarkKmlParser &&rhs) = delete;
    ~PlacemarkKmlParser() override;

    std::vector<Location> parse(QXmlStreamReader &xmlStreamReader) noexcept override;

protected:
    void parseFolderName(const QString &folderName) noexcept override;
    void parsePlacemark(std::vector<Location> &locations) noexcept override;

private:
    std::unique_ptr<PlacemarkKmlParserPrivate> d;

    void parsePoint(Location &location) noexcept;
    void guesstimateCurrentCategoryId(const QString &folderName) noexcept;
    static void unHtmlify(QString &description) noexcept;
    static QString extractIcao(const QString &description);
};

#endif // PLACEMARKKMLPARSER_H
