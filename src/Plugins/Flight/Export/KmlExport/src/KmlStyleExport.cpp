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
#include <vector>

#include <QIODevice>
#include <QString>
#include <QStringBuilder>
#include <QColor>

#include <Kernel/Color.h>
#include <Model/SimType.h>
#include "KmlExportSettings.h"
#include "KmlStyleExport.h"

namespace
{
    constexpr QRgb LineHighlightColor = 0xffffff00;
    constexpr QRgb PolygonHighlightColor = 0xcc7ed5c9;
    constexpr QRgb PolygonColor = 0x337ed5c9;

    constexpr const char *JetStyleId {"s_jet_style"};
    constexpr const char *TurbopropStyleId {"s_turbo_prop_style"};
    constexpr const char *PistonStyleId {"s_piston_style"};
    constexpr const char *AllStyleId {"s_all_style"};

    constexpr const char *JetStyleMapId {"sm_jet_style"};
    constexpr const char *TurbopropStyleMapId {"sm_turbo_prop_style"};
    constexpr const char *PistonStyleMapId {"sm_piston_style"};
    constexpr const char *AllStyleMapId {"sm_all_style"};
}

struct KmlStyleExportPrivate
{
    KmlStyleExportPrivate(const KmlExportSettings &pluginSettings) noexcept
        : pluginSettings(pluginSettings)
    {}

    const KmlExportSettings &pluginSettings;
    std::vector<QRgb> jetColorRamp;
    std::vector<QRgb> turbopropColorRamp;
    std::vector<QRgb> pistonColorRamp;
    std::vector<QRgb> allColorRamp;
    // Index into color ramp, modulo its size
    int jetColorRampIndex {0};
    int turbopropColorRampIndex {0};
    int pistonColorRampIndex {0};
    int allColorRampIndex {0};
};

// PUBLIC

KmlStyleExport::KmlStyleExport(const KmlExportSettings &pluginSettings) noexcept
    : d(std::make_unique<KmlStyleExportPrivate>(pluginSettings))
{}

KmlStyleExport::~KmlStyleExport() = default;

bool KmlStyleExport::exportStyles(QIODevice &io) noexcept
{
    initialiseColorRamps();
    bool ok = exportHighlightLineStyle(io);
    if (ok) {
        ok = exportNormalLineStyles(io);
    }
    if (ok) {
        ok = exportLineStyleMaps(io);
    }
    if (ok) {
        ok = exportPlacemarkStyles(io);
    }
    return ok;
}

QString KmlStyleExport::getNextEngineTypeStyleMap(SimType::EngineType engineType) noexcept
{
    QString styleMapId;
    std::size_t nofColors {0};

    if (d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::OneColorPerEngineType || d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::ColorRampPerEngineType) {
        switch (engineType) {
        case SimType::EngineType::Jet:
            nofColors = d->jetColorRamp.size();
            if (nofColors > 0) {
                styleMapId = QString::fromLatin1(::JetStyleMapId) % "_" % QString::number(d->jetColorRampIndex % nofColors);
                ++d->jetColorRampIndex;
            }
            break;
        case SimType::EngineType::Turboprop:
            nofColors = d->turbopropColorRamp.size();
            if (nofColors > 0) {
                styleMapId = QString::fromLatin1(::TurbopropStyleMapId) % "_" % QString::number(d->turbopropColorRampIndex % d->turbopropColorRamp.size());
                ++d->turbopropColorRampIndex;
            }
            break;
        case SimType::EngineType::Piston:
            nofColors = d->pistonColorRamp.size();
            if (nofColors > 0) {
                styleMapId = QString::fromLatin1(::PistonStyleMapId) % "_" % QString::number(d->pistonColorRampIndex % d->pistonColorRamp.size());
                ++d->pistonColorRampIndex;
            }
            break;
        case SimType::EngineType::Unknown:
            [[fallthrough]];
        case SimType::EngineType::None:
            [[fallthrough]];
        case SimType::EngineType::HeloBellTurbine:
            [[fallthrough]];
        case SimType::EngineType::Unsupported:
            [[fallthrough]];
        case SimType::EngineType::All:
            nofColors = d->allColorRamp.size();
            if (nofColors > 0) {
                styleMapId = QString::fromLatin1(::AllStyleMapId) % "_" % QString::number(d->allColorRampIndex % d->allColorRamp.size());
                ++d->allColorRampIndex;
            }
            break;
        }

    } else {
        nofColors = d->allColorRamp.size();
        if (nofColors > 0) {
            styleMapId = QString::fromLatin1(::AllStyleMapId) % "_" % QString::number(d->allColorRampIndex % d->allColorRamp.size());
            ++d->allColorRampIndex;
        }
    }

    return styleMapId;
}

