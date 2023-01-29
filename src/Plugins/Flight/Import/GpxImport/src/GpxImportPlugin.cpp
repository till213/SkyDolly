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
#include <unordered_set>

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringLiteral>
#include <QStringBuilder>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QTimeZone>
#include <QFlags>
#include <QWidget>

#include <Flight/Analytics.h>
#include <Model/Flight.h>
#include <Model/FlightCondition.h>
#include <Model/Aircraft.h>
#include <Model/Position.h>
#include <Model/PositionData.h>
#include <Model/Engine.h>
#include <Model/EngineData.h>
#include <Model/FlightPlan.h>
#include <Model/Waypoint.h>
#include "GpxImportOptionWidget.h"
#include "GpxImportSettings.h"
#include "GpxParser.h"
#include "GpxImportPlugin.h"

struct GpxImportPluginPrivate
{
    Flight *flight {nullptr};
    GpxImportSettings pluginSettings;
    QXmlStreamReader xml;    
    std::unique_ptr<GpxParser> parser;

    static constexpr const char *FileExtension {"gpx"};
};

// PUBLIC

GpxImportPlugin::GpxImportPlugin() noexcept
    : d(std::make_unique<GpxImportPluginPrivate>())
{}

GpxImportPlugin::~GpxImportPlugin() = default;

// PROTECTED

FlightImportPluginBaseSettings &GpxImportPlugin::getPluginSettings() const noexcept
{
    return d->pluginSettings;
}

QString GpxImportPlugin::getFileExtension() const noexcept
{
    return GpxImportPluginPrivate::FileExtension;
}

QString GpxImportPlugin::getFileFilter() const noexcept
{
    return QObject::tr("GPX exchange format (*.%1)").arg(getFileExtension());
}

std::unique_ptr<QWidget> GpxImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<GpxImportOptionWidget>(d->pluginSettings);
}

bool GpxImportPlugin::importFlight(QFile &file, Flight &flight) noexcept
{
    d->flight = &flight;
    d->xml.setDevice(&file);
    parseGPX();

    bool ok = !d->xml.hasError();
#ifdef DEBUG
    if (!ok) {
        qDebug() << "GpxImportPlugin::import: XML error" << d->xml.errorString();
    }
#endif
    // We are done with the import
    d->flight = nullptr;
    return ok;
}

FlightAugmentation::Procedures GpxImportPlugin::getProcedures() const noexcept
{
    return FlightAugmentation::Procedure::All;
}

FlightAugmentation::Aspects GpxImportPlugin::getAspects() const noexcept
{
    return FlightAugmentation::Aspect::All;
}

QDateTime GpxImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->parser->getFirstDateTimeUtc();
}

QString GpxImportPlugin::getTitle() const noexcept
{
    QString title = d->parser->getDocumentName();
    if (title.isEmpty()) {
        title = QObject::tr("GPX import");
    }
    return title;
}

void GpxImportPlugin::updateExtendedAircraftInfo([[maybe_unused]] AircraftInfo &aircraftInfo) noexcept
{}

void GpxImportPlugin::updateExtendedFlightInfo(Flight &flight) noexcept
{
    const QString description = flight.getDescription() % "\n\n" % d->parser->getDescription();
    flight.setDescription(description);
}

void GpxImportPlugin::updateExtendedFlightCondition([[maybe_unused]] FlightCondition &flightCondition) noexcept
{}

// PRIVATE

void GpxImportPlugin::parseGPX() noexcept
{
    d->parser = std::make_unique<GpxParser>(*d->flight, d->xml, d->pluginSettings);
    d->parser->parse();
    updateWaypoints();
}

void GpxImportPlugin::updateWaypoints() noexcept
{
    const Aircraft &aircraft = d->flight->getUserAircraft();
    Position &position = aircraft.getPosition();

    if (position.count() > 0) {
        Analytics analytics(aircraft);
        const PositionData firstPositionData = position.getFirst();
        const PositionData lastPositionData = position.getLast();
        const QDateTime startDateTimeUtc = d->parser->getFirstDateTimeUtc();
        const QDateTime endDateTimeUtc = startDateTimeUtc.addMSecs(lastPositionData.timestamp);

        // Assign timestamps according to the closest flown position
        std::unordered_set<std::int64_t> timestamps;
        std::int64_t uniqueTimestamp {0};
        FlightPlan &flightPlan = aircraft.getFlightPlan();
        const std::size_t count = flightPlan.count();
        for (int i = 0; i < count; ++i) {
            Waypoint &waypoint = flightPlan[i];
            if (i == 0) {
                // First waypoint
                waypoint.localTime = startDateTimeUtc.toLocalTime();
                waypoint.zuluTime = startDateTimeUtc;
                uniqueTimestamp = firstPositionData.timestamp;
                waypoint.timestamp = uniqueTimestamp;
                timestamps.insert(uniqueTimestamp);
            } else if (i == count - 1) {
                // Last waypoint
                waypoint.localTime = endDateTimeUtc.toLocalTime();
                waypoint.zuluTime = endDateTimeUtc;
                uniqueTimestamp = lastPositionData.timestamp;
                while (timestamps.contains(uniqueTimestamp)) {
                    ++uniqueTimestamp;
                }
                waypoint.timestamp = uniqueTimestamp;
                timestamps.insert(uniqueTimestamp);
            } else {
                // In between waypoints
                if (waypoint.timestamp == TimeVariableData::InvalidTime) {
                    const PositionData &closestPositionData = analytics.closestPosition(waypoint.latitude, waypoint.longitude);
                    const QDateTime dateTimeUtc = startDateTimeUtc.addMSecs(closestPositionData.timestamp);
                    waypoint.localTime = dateTimeUtc.toLocalTime();
                    waypoint.zuluTime = dateTimeUtc;
                    uniqueTimestamp = closestPositionData.timestamp;
                    while (timestamps.contains(uniqueTimestamp)) {
                        ++uniqueTimestamp;
                    }
                    waypoint.timestamp = uniqueTimestamp;
                    timestamps.insert(uniqueTimestamp);
                }
            }
        }
    } else {
        // No positions - use timestamps 0, 1, 2, ...
        std::int64_t currentWaypointTimestamp = 0;
        for (Waypoint &waypoint : aircraft.getFlightPlan()) {
            waypoint.timestamp = currentWaypointTimestamp;
            ++currentWaypointTimestamp;
        }
    }
}
