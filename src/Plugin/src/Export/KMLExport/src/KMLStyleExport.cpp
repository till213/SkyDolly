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
#include <memory>
#include <vector>

#include <QIODevice>
#include <QString>
#include <QStringBuilder>
#include <QColor>

#include "../../../../../Kernel/src/Color.h"
#include "../../../../../Model/src/SimType.h"
#include "KMLExportSettings.h"
#include "KMLStyleExport.h"

namespace
{
    constexpr QRgb LineHighlightColor = 0xffffff00;
    constexpr QRgb PolygonHighlightColor = 0xcc7ed5c9;
    constexpr QRgb PolygonColor = 0x337ed5c9;

    constexpr char JetStyleId[] = "s_jet_style";
    constexpr char TurbopropStyleId[] = "s_turbo_prop_style";
    constexpr char PistonStyleId[] = "s_piston_style";
    constexpr char AllStyleId[] = "s_all_style";

    constexpr char JetStyleMapId[] = "sm_jet_style";
    constexpr char TurbopropStyleMapId[] = "sm_turbo_prop_style";
    constexpr char PistonStyleMapId[] = "sm_piston_style";
    constexpr char AllStyleMapId[] = "sm_all_style";
}

class KMLStyleExportPrivate
{
public:
    KMLStyleExportPrivate(const KMLExportSettings &theExportSettings) noexcept
        : settings(theExportSettings),
          jetColorRampIndex(0),
          turbopropColorRampIndex(0),
          pistonColorRampIndex(0),
          allColorRampIndex(0)
    {
        jetColorRampIndex = 0;
        turbopropColorRampIndex = 0;
        pistonColorRampIndex = 0;
        allColorRampIndex = 0;
    }

    const KMLExportSettings &settings;
    std::vector<QRgb> jetColorRamp;
    std::vector<QRgb> turbopropColorRamp;
    std::vector<QRgb> pistonColorRamp;
    std::vector<QRgb> allColorRamp;
    // Index into color ramp, modulo its size
    int jetColorRampIndex;
    int turbopropColorRampIndex;
    int pistonColorRampIndex;
    int allColorRampIndex;
};

// PUBLIC

KMLStyleExport::KMLStyleExport(const KMLExportSettings &settings) noexcept
    : d(std::make_unique<KMLStyleExportPrivate>(settings))
{}

KMLStyleExport::~KMLStyleExport() noexcept
{}

bool KMLStyleExport::exportStyles(QIODevice &io) noexcept
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

QString KMLStyleExport::getNextEngineTypeStyleMap(SimType::EngineType engineType) noexcept
{
    QString styleMapId;
    std::size_t nofColors;

    if (d->settings.getColorStyle() == KMLExportSettings::ColorStyle::OneColorPerEngineType || d->settings.getColorStyle() == KMLExportSettings::ColorStyle::ColorRampPerEngineType) {
        switch (engineType) {
        case SimType::EngineType::Jet:
            nofColors = d->jetColorRamp.size();
            if (nofColors > 0) {
                styleMapId = QString(JetStyleMapId) % "_" % QString::number(d->jetColorRampIndex % nofColors);
                ++d->jetColorRampIndex;
            }
            break;
        case SimType::EngineType::Turboprop:
            nofColors = d->turbopropColorRamp.size();
            if (nofColors > 0) {
                styleMapId = QString(TurbopropStyleMapId) % "_" % QString::number(d->turbopropColorRampIndex % d->turbopropColorRamp.size());
                ++d->turbopropColorRampIndex;
            }
            break;
        case SimType::EngineType::Piston:
            nofColors = d->pistonColorRamp.size();
            if (nofColors > 0) {
                styleMapId = QString(PistonStyleMapId) % "_" % QString::number(d->pistonColorRampIndex % d->pistonColorRamp.size());
                ++d->pistonColorRampIndex;
            }
            break;
        default:
            nofColors = d->allColorRamp.size();
            if (nofColors > 0) {
                styleMapId = QString(AllStyleMapId) % "_" % QString::number(d->allColorRampIndex % d->allColorRamp.size());
                ++d->allColorRampIndex;
            }
            break;
        }
    } else {
        nofColors = d->allColorRamp.size();
        if (nofColors > 0) {
            styleMapId = QString(AllStyleMapId) % "_" % QString::number(d->allColorRampIndex % d->allColorRamp.size());
            ++d->allColorRampIndex;
        }
    }

    return styleMapId;
}

