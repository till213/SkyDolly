/**
 * Sky Dolly - The Black Sheep for Your Flight Recordings
 *
 * Copyright (c) 2020 - 2024 Oliver Knoll
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

/*!
 * GPX format element names.
 *
 * In analogy to the KML naming conventions we use the same (de-)capitalisation for constant names
 * as given by the actual GPX element names. For GPX that essentially means all lower-case.
 */
namespace Gpx
{
    inline const QString gpx = "gpx";

    // Metadata
    inline const QString metadata = "metadata";
    inline const QString name = "name";
    inline const QString desc = "desc";
    inline const QString author = "author";

    // Waypoints
    inline const QString wpt = "wpt";

    // Routes
    inline const QString rte = "rte";
    inline const QString rtept = "rtept";

    // Tracks
    inline const QString trk = "trk";
    inline const QString trkseg = "trkseg";
    inline const QString trkpt = "trkpt";
    inline const QString ele = "ele";
    inline const QString time = "time";
    inline const QString lat = "lat";
    inline const QString lon = "lon";
}

#endif // GPX_H
