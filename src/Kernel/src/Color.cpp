/**
 * Sky Dolly - The black sheep for your flight recordings
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
#include <vector>

#include <QColor>

#include "Color.h"

// PUBLIC

std::vector<QRgb> Color::createColorRamp(QColor startColor, QColor endColor, int nofTotalColors) noexcept
{
    return createColorRamp(startColor.rgba(), endColor.rgba(), nofTotalColors);
}

std::vector<QRgb> Color::createColorRamp(QRgb startColor, QRgb endColor, int nofTotalColors) noexcept
{
    std::vector<QRgb> colorRamp;

    if (nofTotalColors > 2) {

        const double startAlpha = qAlpha(startColor);
        const double startRed = qRed(startColor);
        const double startGreen = qGreen(startColor);
        const double startBlue = qBlue(startColor);

        const double endAlpha = qAlpha(endColor);
        const double endRed = qRed(endColor);
        const double endGreen = qGreen(endColor);
        const double endBlue = qBlue(endColor);

        // Distance values may be negative
        const double nofIntervals = nofTotalColors - 1;
        const double deltaAlpha = (endAlpha - startAlpha) / static_cast<double>(nofIntervals);
        const double deltaRed = (endRed - startRed) / static_cast<double>(nofIntervals);
        const double deltaGreen = (endGreen - startGreen) / static_cast<double>(nofIntervals);
        const double deltaBlue = (endBlue - startBlue) / static_cast<double>(nofIntervals);

        // First color
        colorRamp.push_back(startColor);
        // Color ramp
        for (int i = 1; i < nofTotalColors - 1; ++i) {
            const double newAlpha = startAlpha + static_cast<double>(i) * deltaAlpha;
            const double newRed = startRed + static_cast<double>(i) * deltaRed;
            const double newGreen = startGreen + static_cast<double>(i) * deltaGreen;
            const double newBlue = startBlue + static_cast<double>(i) * deltaBlue;

            const QRgb newRgb = qRound(newAlpha) << 24 | qRound(newRed) << 16 | qRound(newGreen) << 8 | qRound(newBlue);
            colorRamp.push_back(newRgb);
        }
        // Last color
        colorRamp.push_back(endColor);

    } else if (nofTotalColors == 2) {
        colorRamp.push_back(startColor);
        colorRamp.push_back(endColor);
    } else if (nofTotalColors == 1) {
        colorRamp.push_back(startColor);
    }

    return colorRamp;
}