QString KmlStyleExport::getStyleUrl(Icon icon) noexcept
{
    QString styleUrl;
    switch (icon) {
    case Icon::Airport:
        styleUrl = QStringLiteral("#sm_airports");
        break;
    case Icon::Flag:
        styleUrl = QStringLiteral("#sm_flag");
        break;
    }
    return styleUrl;
}

// PRIVATE

inline void KmlStyleExport::initialiseColorRamps() noexcept
{
    const bool doColorRamp = d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::ColorRamp || d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::ColorRampPerEngineType;
    if (d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::OneColorPerEngineType || d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::ColorRampPerEngineType) {
        d->jetColorRamp = Color::createColorRamp(
                    d->pluginSettings.getJetStartColor(),
                    doColorRamp ? d->pluginSettings.getJetEndColor() : d->pluginSettings.getJetStartColor(),
                    d->pluginSettings.getNofColorsPerRamp());
        d->turbopropColorRamp = Color::createColorRamp(
                    d->pluginSettings.getTurbopropStartColor(),
                    doColorRamp ? d->pluginSettings.getTurbopropEndColor(): d->pluginSettings.getTurbopropStartColor(),
                    d->pluginSettings.getNofColorsPerRamp());
        d->pistonColorRamp = Color::createColorRamp(
                    d->pluginSettings.getPistonStartColor(),
                    doColorRamp ? d->pluginSettings.getPistonEndColor() : d->pluginSettings.getPistonStartColor(),
                    d->pluginSettings.getNofColorsPerRamp());
    }
    d->allColorRamp = Color::createColorRamp(
                d->pluginSettings.getAllStartColor(),
                doColorRamp ? d->pluginSettings.getAllEndColor() : d->pluginSettings.getAllStartColor(),
                d->pluginSettings.getNofColorsPerRamp());
}

bool KmlStyleExport::exportHighlightLineStyle(QIODevice &io) const noexcept
{
    const QRgb lineHighlightKml = Color::convertRgbToKml(LineHighlightColor);
    const QRgb polygonHighlightKml = Color::convertRgbToKml(PolygonHighlightColor);
    const QString style =
"    <Style id=\"s_flight_h\">\n"
"      <LineStyle>\n"
"        <color>" % QString::number(lineHighlightKml, 16) % "</color>\n"
"        <width>" % QString::number(d->pluginSettings.getLineWidth()) % "</width>\n"
"      </LineStyle>\n"
"      <PolyStyle>\n"
"        <color>" % QString::number(polygonHighlightKml, 16) % "</color>\n"
"        <outline>0</outline>\n"
"      </PolyStyle>\n"
"    </Style>\n";

    return io.write(style.toUtf8());
}

bool KmlStyleExport::exportNormalLineStyles(QIODevice &io) const noexcept
{
    const float lineWidth = d->pluginSettings.getLineWidth();
    bool ok {true};
    if (d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::OneColorPerEngineType || d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::ColorRampPerEngineType) {
        // Per engine type (one color or ramp)
        ok = exportNormalLineStylesPerEngineType(SimType::EngineType::Jet, d->jetColorRamp, lineWidth, io);
        if (ok) {
            ok = exportNormalLineStylesPerEngineType(SimType::EngineType::Turboprop, d->turbopropColorRamp, lineWidth, io);
        }
        if (ok) {
            ok = exportNormalLineStylesPerEngineType(SimType::EngineType::Piston, d->pistonColorRamp, lineWidth, io);
        }
    }
    if (ok) {
        ok = exportNormalLineStylesPerEngineType(SimType::EngineType::All, d->allColorRamp, lineWidth, io);
    }
    return ok;
}

