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
#ifndef COLOR_H
#define COLOR_H

#include <vector>

#include <QColor>

#include "KernelLib.h"

class KERNEL_API Color
{
public:
    static std::vector<QRgb> createColorRamp(QColor startColor, QColor endColor, int nofTotalColors) noexcept;
    static std::vector<QRgb> createColorRamp(QRgb start, QRgb end, int nofTotalColors) noexcept;

    /*!
     * Converts the \c color from format AARRGGBB to the KML format AABBGGRR.
     *
     * \param color
     *        the color in format AARRGGBB to be converted
     * \return the converted color in format AABBGGRR
     */
    inline static QRgb convertRgbToKml(QRgb color) {
        const QRgb alpha = qAlpha(color);
        const QRgb red = qRed(color);
        const QRgb green = qGreen(color);
        const QRgb blue = qBlue(color);
        return alpha << 24 | blue << 16 | green << 8 | red;
    }
};

#endif // COLOR_H
