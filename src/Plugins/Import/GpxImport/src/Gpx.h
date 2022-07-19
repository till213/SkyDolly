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
#ifndef GPX_H
#define GPX_H

#include <QString>
#include <QStringLiteral>

/*!
 * GPX format element names.
 *
 * In analogy to the KML naming conventions we use the same (de-)capitalisation for constant names
 * as given by the actual GPX element names. For GPX that essentially means all lower-case.
 */
namespace Gpx
{
    inline const QString gpx = QStringLiteral("gpx");

    // Metadata
    inline const QString metadata = QStringLiteral("metadata");
    inline const QString name = QStringLiteral("name");
    inline const QString desc = QStringLiteral("desc");
    inline const QString author = QStringLiteral("author");

    // Waypoints
    inline const QString wpt = QStringLiteral("wpt");

    // Routes
    inline const QString rte = QStringLiteral("rte");
    inline const QString rtept = QStringLiteral("rtept");


    // Tracks
    inline const QString trk = QStringLiteral("trk");
    inline const QString trkseg = QStringLiteral("trkseg");
    inline const QString trkpt = QStringLiteral("trkpt");
    inline const QString ele = QStringLiteral("ele");
    inline const QString time = QStringLiteral("time");
    inline const QString lat = QStringLiteral("lat");
    inline const QString lon = QStringLiteral("lon");
}

#endif // GPX_H
