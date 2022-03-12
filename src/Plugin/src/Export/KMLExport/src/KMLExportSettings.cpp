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

#include "../../../../../Kernel/src/Enum.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Model/src/SimType.h"
#include "../../Plugin/src/ExportPluginBaseSettings.h"
#include "KMLExportSettings.h"

namespace
{
    // Keys
    constexpr char ColorStyleKey[] = "ColorStyle";
    constexpr char NofColorsPerRampKey[] = "NofColorsPerRamp";
    constexpr char LineWidthKey[] = "LineWidth";
    constexpr char JetStartColorKey[] = "JetStartColor";
    constexpr char JetEndColorKey[] = "JetEndColor";
    constexpr char TurbopropStartColorKey[] = "TurbopropStartColor";
    constexpr char TurbopropEndColorKey[] = "TurbopropEndColor";
    constexpr char PistonStartColorKey[] = "PistonStartColor";
    constexpr char PistonEndColorKey[] = "PistonEndColor";
    constexpr char AllStartColorKey[] = "AllStartColor";
    constexpr char AllEndColorKey[] = "AllEndColor";

    // Defaults
    constexpr KMLExportSettings::ColorStyle DefaultColorStyle = KMLExportSettings::ColorStyle::OneColor;

    // in AARRGGBB format
    // https://designs.ai/colors/color-wheel
    // http://khroma.co/generator/
    // http://colormind.io/
    constexpr QRgb Opaque = 0xff000000;

    // "Tetraedic" colors
    constexpr QRgb DefaultJetStartRgba = Opaque | 0xde7b51;
    constexpr QRgb DefaultJetEndRgba = Opaque | 0x6f3d28;
    constexpr QRgb DefaultTurbopropStartRgba = Opaque | 0x6ade4b;
    constexpr QRgb DefaultTurbopropEndRgba = Opaque | 0x356f25;
    constexpr QRgb DefaultPistonStartRgba = Opaque | 0x4bb3de;
    constexpr QRgb DefaultPistonEndRgba = Opaque | 0x255a6f;
    constexpr QRgb DefaultAllStartRgba = Opaque | 0xc561de;
    constexpr QRgb DefaultAllEndRgba = Opaque | 0x63316f;

    constexpr int DefaultNofColorsPerRamp = 8;
    constexpr float DefaultLineWidth = 3.0f;

}

class KMLExportSettingsPrivate
{
public:
    KMLExportSettingsPrivate()
        : colorStyle(::DefaultColorStyle),
          nofColorsPerRamp(::DefaultNofColorsPerRamp),
          lineWidth(::DefaultLineWidth),
          jetStartColor(::DefaultJetStartRgba),
          jetEndColor(::DefaultJetEndRgba),
          turbopropStartColor(::DefaultTurbopropStartRgba),
          turbopropEndColor(::DefaultTurbopropEndRgba),
          pistonStartColor(::DefaultPistonStartRgba),
          pistonEndColor(::DefaultPistonEndRgba),
          allStartColor(::DefaultAllStartRgba),
          allEndColor(::DefaultAllEndRgba)
    {}

    KMLExportSettings::ColorStyle colorStyle;
    int nofColorsPerRamp;
    float lineWidth;
    QColor jetStartColor;
    QColor jetEndColor;
    QColor turbopropStartColor;
    QColor turbopropEndColor;
    QColor pistonStartColor;
    QColor pistonEndColor;
    QColor allStartColor;
    QColor allEndColor;
};

// PUBLIC

KMLExportSettings::KMLExportSettings() noexcept
    : ExportPluginBaseSettings(),
      d(std::make_unique<KMLExportSettingsPrivate>())
{
#ifdef DEBUG
    qDebug("KMLExportSettings::KMLExportSettings: CREATED");
#endif
}

KMLExportSettings::~KMLExportSettings() noexcept
{
#ifdef DEBUG
    qDebug("KMLExportSettings::~KMLExportSettings: DELETED");
#endif
}

KMLExportSettings::ColorStyle KMLExportSettings::getColorStyle() const noexcept
{
    return d->colorStyle;
}

void KMLExportSettings::setColorStyle(ColorStyle colorStyle) noexcept
{
    if (d->colorStyle != colorStyle) {
        d->colorStyle = colorStyle;
        if (colorStyle== KMLExportSettings::ColorStyle::ColorRamp || colorStyle == KMLExportSettings::ColorStyle::ColorRampPerEngineType) {
            d->nofColorsPerRamp = ::DefaultNofColorsPerRamp;
        } else {
            d->nofColorsPerRamp = 1;
        }
        emit extendedSettingsChanged();
    }
}

