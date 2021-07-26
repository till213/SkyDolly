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

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/SimType.h"
#include "KMLExportSettings.h"

// PUBLIC

KMLExportSettings::KMLExportSettings() noexcept
{
    restoreDefaults();
}

Settings::PluginSettings KMLExportSettings::getSettings() const noexcept
{
    Settings::PluginSettings settings;
    Settings::KeyValue keyValue;

    keyValue.first = "ResamplingPeriod";
    keyValue.second = Enum::toUnderlyingType(resamplingPeriod);
    settings.push_back(keyValue);

    keyValue.first = "ColorStyle";
    keyValue.second = Enum::toUnderlyingType(colorStyle);
    settings.push_back(keyValue);

    keyValue.first = "NofColorsPerRamp";
    keyValue.second = nofColorsPerRamp;
    settings.push_back(keyValue);

    keyValue.first = "LineWidth";
    keyValue.second = lineWidth;
    settings.push_back(keyValue);

    keyValue.first = "JetStartColor";
    keyValue.second = jetStartColor;
    settings.push_back(keyValue);

    keyValue.first = "JetEndColor";
    keyValue.second = jetEndColor;
    settings.push_back(keyValue);

    keyValue.first = "TurbopropStartColor";
    keyValue.second = turbopropStartColor;
    settings.push_back(keyValue);

    keyValue.first = "TurbopropEndColor";
    keyValue.second = turbopropEndColor;
    settings.push_back(keyValue);

    keyValue.first = "PistonStartColor";
    keyValue.second = pistonStartColor;
    settings.push_back(keyValue);

    keyValue.first = "PistonEndColor";
    keyValue.second = pistonEndColor;
    settings.push_back(keyValue);

    keyValue.first = "AllStartColor";
    keyValue.second = allStartColor;
    settings.push_back(keyValue);

    keyValue.first = "AllEndColor";
    keyValue.second = allEndColor;
    settings.push_back(keyValue);

    return settings;
}

Settings::KeysWithDefaults KMLExportSettings::getKeys() const noexcept
{
    Settings::KeysWithDefaults keys;

    // TODO IMPLEMENT ME (other values)
    Settings::KeyValue keyValue;

    keyValue.first = "ResamplingPeriod";
    keyValue.second = Enum::toUnderlyingType(KMLExportSettings::DefaultResamplingPeriod);
    keys.push_back(keyValue);

    keyValue.first = "ColorStyle";
    keyValue.second = Enum::toUnderlyingType(KMLExportSettings::DefaultColorStyle);
    keys.push_back(keyValue);

    keyValue.first = "NofColorsPerRamp";
    keyValue.second = DefaultNofColorsPerRamp;
    keys.push_back(keyValue);

    keyValue.first = "LineWidth";
    keyValue.second = DefaultLineWidth;
    keys.push_back(keyValue);

    keyValue.first = "JetStartColor";
    keyValue.second = QColor(DefaultJetStartRgba);
    keys.push_back(keyValue);

    keyValue.first = "JetEndColor";
    keyValue.second = QColor(DefaultJetEndColor);
    keys.push_back(keyValue);

    keyValue.first = "TurbopropStartColor";
    keyValue.second = QColor(DefaultTurbopropStartRgba);
    keys.push_back(keyValue);

    keyValue.first = "TurbopropEndColor";
    keyValue.second = QColor(DefaultTurbopropEndRgba);
    keys.push_back(keyValue);

    keyValue.first = "PistonStartColor";
    keyValue.second = QColor(DefaultPistonStartRgba);
    keys.push_back(keyValue);

    keyValue.first = "PistonEndColor";
    keyValue.second = QColor(DefaultPistonEndRgba);
    keys.push_back(keyValue);

    keyValue.first = "AllStartColor";
    keyValue.second = QColor(DefaultAllStartRgba);
    keys.push_back(keyValue);

    keyValue.first = "AllEndColor";
    keyValue.second = QColor(DefaultAllEndRgba);
    keys.push_back(keyValue);

    return keys;
}

void KMLExportSettings::setSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    bool ok;
    int enumeration = valuesByKey["ResamplingPeriod"].toInt(&ok);
    if (ok) {
        resamplingPeriod = static_cast<KMLExportSettings::ResamplingPeriod >(enumeration);
    } else {
        resamplingPeriod = DefaultResamplingPeriod;
    }

    enumeration = valuesByKey["ColorStyle"].toInt(&ok);
    if (ok) {
        colorStyle = static_cast<KMLExportSettings::ColorStyle >(enumeration);
    } else {
        colorStyle = DefaultColorStyle;
    }

    nofColorsPerRamp = valuesByKey["NofColorsPerRamp"].toInt(&ok);
    if (!ok) {
        nofColorsPerRamp = DefaultNofColorsPerRamp;
    }

    lineWidth = valuesByKey["LineWidth"].toFloat(&ok);
    if (!ok) {
        lineWidth = DefaultLineWidth;
    }

    jetStartColor = valuesByKey["JetStartColor"].value<QColor>();
    jetEndColor = valuesByKey["JetEndColor"].value<QColor>();
    turbopropStartColor = valuesByKey["TurbopropStartColor"].value<QColor>();
    turbopropEndColor = valuesByKey["TurbopropEndColor"].value<QColor>();
    pistonStartColor = valuesByKey["PistonStartColor"].value<QColor>();
    pistonEndColor = valuesByKey["PistonEndColor"].value<QColor>();
    allStartColor = valuesByKey["AllStartColor"].value<QColor>();
    allEndColor = valuesByKey["AllEndColor"].value<QColor>();
}

inline void KMLExportSettings::restoreDefaults() noexcept
{
    resamplingPeriod = DefaultResamplingPeriod;
    colorStyle = DefaultColorStyle;
    nofColorsPerRamp = DefaultNofColorsPerRamp;
    lineWidth = DefaultLineWidth;
    jetStartColor = DefaultJetStartRgba;
    jetEndColor = DefaultJetEndColor;
    turbopropStartColor = DefaultTurbopropStartRgba;
    turbopropEndColor = DefaultTurbopropEndRgba;
    pistonStartColor = DefaultPistonStartRgba;
    pistonEndColor = DefaultPistonEndRgba;
    allStartColor = DefaultAllStartRgba;
    allEndColor = DefaultAllEndRgba;
}