bool KmlStyleExport::exportLineStyleMaps(QIODevice &io) const noexcept
{
    bool ok {true};

    // Jet style map
    if (d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::OneColorPerEngineType || d->pluginSettings.getColorStyle() == KmlExportSettings::ColorStyle::ColorRampPerEngineType) {
        // Per engine type (one color or ramp)
        for (std::size_t index = 0; ok && index < d->jetColorRamp.size(); ++index) {
            const QString styleMap =
"    <StyleMap id=\"" % QString(JetStyleMapId) % "_" % QString::number(index) % "\">\n"
"      <Pair>\n"
"        <key>normal</key>\n"
"        <styleUrl>#" % QString(JetStyleId) % "_" % QString::number(index) % "</styleUrl>\n"
"      </Pair>\n"
"      <Pair>\n"
"        <key>highlight</key>\n"
"        <styleUrl>#s_flight_h</styleUrl>\n"
"      </Pair>\n"
"    </StyleMap>\n";
            ok = io.write(styleMap.toUtf8());
        }

        // Turboprop style map
        for (std::size_t index = 0; ok && index < d->turbopropColorRamp.size(); ++index) {
            const QString styleMap =
"    <StyleMap id=\"" % QString(TurbopropStyleMapId) % "_" % QString::number(index) % "\">\n"
"      <Pair>\n"
"        <key>normal</key>\n"
"        <styleUrl>#" % QString(TurbopropStyleId) % "_" % QString::number(index) % "</styleUrl>\n"
"      </Pair>\n"
"      <Pair>\n"
"        <key>highlight</key>\n"
"        <styleUrl>#s_flight_h</styleUrl>\n"
"      </Pair>\n"
"    </StyleMap>\n";
            ok = io.write(styleMap.toUtf8());
        }

        // Piston style map
        for (std::size_t index = 0; ok && index < d->pistonColorRamp.size(); ++index) {
            const QString styleMap =
"    <StyleMap id=\"" % QString(PistonStyleMapId) % "_" % QString::number(index) % "\">\n"
"      <Pair>\n"
"        <key>normal</key>\n"
"        <styleUrl>#" % QString(PistonStyleId) % "_" % QString::number(index) % "</styleUrl>\n"
"      </Pair>\n"
"      <Pair>\n"
"        <key>highlight</key>\n"
"        <styleUrl>#s_flight_h</styleUrl>\n"
"      </Pair>\n"
"    </StyleMap>\n";
            ok = io.write(styleMap.toUtf8());
        }
    } // if color ramps

    // All style map
    for (std::size_t index = 0; ok && index < d->allColorRamp.size(); ++index) {
        const QString styleMap =
"    <StyleMap id=\"" % QString(AllStyleMapId) % "_" % QString::number(index) % "\">\n"
"      <Pair>\n"
"        <key>normal</key>\n"
"        <styleUrl>#" % QString(AllStyleId) % "_" % QString::number(index) % "</styleUrl>\n"
"      </Pair>\n"
"      <Pair>\n"
"        <key>highlight</key>\n"
"        <styleUrl>#s_flight_h</styleUrl>\n"
"      </Pair>\n"
"    </StyleMap>\n";
        ok = io.write(styleMap.toUtf8());
    }

    return ok;
}

