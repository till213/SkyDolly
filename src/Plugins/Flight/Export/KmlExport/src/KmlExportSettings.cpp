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

#include <memory>

#include <QColor>

#include <Kernel/Enum.h>
#include <Kernel/Settings.h>
#include <PluginManager/FlightExportPluginBaseSettings.h>
#include "KmlExportSettings.h"

namespace
{
    // Keys
    constexpr const char *ColorStyleKey {"ColorStyle"};
    constexpr const char *NofColorsPerRampKey {"NofColorsPerRamp"};
    constexpr const char *LineWidthKey {"LineWidth"};
    constexpr const char *JetStartColorKey {"JetStartColor"};
    constexpr const char *JetEndColorKey {"JetEndColor"};
    constexpr const char *TurbopropStartColorKey {"TurbopropStartColor"};
    constexpr const char *TurbopropEndColorKey {"TurbopropEndColor"};
    constexpr const char *PistonStartColorKey {"PistonStartColor"};
    constexpr const char *PistonEndColorKey {"PistonEndColor"};
    constexpr const char *AllStartColorKey {"AllStartColor"};
    constexpr const char *AllEndColorKey {"AllEndColor"};

    // Defaults
    constexpr KmlExportSettings::ColorStyle DefaultColorStyle {KmlExportSettings::ColorStyle::OneColor};

    // in AARRGGBB format
    // https://designs.ai/colors/color-wheel
    // http://khroma.co/generator/
    // http://colormind.io/
    constexpr QRgb Opaque {0xff000000};

    // "Tetraedic" colors
    constexpr QRgb DefaultJetStartRgba {Opaque | 0xde7b51};
    constexpr QRgb DefaultJetEndRgba {Opaque | 0x6f3d28};
    constexpr QRgb DefaultTurbopropStartRgba {Opaque | 0x6ade4b};
    constexpr QRgb DefaultTurbopropEndRgba {Opaque | 0x356f25};
    constexpr QRgb DefaultPistonStartRgba {Opaque | 0x4bb3de};
    constexpr QRgb DefaultPistonEndRgba {Opaque | 0x255a6f};
    constexpr QRgb DefaultAllStartRgba {Opaque | 0xc561de};
    constexpr QRgb DefaultAllEndRgba {Opaque | 0x63316f};

    constexpr int DefaultNofColorsPerRamp {8};
    constexpr float DefaultLineWidth {3.0f};

}

struct KmlExportSettingsPrivate
{
    KmlExportSettingsPrivate()
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

