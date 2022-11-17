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
#include <QChar>
#include <QString>
#include <QStringBuilder>

#include <Kernel/Enum.h>
#include <Kernel/Unit.h>
#include <Model/Location.h>
#include <Model/Enumeration.h>
#include <Persistence/Service/EnumerationService.h>
#include <PluginManager/CsvConst.h>
#include <PluginManager/Export.h>
#include "CsvLocationExportSettings.h"
#include "LittleNavmapCsvLocationWriter.h"

namespace
{
    const QString TypeColumn = QStringLiteral("Type");
    const QString NameColumn = QStringLiteral("Name");
    const QString IdentColumn = QStringLiteral("Ident");
    const QString LatitudeColumn = QStringLiteral("Latitude");
    const QString LongitudeColumn = QStringLiteral("Longitude");
    const QString ElevationColumn = QStringLiteral("Elevation");
    const QString MagneticDeclinationColumn = QStringLiteral("Magnetic Declination");
    const QString TagsColumn = QStringLiteral("Tags");
    const QString DescriptionColumn = QStringLiteral("Description");
    const QString RegionColumn = QStringLiteral("Region");
    const QString VisibleFromColumn = QStringLiteral("Visible From");
    const QString LastEditColumn = QStringLiteral("Last Edit");
    const QString ImportFilenameColumn = QStringLiteral("Import Filename");
}

struct LittleNavmapCsvLocationWriterPrivate
{
    LittleNavmapCsvLocationWriterPrivate(const CsvLocationExportSettings &thePluginSettings) noexcept
        : pluginSettings(thePluginSettings)
    {}

    const CsvLocationExportSettings &pluginSettings;
    EnumerationService enumerationService;
};

// PUBLIC

LittleNavmapCsvLocationWriter::LittleNavmapCsvLocationWriter(const CsvLocationExportSettings &pluginSettings) noexcept
    : d(std::make_unique<LittleNavmapCsvLocationWriterPrivate>(pluginSettings))
{}

LittleNavmapCsvLocationWriter::~LittleNavmapCsvLocationWriter() noexcept = default;

bool LittleNavmapCsvLocationWriter::write(const std::vector<Location> &locations, QIODevice &io) noexcept
{
    QString csv = QString(::TypeColumn % CsvConst::CommaSep %
                          ::NameColumn % CsvConst::CommaSep %
                          ::IdentColumn % CsvConst::CommaSep %
                          ::LatitudeColumn % CsvConst::CommaSep %
                          ::LongitudeColumn % CsvConst::CommaSep %
                          ::ElevationColumn % CsvConst::CommaSep %
                          ::MagneticDeclinationColumn % CsvConst::CommaSep %
                          ::TagsColumn % CsvConst::CommaSep %
                          ::DescriptionColumn % CsvConst::CommaSep %
                          ::RegionColumn % CsvConst::CommaSep %
                          ::VisibleFromColumn % CsvConst::CommaSep %
                          ::LastEditColumn % CsvConst::CommaSep %
                          ::ImportFilenameColumn % CsvConst::Ln
                          );

    bool ok = io.write(csv.toUtf8());
    Enumeration locationTypeEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::LocationType);
    Enumeration locationCategoryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::LocationCategory);
    Enumeration countryEnumeration = d->enumerationService.getEnumerationByName(EnumerationService::Country);
    if (ok) {
        for (const Location &location : locations) {
            QString title = location.title;
            QString description = location.description;
            QString identifier = location.identifier;
            const QString countrySymbolicId = countryEnumeration.getItemById(location.countryId).symbolicId;
            const QString csv = "\"" % QString("Settlement") % "\"" % CsvConst::CommaSep %
                                "\"" % title.replace("\"", "\"\"") % "\"" % CsvConst::CommaSep %
                                "\"" % identifier.replace("\"", "\"\"") % "\"" % CsvConst::CommaSep %
                                QString::number(location.latitude) % CsvConst::CommaSep %
                                QString::number(location.longitude) % CsvConst::CommaSep %
                                QString::number(location.altitude) % CsvConst::CommaSep %
                                "" % CsvConst::CommaSep %
                                "" % CsvConst::CommaSep %
                                "\"" % description.replace("\"", "\"\"") % "\"" % CsvConst::CommaSep %
                                countrySymbolicId % CsvConst::CommaSep %
                                "" % CsvConst::CommaSep %
                                "" % CsvConst::CommaSep %
                                "" % CsvConst::Ln;
            ok = io.write(csv.toUtf8());

            if (!ok) {
                break;
            }
        }
    }
    return ok;
}

// PRIVATE