QString KMLStyleExport::getStyleUrl(Icon icon) noexcept
{
    QString styleUrl;
    switch (icon) {
    case Icon::Airport:
        styleUrl = "#sm_airports";
        break;
    case Icon::Flag:
        styleUrl = "#sm_flag";
        break;
    default:
        styleUrl = "#sm_airports";
        break;
    }
    return styleUrl;
}

// PRIVATE

inline void KMLStyleExport::initialiseColorRamps() noexcept
{
    const bool doColorRamp = d->settings.getColorStyle() == KMLExportSettings::ColorStyle::ColorRamp || d->settings.getColorStyle() == KMLExportSettings::ColorStyle::ColorRampPerEngineType;
    if (d->settings.getColorStyle() == KMLExportSettings::ColorStyle::OneColorPerEngineType || d->settings.getColorStyle() == KMLExportSettings::ColorStyle::ColorRampPerEngineType) {
        d->jetColorRamp = Color::createColorRamp(
                    d->settings.getJetStartColor(),
                    doColorRamp ? d->settings.getJetEndColor() : d->settings.getJetStartColor(),
                    d->settings.getNofColorsPerRamp());
        d->turbopropColorRamp = Color::createColorRamp(
                    d->settings.getTurbopropStartColor(),
                    doColorRamp ? d->settings.getTurbopropEndColor(): d->settings.getTurbopropStartColor(),
                    d->settings.getNofColorsPerRamp());
        d->pistonColorRamp = Color::createColorRamp(
                    d->settings.getPistonStartColor(),
                    doColorRamp ? d->settings.getPistonEndColor() : d->settings.getPistonStartColor(),
                    d->settings.getNofColorsPerRamp());
    }
    d->allColorRamp = Color::createColorRamp(
                d->settings.getAllStartColor(),
                doColorRamp ? d->settings.getAllEndColor() : d->settings.getAllStartColor(),
                d->settings.getNofColorsPerRamp());
}

bool KMLStyleExport::exportHighlightLineStyle(QIODevice &io) const noexcept
{
    const QRgb lineHighlightKml = Color::convertRgbToKml(LineHighlightColor);
    const QRgb polygonHighlightKml = Color::convertRgbToKml(PolygonHighlightColor);
    const QString style =
"    <Style id=\"s_flight_h\">\n"
"      <LineStyle>\n"
"        <color>" % QString::number(lineHighlightKml, 16) % "</color>\n"
"        <width>" % QString::number(d->settings.getLineWidth()) % "</width>\n"
"      </LineStyle>\n"
"      <PolyStyle>\n"
"        <color>" % QString::number(polygonHighlightKml, 16) % "</color>\n"
"        <outline>0</outline>\n"
"      </PolyStyle>\n"
"    </Style>\n";

    return io.write(style.toUtf8());
}

bool KMLStyleExport::exportNormalLineStyles(QIODevice &io) const noexcept
{
    const float lineWidth = d->settings.getLineWidth();
    bool ok = true;
    if (d->settings.getColorStyle() == KMLExportSettings::ColorStyle::OneColorPerEngineType || d->settings.getColorStyle() == KMLExportSettings::ColorStyle::ColorRampPerEngineType) {
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

bool KMLStyleExport::exportLineStyleMaps(QIODevice &io) const noexcept
{
    bool ok = true;

    // Jet style map
    if (d->settings.getColorStyle() == KMLExportSettings::ColorStyle::OneColorPerEngineType || d->settings.getColorStyle() == KMLExportSettings::ColorStyle::ColorRampPerEngineType) {
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

bool KMLStyleExport::exportPlacemarkStyles(QIODevice &io) const noexcept
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

bool KMLStyleExport::exportNormalLineStylesPerEngineType(SimType::EngineType engineType, std::vector<QRgb> &colorRamp, float lineWidth, QIODevice &io) noexcept
{
    QString styleId;
    switch (engineType) {
    case SimType::EngineType::Jet:
        styleId = JetStyleId;
        break;
    case SimType::EngineType::Turboprop:
        styleId = TurbopropStyleId;
        break;
    case SimType::EngineType::Piston:
        styleId = PistonStyleId;
        break;
    default:
        styleId = AllStyleId;
        break;
    }

    bool ok = true;
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