    KmlExportSettings::ColorStyle colorStyle;
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

KmlExportSettings::KmlExportSettings() noexcept
    : FlightExportPluginBaseSettings(),
      d(std::make_unique<KmlExportSettingsPrivate>())
{}

KmlExportSettings::~KmlExportSettings() = default;

KmlExportSettings::ColorStyle KmlExportSettings::getColorStyle() const noexcept
{
    return d->colorStyle;
}

void KmlExportSettings::setColorStyle(ColorStyle colorStyle) noexcept
{
    if (d->colorStyle != colorStyle) {
        d->colorStyle = colorStyle;
        if (colorStyle== KmlExportSettings::ColorStyle::ColorRamp || colorStyle == KmlExportSettings::ColorStyle::ColorRampPerEngineType) {
            d->nofColorsPerRamp = ::DefaultNofColorsPerRamp;
        } else {
            d->nofColorsPerRamp = 1;
        }
        emit changed();
    }
}

int KmlExportSettings::getNofColorsPerRamp() const noexcept
{
    return d->nofColorsPerRamp;
}

void KmlExportSettings::setNofColorsPerRamp(int nofColors) noexcept
{
    if (d->nofColorsPerRamp != nofColors) {
        d->nofColorsPerRamp = nofColors;
        emit changed();
    }
}

float KmlExportSettings::getLineWidth() const noexcept
{
    return d->lineWidth;
}

void KmlExportSettings::setLineWidth(float lineWidth) noexcept
{
    if (d->lineWidth != lineWidth) {
        d->lineWidth = lineWidth;
        emit changed();
    }
}

QColor KmlExportSettings::getJetStartColor() const noexcept
{
    return d->jetStartColor;
}

void KmlExportSettings::setJetStartColor(const QColor &color) noexcept
{
    if (d->jetStartColor != color) {
        d->jetStartColor = color;
        emit changed();
    }
}

QColor KmlExportSettings::getJetEndColor() const noexcept
{
    return d->jetEndColor;
}

void KmlExportSettings::setJetEndColor(const QColor &color) noexcept
{
    if (d->jetEndColor != color) {
        d->jetEndColor = color;
        emit changed();
    }
}

QColor KmlExportSettings::getTurbopropStartColor() const noexcept
{
    return d->turbopropStartColor;
}

void KmlExportSettings::setTurbopropStartColor(const QColor &color) noexcept
{
    if (d->turbopropStartColor != color) {
        d->turbopropStartColor = color;
        emit changed();
    }
}

QColor KmlExportSettings::getTurbopropEndColor() const noexcept
{
    return d->turbopropEndColor;
}

void KmlExportSettings::setTurbopropEndColor(const QColor &color) noexcept
{
    if (d->turbopropEndColor != color) {
        d->turbopropEndColor = color;
        emit changed();
    }
}

QColor KmlExportSettings::getPistonStartColor() const noexcept
{
    return d->pistonStartColor;
}

void KmlExportSettings::setPistonStartColor(const QColor &color) noexcept
{
    if (d->pistonStartColor != color) {
        d->pistonStartColor = color;
        emit changed();
    }
}

QColor KmlExportSettings::getPistonEndColor() const noexcept
{
    return d->pistonEndColor;
}

void KmlExportSettings::setPistonEndColor(const QColor &color) noexcept
{
    if (d->pistonEndColor != color) {
        d->pistonEndColor = color;
        emit changed();
    }
}

QColor KmlExportSettings::getAllStartColor() const noexcept
{
    return d->allStartColor;
}

void KmlExportSettings::setAllStartColor(const QColor &color) noexcept
{
    if (d->allStartColor != color) {
        d->allStartColor = color;
        emit changed();
    }
}

QColor KmlExportSettings::getAllEndColor() const noexcept
{
    return d->allEndColor;
}

void KmlExportSettings::setAllEndColor(const QColor &color) noexcept
{
    if (d->allEndColor != color) {
        d->allEndColor = color;
        emit changed();
    }
}

bool KmlExportSettings::isResamplingSupported() const noexcept
{
    return true;
}

bool KmlExportSettings::isFormationExportSupported(FormationExport formationExport) const noexcept
{
    bool supported {false};
    switch (formationExport) {
    case FormationExport::AllAircraftOneFile:
        supported = true;
        break;
    case FormationExport::AllAircraftSeparateFiles:
        supported = true;
        break;
    case FormationExport::UserAircraftOnly:
        supported = true;
        break;
    }
    return supported;
};

// PROTECTED

void KmlExportSettings::addSettingsExtn(Settings::KeyValues &keyValues) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ColorStyleKey;
    keyValue.second = Enum::underly(d->colorStyle);
    keyValues.push_back(keyValue);

    keyValue.first = ::NofColorsPerRampKey;
    keyValue.second = d->nofColorsPerRamp;
    keyValues.push_back(keyValue);

    keyValue.first = ::LineWidthKey;
    keyValue.second = d->lineWidth;
    keyValues.push_back(keyValue);

    keyValue.first = ::JetStartColorKey;
    keyValue.second = d->jetStartColor;
    keyValues.push_back(keyValue);

    keyValue.first = ::JetEndColorKey;
    keyValue.second = d->jetEndColor;
    keyValues.push_back(keyValue);

