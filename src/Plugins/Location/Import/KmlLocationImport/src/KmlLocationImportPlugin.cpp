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

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringLiteral>
#include <QXmlStreamReader>
#include <QWidget>
#include <QIODevice>
#include <QFile>

#include <Kernel/Const.h>
#include <Model/Location.h>
#include <Persistence/Service/LogbookService.h>
#include <Persistence/Service/DatabaseService.h>
#include <Persistence/Service/LocationService.h>
#include "KmlLocationImportSettings.h"
#include "KmlParserIntf.h"
#include "PlacemarkKmlParser.h"
#include "KmlLocationImportOptionWidget.h"
#include "KmlLocationImportPlugin.h"

struct KmlLocationImportPluginPrivate
{
    QXmlStreamReader xml;
    KmlLocationImportSettings pluginSettings;

    static inline const QString FileExtension {"kml"};
};

// PUBLIC

KmlLocationImportPlugin::KmlLocationImportPlugin() noexcept
    : d {std::make_unique<KmlLocationImportPluginPrivate>()}
{}

KmlLocationImportPlugin::~KmlLocationImportPlugin() = default;

// PROTECTED

LocationImportPluginBaseSettings &KmlLocationImportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString KmlLocationImportPlugin::getFileExtension() const noexcept
{
    return KmlLocationImportPluginPrivate::FileExtension;
}

QString KmlLocationImportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Keyhole markup language (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> KmlLocationImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<KmlLocationImportOptionWidget>(d->pluginSettings);
}

std::vector<Location> KmlLocationImportPlugin::importLocations(QIODevice &io, bool &ok) noexcept
{
    std::vector<Location> locations;

    d->xml.setDevice(&io);
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug() << "KmlImportPlugin::importSelectedFlights: XML start element:" << d->xml.name().toString();
#endif
        if (d->xml.name() == QStringLiteral("kml")) {
            locations = parseKML();
        } else {
            d->xml.raiseError("The file is not a KML file.");
        }
    }

    ok = !d->xml.hasError();
    if (!ok) {
#ifdef DEBUG
        qDebug() << "KmlImportPlugin::importLocations: XML error:" << d->xml.errorString();
#endif
    }

    return locations;
}

// PRIVATE

std::vector<Location> KmlLocationImportPlugin::parseKML() noexcept
{
    std::vector<Location> locations;
    std::unique_ptr<KmlParserIntf> parser;

    parser = std::make_unique<PlacemarkKmlParser>(d->pluginSettings);
    locations = parser->parse(d->xml);

    return locations;
}