int KMLExportSettings::getNofColorsPerRamp() const noexcept
{
    return d->nofColorsPerRamp;
}

void KMLExportSettings::setNofColorsPerRamp(int nofColors) noexcept
{
    if (d->nofColorsPerRamp != nofColors) {
        d->nofColorsPerRamp = nofColors;
        emit extendedSettingsChanged();
    }
}

float KMLExportSettings::getLineWidth() const noexcept
{
    return d->lineWidth;
}

void KMLExportSettings::setLineWidth(float lineWidth) noexcept
{
    if (d->lineWidth != lineWidth) {
        d->lineWidth = lineWidth;
        emit extendedSettingsChanged();
    }
}

QColor KMLExportSettings::getJetStartColor() const noexcept
{
    return d->jetStartColor;
}

void KMLExportSettings::setJetStartColor(QColor color) noexcept
{
    if (d->jetStartColor != color) {
        d->jetStartColor = color;
        emit extendedSettingsChanged();
    }
}

QColor KMLExportSettings::getJetEndColor() const noexcept
{
    return d->jetEndColor;
}

void KMLExportSettings::setJetEndColor(QColor color) noexcept
{
    if (d->jetEndColor != color) {
        d->jetEndColor = color;
        emit extendedSettingsChanged();
    }
}

QColor KMLExportSettings::getTurbopropStartColor() const noexcept
{
    return d->turbopropStartColor;
}

void KMLExportSettings::setTurbopropStartColor(QColor color) noexcept
{
    if (d->turbopropStartColor != color) {
        d->turbopropStartColor = color;
        emit extendedSettingsChanged();
    }
}

QColor KMLExportSettings::getTurbopropEndColor() const noexcept
{
    return d->turbopropEndColor;
}

void KMLExportSettings::setTurbopropEndColor(QColor color) noexcept
{
    if (d->turbopropEndColor != color) {
        d->turbopropEndColor = color;
        emit extendedSettingsChanged();
    }
}

QColor KMLExportSettings::getPistonStartColor() const noexcept
{
    return d->pistonStartColor;
}

void KMLExportSettings::setPistonStartColor(QColor color) noexcept
{
    if (d->pistonStartColor != color) {
        d->pistonStartColor = color;
        emit extendedSettingsChanged();
    }
}

QColor KMLExportSettings::getPistonEndColor() const noexcept
{
    return d->pistonEndColor;
}

void KMLExportSettings::setPistonEndColor(QColor color) noexcept
{
    if (d->pistonEndColor != color) {
        d->pistonEndColor = color;
        emit extendedSettingsChanged();
    }
}

QColor KMLExportSettings::getAllStartColor() const noexcept
{
    return d->allStartColor;
}

void KMLExportSettings::setAllStartColor(QColor color) noexcept
{
    if (d->allStartColor != color) {
        d->allStartColor = color;
        emit extendedSettingsChanged();
    }
}

QColor KMLExportSettings::getAllEndColor() const noexcept
{
    return d->allEndColor;
}

void KMLExportSettings::setAllEndColor(QColor color) noexcept
{
    if (d->allEndColor != color) {
        d->allEndColor = color;
        emit extendedSettingsChanged();
    }
}

void KMLExportSettings::addSettings(Settings::PluginSettings &settings) const noexcept
{
    ExportPluginBaseSettings::addKeysWithDefaults(settings);

    Settings::KeyValue keyValue;

    keyValue.first = ::ColorStyleKey;
    keyValue.second = Enum::toUnderlyingType(d->colorStyle);
    settings.push_back(keyValue);

    keyValue.first = ::NofColorsPerRampKey;
    keyValue.second = d->nofColorsPerRamp;
    settings.push_back(keyValue);

    keyValue.first = ::LineWidthKey;
    keyValue.second = d->lineWidth;
    settings.push_back(keyValue);

    keyValue.first = ::JetStartColorKey;
    keyValue.second = d->jetStartColor;
    settings.push_back(keyValue);

    keyValue.first = ::JetEndColorKey;
    keyValue.second = d->jetEndColor;
    settings.push_back(keyValue);

    keyValue.first = ::TurbopropStartColorKey;
    keyValue.second = d->turbopropStartColor;
    settings.push_back(keyValue);

    keyValue.first = ::TurbopropEndColorKey;
    keyValue.second = d->turbopropEndColor;
    settings.push_back(keyValue);

    keyValue.first = ::PistonStartColorKey;
    keyValue.second = d->pistonStartColor;
    settings.push_back(keyValue);

    keyValue.first = ::PistonEndColorKey;
    keyValue.second = d->pistonEndColor;
    settings.push_back(keyValue);

    keyValue.first = ::AllStartColorKey;
    keyValue.second = d->allStartColor;
    settings.push_back(keyValue);

    keyValue.first = ::AllEndColorKey;
    keyValue.second = d->allEndColor;
    settings.push_back(keyValue);
}

