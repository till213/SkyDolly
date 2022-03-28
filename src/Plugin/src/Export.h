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
#ifndef EXPORT_H
#define EXPORT_H

#include <QStringView>
#include <QString>

#include "PluginLib.h"

/*!
 * Common export functionality.
 */
class PLUGIN_API Export
{
public:

    /*!
     * Returns a file path with the suggested file name having the given \c suffix.
     *
     * \param suffix
     *        the desired file suffix (e.g. \em kml or \em csv)
     * \return the file path having a suggested file name based on the current flight
     */
    static QString suggestFilePath(QStringView suffix) noexcept;

    /*!
     * Formats the GNSS \c coordinate (latitude or longitude) with the appropriate decimal point precision.
     *
     * \param coordinate
     *        the coordinate to be formatted as QString
     * \return the text representation of \c coordinate
     */
    static QString formatCoordinate(double coordinate) noexcept;

    /*!
     * Formats the general \c number (e.g. altitude or heading) with the appropriate decimal point precision.
     *
     * Note that in case the \c number should be displayed as text to a user, according to local settings, then
     * Unit::formatNumber should be used instead.
     *
     * \param number
     *        the number to be formatted as QString
     * \return the text representation of \c number
     */
    static QString formatNumber(double number) noexcept;
};

#endif // EXPORT_H
