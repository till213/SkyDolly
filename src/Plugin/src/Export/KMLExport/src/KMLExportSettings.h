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
#ifndef KMLEXPORTSETTINGS_H
#define KMLEXPORTSETTINGS_H

#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/SimType.h"
#include <QColor>

struct KMLExportSettings
{
    /*!
     * Resampling period [millisecons]
     */
    enum struct ResamplingPeriod {
        Original = 0,
        TenHz = 100,
        FiveHz = 200,
        TwoHz = 500,
        OneHz = 1000,
        AFifthHz = 5000,
        ATenthHz = 10000
    };
    enum struct ColorStyle {
        OneColor,
        OneColorPerEngineType,
        ColorRamp,
        ColorRampPerEngineType
    };

    KMLExportSettings() noexcept;

    ResamplingPeriod resamplingPeriod;
    QColor jetStartColor;
    QColor jetEndColor;
    QColor turbopropStartColor;
    QColor turbopropEndColor;
    QColor pistonStartColor;
    QColor pistonEndColor;
    QColor allStartColor;
    QColor allEndColor;
    ColorStyle colorStyle;
    int nofColorsPerRamp;
    float lineWidth;

    Settings::PluginSettings getSettings() const noexcept;
    Settings::KeysWithDefaults getKeys() const noexcept;
    void setSettings(Settings::ValuesByKey) noexcept;
    void restoreDefaults() noexcept;

    static constexpr ResamplingPeriod DefaultResamplingPeriod = ResamplingPeriod::OneHz;
    static constexpr ColorStyle DefaultColorStyle = ColorStyle::OneColor;

    // in AARRGGBB format
    // https://designs.ai/colors/color-wheel
    // http://khroma.co/generator/
    // http://colormind.io/
    static constexpr QRgb Opaque = 0xff000000;

    // "Tetraedic" colors
    static constexpr QRgb DefaultJetStartRgba = Opaque | 0xde7b51;
    static constexpr QRgb DefaultJetEndColor = Opaque | 0x6f3d28;
    static constexpr QRgb DefaultTurbopropStartRgba = Opaque | 0x6ade4b;
    static constexpr QRgb DefaultTurbopropEndRgba = Opaque | 0x356f25;
    static constexpr QRgb DefaultPistonStartRgba = Opaque | 0x4bb3de;
    static constexpr QRgb DefaultPistonEndRgba = Opaque | 0x255a6f;
    static constexpr QRgb DefaultAllStartRgba = Opaque | 0xc561de;
    static constexpr QRgb DefaultAllEndRgba = Opaque | 0x63316f;

    static constexpr int DefaultNofColorsPerRamp = 8;
    static constexpr float DefaultLineWidth = 3.0f;
};

#endif // KMLEXPORTSETTINGS_H
