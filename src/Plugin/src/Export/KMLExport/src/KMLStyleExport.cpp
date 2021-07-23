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
#include <memory>
#include <vector>

#include <QIODevice>
#include <QString>
#include <QStringBuilder>
#include <QColor>

#include "../../../../../Kernel/src/Color.h"
#include "../../../../../Model/src/SimType.h"
#include "KMLStyleExport.h"

namespace
{
    constexpr char LineWidth[] = "3.5";

    // in AARRGGBB format
    // https://designs.ai/colors/color-wheel
    // http://khroma.co/generator/
    // http://colormind.io/
    const QRgb Opaque = 0xff000000;

    // Cyan
    constexpr QRgb JetStartColor = Opaque | 0x00d9ff;
    constexpr QRgb JetEndColor = Opaque | 0x001aff;
    // Red
    constexpr QRgb TurbopropStartColor = Opaque | 0xe60000;
    constexpr QRgb TurbopropEndColor = Opaque | 0x66008c;
    // Green
    constexpr QRgb PistonStartColor = Opaque | 0x00ff20;
    constexpr QRgb PistonEndColor = Opaque | 0x007020;
    // Orange
    constexpr QRgb OtherStartColor = Opaque | 0xff8f00;
    constexpr QRgb OtherEndColor = Opaque | 0xa06417;

    constexpr QRgb LineHighlightColor = 0xffffff00;
    constexpr QRgb PolygonHighlightColor = 0xcc7ed5c9;
    constexpr QRgb PolygonColor = 0x337ed5c9;

    constexpr char JetStyleId[] = "s_jet_style";
    constexpr char TurbopropStyleId[] = "s_turbo_prop_style";
    constexpr char PistonStyleId[] = "s_piston_style";
    constexpr char OtherStyleId[] = "s_other_style";

    constexpr char JetStyleMapId[] = "sm_jet_style";
    constexpr char TurbopropStyleMapId[] = "sm_turbo_prop_style";
    constexpr char PistonStyleMapId[] = "sm_piston_style";
    constexpr char OtherStyleMapId[] = "sm_other_style";
}

class KMLStyleExportPrivate
{
public:
    KMLStyleExportPrivate(int theNofColorsPerRamp) noexcept
        : nofColorsPerRamp(theNofColorsPerRamp),
          jetColorRampIndex(0),
          turbopropColorRampIndex(0),
          pistonColorRampIndex(0),
          otherColorRampIndex(0)
    {
        jetColorRamp = Color::createColorRamp(JetStartColor, JetEndColor, nofColorsPerRamp);
        turbopropColorRamp = Color::createColorRamp(TurbopropStartColor, TurbopropEndColor, nofColorsPerRamp);
        pistonColorRamp = Color::createColorRamp(PistonStartColor, PistonEndColor, nofColorsPerRamp);
        otherColorRamp = Color::createColorRamp(OtherStartColor, OtherEndColor, nofColorsPerRamp);
        jetColorRampIndex = 0;
        turbopropColorRampIndex = 0;
        pistonColorRampIndex = 0;
        otherColorRampIndex = 0;
    }

    int nofColorsPerRamp;
    std::vector<QRgb> jetColorRamp;
    std::vector<QRgb> turbopropColorRamp;
    std::vector<QRgb> pistonColorRamp;
    std::vector<QRgb> otherColorRamp;
    // Index into color ramp, modulo its size
    int jetColorRampIndex;
    int turbopropColorRampIndex;
    int pistonColorRampIndex;
    int otherColorRampIndex;
};

// PUBLIC

KMLStyleExport::KMLStyleExport(int nofColorsPerRamp) noexcept
    : d(std::make_unique<KMLStyleExportPrivate>(nofColorsPerRamp))
{}

KMLStyleExport::~KMLStyleExport() noexcept
{}

bool KMLStyleExport::exportStyles(QIODevice &io) const noexcept
{
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

QString KMLStyleExport::getNextStyleMapPerEngineType(SimType::EngineType engineType) noexcept
{
    QString styleMapId;

    switch (engineType) {
    case SimType::EngineType::Jet:
        styleMapId = QString(JetStyleMapId) % "_" % QString::number(d->jetColorRampIndex % d->nofColorsPerRamp);
        ++d->jetColorRampIndex;
        break;
    case SimType::EngineType::Turboprop:
        styleMapId = QString(TurbopropStyleMapId) % "_" % QString::number(d->turbopropColorRampIndex % d->nofColorsPerRamp);
        ++d->turbopropColorRampIndex;
        break;
    case SimType::EngineType::Piston:
        styleMapId = QString(PistonStyleMapId) % "_" % QString::number(d->pistonColorRampIndex % d->nofColorsPerRamp);
        ++d->pistonColorRampIndex;
        break;
    default:
        styleMapId = QString(OtherStyleMapId) % "_" % QString::number(d->otherColorRampIndex % d->nofColorsPerRamp);
        ++d->otherColorRampIndex;
        break;
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

bool KMLStyleExport::exportHighlightLineStyle(QIODevice &io) const noexcept
{
    const QRgb lineHighlightKml = Color::convertRgbToKml(LineHighlightColor);
    const QRgb polygonHighlightKml = Color::convertRgbToKml(PolygonHighlightColor);
    const QString style =
"    <Style id=\"s_flight_h\">\n"
"      <LineStyle>\n"
"        <color>" % QString::number(lineHighlightKml, 16) % "</color>\n"
"        <width>" % QString(LineWidth) % "</width>\n"
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
    bool ok = exportNormalLineStylesPerEngineType(SimType::EngineType::Jet, d->jetColorRamp, io);
    if (ok) {
        ok = exportNormalLineStylesPerEngineType(SimType::EngineType::Turboprop, d->turbopropColorRamp, io);
    }
    if (ok) {
        ok = exportNormalLineStylesPerEngineType(SimType::EngineType::Piston, d->pistonColorRamp, io);
    }
    if (ok) {
        ok = exportNormalLineStylesPerEngineType(SimType::EngineType::All, d->otherColorRamp, io);
    }

    return ok;
}

bool KMLStyleExport::exportLineStyleMaps(QIODevice &io) const noexcept
{
    bool ok = true;

    // Jet style map
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
    for (std::size_t index = 0; ok && index < d->jetColorRamp.size(); ++index) {
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
    for (std::size_t index = 0; ok && index < d->jetColorRamp.size(); ++index) {
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

    // Other style map
    for (std::size_t index = 0; ok && index < d->jetColorRamp.size(); ++index) {
        const QString styleMap =
"    <StyleMap id=\"" % QString(OtherStyleMapId) % "_" % QString::number(index) % "\">\n"
"      <Pair>\n"
"        <key>normal</key>\n"
"        <styleUrl>#" % QString(OtherStyleId) % "_" % QString::number(index) % "</styleUrl>\n"
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

bool KMLStyleExport::exportNormalLineStylesPerEngineType(SimType::EngineType engineType, const std::vector<QRgb> &colorRamp, QIODevice &io) noexcept
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
        styleId = OtherStyleId;
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
"        <width>" % QString(LineWidth) % "</width>\n"
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