bool KmlStyleExport::exportPlacemarkStyles(QIODevice &io) const noexcept
{
    const QString styles =
// Airport
"    <Style id=\"s_airports\">\n"
"      <IconStyle>\n"
"        <scale>1.2</scale>\n"
"        <Icon><href>http://maps.google.com/mapfiles/kml/shapes/airports.png</href></Icon>\n"
"        <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
"      </IconStyle>\n"
"	 </Style>\n"
"    <Style id=\"s_airports_h\">\n"
"      <IconStyle>\n"
"        <scale>1.4</scale>\n"
"        <Icon><href>http://maps.google.com/mapfiles/kml/shapes/airports.png</href></Icon>\n"
"        <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
"      </IconStyle>\n"
"	 </Style>\n"
"    <StyleMap id=\"sm_airports\">\n"
"      <Pair>\n"
"        <key>normal</key>\n"
"        <styleUrl>#s_airports</styleUrl>\n"
"      </Pair>\n"
"      <Pair>\n"
"        <key>highlight</key>\n"
"        <styleUrl>#s_airports_h</styleUrl>\n"
"      </Pair>\n"
"    </StyleMap>\n"

// Flag
"    <Style id=\"s_flag\">\n"
"      <IconStyle>\n"
"        <scale>1.2</scale>\n"
"        <Icon><href>http://maps.google.com/mapfiles/kml/shapes/flag.png</href></Icon>\n"
"        <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
"      </IconStyle>\n"
"	 </Style>\n"
"    <Style id=\"s_flag_h\">\n"
"      <IconStyle>\n"
"        <scale>1.4</scale>\n"
"        <Icon><href>http://maps.google.com/mapfiles/kml/shapes/flag.png</href></Icon>\n"
"        <hotSpot x=\"0.5\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>\n"
"      </IconStyle>\n"
"    </Style>\n"
"    <StyleMap id=\"sm_flag\">\n"
"      <Pair>\n"
"        <key>normal</key>\n"
"        <styleUrl>#s_flag</styleUrl>\n"
"      </Pair>\n"
"      <Pair>\n"
"        <key>highlight</key>\n"
"        <styleUrl>#s_flag_h</styleUrl>\n"
"      </Pair>\n"
"    </StyleMap>\n";

    return io.write(styles.toUtf8());
}

bool KmlStyleExport::exportNormalLineStylesPerEngineType(SimType::EngineType engineType, std::vector<QRgb> &colorRamp, float lineWidth, QIODevice &io) noexcept
{
    QString styleId;
    switch (engineType) {
    case SimType::EngineType::Jet:
        styleId = QString::fromLatin1(::JetStyleId);
        break;
    case SimType::EngineType::Turboprop:
        styleId = QString::fromLatin1(::TurbopropStyleId);
        break;
    case SimType::EngineType::Piston:
        styleId = QString::fromLatin1(::PistonStyleId);
        break;
    case SimType::EngineType::Unknown:
        [[fallthrough]];
    case SimType::EngineType::None:
        [[fallthrough]];
    case SimType::EngineType::HeloBellTurbine:
        [[fallthrough]];
    case SimType::EngineType::Unsupported:
        [[fallthrough]];
    case SimType::EngineType::All:
        styleId = QString::fromLatin1(::AllStyleId);
        break;
    }

    bool ok {true};
    int index = 0;
    const QRgb polygonColorKml = Color::convertRgbToKml(PolygonColor);

    for (const QRgb color : colorRamp) {

        const QRgb lineColorKml = Color::convertRgbToKml(color);
        const QString style =
"    <Style id=\"" % styleId % "_" % QString::number(index) % "\">\n"
"      <LineStyle>\n"
"        <color>" % QString::number(lineColorKml, 16) % "</color>\n"
"        <width>" % QString::number(lineWidth) % "</width>\n"
"      </LineStyle>\n"
"      <PolyStyle>\n"
"        <color>" % QString::number(polygonColorKml, 16) % "</color>\n"
"        <outline>0</outline>\n"
"      </PolyStyle>\n"
"    </Style>\n";

        ok = io.write(style.toUtf8());
        if (ok) {
            ++index;
        } else {
            break;
        }
    }

    return ok;
}