    keyValue.first = ::TurbopropStartColorKey;
    keyValue.second = d->turbopropStartColor;
    keyValues.push_back(keyValue);

    keyValue.first = ::TurbopropEndColorKey;
    keyValue.second = d->turbopropEndColor;
    keyValues.push_back(keyValue);

    keyValue.first = ::PistonStartColorKey;
    keyValue.second = d->pistonStartColor;
    keyValues.push_back(keyValue);

    keyValue.first = ::PistonEndColorKey;
    keyValue.second = d->pistonEndColor;
    keyValues.push_back(keyValue);

    keyValue.first = ::AllStartColorKey;
    keyValue.second = d->allStartColor;
    keyValues.push_back(keyValue);

    keyValue.first = ::AllEndColorKey;
    keyValue.second = d->allEndColor;
    keyValues.push_back(keyValue);
}

void KmlExportSettings::addKeysWithDefaultsExtn(Settings::KeysWithDefaults &keysWithDefaults) const noexcept
{
    Settings::KeyValue keyValue;

    keyValue.first = ::ColorStyleKey;
    keyValue.second = Enum::underly(::DefaultColorStyle);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::NofColorsPerRampKey;
    keyValue.second = ::DefaultNofColorsPerRamp;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::LineWidthKey;
    keyValue.second = ::DefaultLineWidth;
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::JetStartColorKey;
    keyValue.second = QColor(::DefaultJetStartRgba);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::JetEndColorKey;
    keyValue.second = QColor(::DefaultJetEndRgba);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::TurbopropStartColorKey;
    keyValue.second = QColor(::DefaultTurbopropStartRgba);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::TurbopropEndColorKey;
    keyValue.second = QColor(::DefaultTurbopropEndRgba);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::PistonStartColorKey;
    keyValue.second = QColor(::DefaultPistonStartRgba);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::PistonEndColorKey;
    keyValue.second = QColor(::DefaultPistonEndRgba);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::AllStartColorKey;
    keyValue.second = QColor(::DefaultAllStartRgba);
    keysWithDefaults.push_back(keyValue);

    keyValue.first = ::AllEndColorKey;
    keyValue.second = QColor(::DefaultAllEndRgba);
    keysWithDefaults.push_back(keyValue);
}

void KmlExportSettings::restoreSettingsExtn(const Settings::ValuesByKey &valuesByKey) noexcept
{
    bool ok {true};
    const int enumeration = valuesByKey.at(::ColorStyleKey).toInt(&ok);
    if (ok) {
        d->colorStyle = static_cast<KmlExportSettings::ColorStyle >(enumeration);
    } else {
        d->colorStyle = ::DefaultColorStyle;
    }

    d->nofColorsPerRamp = valuesByKey.at(::NofColorsPerRampKey).toInt(&ok);
    if (!ok) {
        d->nofColorsPerRamp = ::DefaultNofColorsPerRamp;
    }

    d->lineWidth = valuesByKey.at(::LineWidthKey).toFloat(&ok);
    if (!ok) {
        d->lineWidth = ::DefaultLineWidth;
    }

    d->jetStartColor = valuesByKey.at(::JetStartColorKey).value<QColor>();
    d->jetEndColor = valuesByKey.at(::JetEndColorKey).value<QColor>();
    d->turbopropStartColor = valuesByKey.at(::TurbopropStartColorKey).value<QColor>();
    d->turbopropEndColor = valuesByKey.at(::TurbopropEndColorKey).value<QColor>();
    d->pistonStartColor = valuesByKey.at(::PistonStartColorKey).value<QColor>();
    d->pistonEndColor = valuesByKey.at(::PistonEndColorKey).value<QColor>();
    d->allStartColor = valuesByKey.at(::AllStartColorKey).value<QColor>();
    d->allEndColor = valuesByKey.at(::AllEndColorKey).value<QColor>();
}

void KmlExportSettings::restoreDefaultsExtn() noexcept
{
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
}
