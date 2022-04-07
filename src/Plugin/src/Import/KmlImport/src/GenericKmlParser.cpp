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
#include <memory>

#include <QString>
#include <QXmlStreamReader>

#include "Kml.h"
#include "AbstractKmlTrackParser.h"
#include "GenericKmlParser.h"

class GenericKmlParserPrivate
{
public:
    GenericKmlParserPrivate(QXmlStreamReader &xmlStreamReader) noexcept
        : xml(xmlStreamReader)
    {}

    QXmlStreamReader &xml;
};

// PUBLIC

GenericKmlParser::GenericKmlParser(Flight &flight, QXmlStreamReader &xmlStreamReader) noexcept
    : AbstractKmlTrackParser(flight, xmlStreamReader),
      d(std::make_unique<GenericKmlParserPrivate>(xmlStreamReader))
{
#ifdef DEBUG
    qDebug("GenericKmlParser::~GenericKmlParser: CREATED");
#endif
}

GenericKmlParser::~GenericKmlParser() noexcept
{
#ifdef DEBUG
    qDebug("GenericKmlParser::~GenericKmlParser: DELETED");
#endif
}

// Generic KML files (are expected to) have at least one "gx:Track"
void GenericKmlParser::parse() noexcept
{
    parseKML();
}

QString GenericKmlParser::getFlightNumber() const noexcept
{
    return QString();
}
