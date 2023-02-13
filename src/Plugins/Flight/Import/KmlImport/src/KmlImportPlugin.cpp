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

#include <QFile>
#include <QFileInfo>
#include <QStringLiteral>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QTimeZone>
#include <QFlags>

#include <Kernel/Unit.h>
#include <Model/Flight.h>
#include <Model/Aircraft.h>
#include <Model/FlightCondition.h>
#include <Flight/FlightAugmentation.h>
#include "KmlImportSettings.h"
#include "KmlParserIntf.h"
#include "FlightAwareKmlParser.h"
#include "FlightRadar24KmlParser.h"
#include "GenericKmlParser.h"
#include "KmlImportOptionWidget.h"
#include "KmlImportPlugin.h"

struct KmlImportPluginPrivate
{
    QXmlStreamReader xml;
    KmlImportSettings pluginSettings;
    QDateTime firstDateTimeUtc;

    static constexpr const char *FileExtension {"kml"};
};

// PUBLIC

KmlImportPlugin::KmlImportPlugin() noexcept
    : d(std::make_unique<KmlImportPluginPrivate>())
{}

KmlImportPlugin::~KmlImportPlugin() = default;

// PROTECTED

FlightImportPluginBaseSettings &KmlImportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString KmlImportPlugin::getFileExtension() const noexcept
{
    return KmlImportPluginPrivate::FileExtension;
}

QString KmlImportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("Keyhole markup language (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> KmlImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<KmlImportOptionWidget>(d->pluginSettings);
}

std::vector<FlightData> KmlImportPlugin::importFlights(QIODevice &io, bool &ok) noexcept
{
    std::vector<FlightData> flights;
    d->xml.setDevice(&io);
    if (d->xml.readNextStartElement()) {
#ifdef DEBUG
        qDebug() << "KmlImportPlugin::importFlight: XML start element:" << d->xml.name().toString();
#endif
        if (d->xml.name() == QStringLiteral("kml")) {
            flights = parseKML();
        } else {
            d->xml.raiseError(QStringLiteral("The file is not a KML file."));
        }
    }

    ok = !flights.empty() && !d->xml.hasError();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "KmlImportPlugin::importFlight: XML error:" << d->xml.errorString();
    }
#endif
    return flights;
}

FlightAugmentation::Procedures KmlImportPlugin::getProcedures() const noexcept
{
    return FlightAugmentation::Procedure::All;
}

FlightAugmentation::Aspects KmlImportPlugin::getAspects() const noexcept
{
    FlightAugmentation::Aspects aspects;
    switch (d->pluginSettings.getFormat()) {
    case KmlImportSettings::Format::FlightAware:
        aspects = FlightAugmentation::Aspect::All;
        break;
    case KmlImportSettings::Format::FlightRadar24:
        // Do not augment heading and velocity
        aspects = FlightAugmentation::Aspect::All;
        aspects.setFlag(FlightAugmentation::Aspect::Heading, false);
        aspects.setFlag(FlightAugmentation::Aspect::Velocity, false);
        break;
    case KmlImportSettings::Format::Generic:
        aspects = FlightAugmentation::Aspect::All;
        break;
    }

    return aspects;
}

// PRIVATE

std::vector<FlightData> KmlImportPlugin::parseKML() noexcept
{
    std::vector<FlightData> flights;
    std::unique_ptr<KmlParserIntf> parser;
    switch (d->pluginSettings.getFormat()) {
    case KmlImportSettings::Format::FlightAware:
        parser = std::make_unique<FlightAwareKmlParser>();
        break;
    case KmlImportSettings::Format::FlightRadar24:
        parser = std::make_unique<FlightRadar24KmlParser>();
        break;
    case KmlImportSettings::Format::Generic:
        parser = std::make_unique<GenericKmlParser>();
        break;
    }
    if (parser != nullptr) {
        flights = parser->parse(d->xml);
    }
    return flights;
}