void KMLExportSettings::addKeysWithDefaults(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    ExportPluginBaseSettings::addKeysWithDefaults(keysWithDefaults);

    Settings::KeysWithDefaults keys;
    Settings::KeyValue keyValue;

    keyValue.first = ::ColorStyleKey;
    keyValue.second = Enum::toUnderlyingType(::DefaultColorStyle);
    keys.push_back(keyValue);

    keyValue.first = ::NofColorsPerRampKey;
    keyValue.second = ::DefaultNofColorsPerRamp;
    keys.push_back(keyValue);

    keyValue.first = ::LineWidthKey;
    keyValue.second = ::DefaultLineWidth;
    keys.push_back(keyValue);

    keyValue.first = ::JetStartColorKey;
    keyValue.second = QColor(::DefaultJetStartRgba);
    keys.push_back(keyValue);

    keyValue.first = ::JetEndColorKey;
    keyValue.second = QColor(::DefaultJetEndRgba);
    keys.push_back(keyValue);

    keyValue.first = ::TurbopropStartColorKey;
    keyValue.second = QColor(::DefaultTurbopropStartRgba);
    keys.push_back(keyValue);

    keyValue.first = ::TurbopropEndColorKey;
    keyValue.second = QColor(::DefaultTurbopropEndRgba);
    keys.push_back(keyValue);

    keyValue.first = ::PistonStartColorKey;
    keyValue.second = QColor(::DefaultPistonStartRgba);
    keys.push_back(keyValue);

    keyValue.first = ::PistonEndColorKey;
    keyValue.second = QColor(::DefaultPistonEndRgba);
    keys.push_back(keyValue);

    keyValue.first = ::AllStartColorKey;
    keyValue.second = QColor(::DefaultAllStartRgba);
    keys.push_back(keyValue);

    keyValue.first = ::AllEndColorKey;
    keyValue.second = QColor(::DefaultAllEndRgba);
    keys.push_back(keyValue);
}

void KMLExportSettings::restoreSettings(Settings::ValuesByKey valuesByKey) noexcept
{
    ExportPluginBaseSettings::restoreSettings(valuesByKey);

    bool ok;
    const int enumeration = valuesByKey[::ColorStyleKey].toInt(&ok);
    if (ok) {
        d->colorStyle = static_cast<KMLExportSettings::ColorStyle >(enumeration);
    } else {
        d->colorStyle = ::DefaultColorStyle;
    }

    d->nofColorsPerRamp = valuesByKey[::NofColorsPerRampKey].toInt(&ok);
    if (!ok) {
        d->nofColorsPerRamp = ::DefaultNofColorsPerRamp;
    }

    d->lineWidth = valuesByKey[::LineWidthKey].toFloat(&ok);
    if (!ok) {
        d->lineWidth = ::DefaultLineWidth;
    }

    d->jetStartColor = valuesByKey[::JetStartColorKey].value<QColor>();
    d->jetEndColor = valuesByKey[::JetEndColorKey].value<QColor>();
    d->turbopropStartColor = valuesByKey[::TurbopropStartColorKey].value<QColor>();
    d->turbopropEndColor = valuesByKey[::TurbopropEndColorKey].value<QColor>();
    d->pistonStartColor = valuesByKey[::PistonStartColorKey].value<QColor>();
    d->pistonEndColor = valuesByKey[::PistonEndColorKey].value<QColor>();
    d->allStartColor = valuesByKey[::AllStartColorKey].value<QColor>();
    d->allEndColor = valuesByKey[::AllEndColorKey].value<QColor>();

    emit extendedSettingsChanged();
}

void KMLExportSettings::restoreDefaults() noexcept
{
    ExportPluginBaseSettings::restoreDefaults();

    d->colorStyle = ::DefaultColorStyle;
    d->nofColorsPerRamp = ::DefaultNofColorsPerRamp;
    d->lineWidth = ::DefaultLineWidth;
    d->jetStartColor = ::DefaultJetStartRgba;
    d->jetEndColor = ::DefaultJetEndRgba;
    d->turbopropStartColor = ::DefaultTurbopropStartRgba;
    d->turbopropEndColor = ::DefaultTurbopropEndRgba;
    d->pistonStartColor = ::DefaultPistonStartRgba;
    d->pistonEndColor = ::DefaultPistonEndRgba;
    d->allStartColor = ::DefaultAllStartRgba;
    d->allEndColor = ::DefaultAllEndRgba;

    emit extendedSettingsChanged();
}
