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
#ifndef KML_H
#define KML_H

#include <QString>
#include <QStringLiteral>

/*!
 * KML format element names.
 *
 * From https://developers.google.com/kml/documentation/kml_element_hierarchy:
 * "In KML, simple element names begin with a lowercase letter. Simple elements can contain a value, but they do not contain other elements.
 * Complex element names being with an uppercase letter. Complex elements can contain other elements (referred to as their children)."
 *
 * We adhere to this naming convention for the constants defined in this namespace.
 */
namespace KML
{
    inline const QString Document = QStringLiteral("Document");
    inline const QString Folder = QStringLiteral("Folder");
    inline const QString Placemark = QStringLiteral("Placemark");
    inline const QString Point = QStringLiteral("Point");
    inline const QString Track = QStringLiteral("Track");
    inline const QString TimeStamp = QStringLiteral("TimeStamp");

    inline const QString name = QStringLiteral("name");
    inline const QString when = QStringLiteral("when");
    inline const QString coord = QStringLiteral("coord");
    inline const QString coordinates = QStringLiteral("coordinates");
    inline const QString description = QStringLiteral("description");
}

#endif // KML_H
