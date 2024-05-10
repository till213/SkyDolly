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
    GpxImportSettings pluginSettings;
    QXmlStreamReader xml;    
    std::unique_ptr<GpxParser> parser;

    static inline const QString FileExtension {"gpx"};
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

std::vector<FlightData> GpxImportPlugin::importSelectedFlights(QIODevice &io, bool &ok) noexcept
{
    d->xml.setDevice(&io);
    std::vector<FlightData> flights = parseGPX();

    ok = FlightData::hasAllRecording(flights) && !d->xml.hasError();
    if (!ok) {
        flights.clear();
#ifdef DEBUG
        qDebug() << "GpxImportPlugin::importSelectedFlights: XML error" << d->xml.errorString();
#endif
    }
    return flights;
}

FlightAugmentation::Procedures GpxImportPlugin::getAugmentationProcedures() const noexcept
{
    return FlightAugmentation::Procedure::All;
}

FlightAugmentation::Aspects GpxImportPlugin::getAugmentationAspects() const noexcept
{
    return FlightAugmentation::Aspect::All;
}

// PRIVATE

std::vector<FlightData> GpxImportPlugin::parseGPX() noexcept
{
    std::vector<FlightData> flights;
    d->parser = std::make_unique<GpxParser>(d->xml, d->pluginSettings);
    flights = d->parser->parse();
    updateFlightWaypoints(flights);
    return flights;
}

void GpxImportPlugin::updateFlightWaypoints(std::vector<FlightData> &flights) noexcept
{
    for (FlightData &flightData : flights) {
        for (Aircraft &aircraft : flightData) {
            updateAircraftWaypoints(aircraft, flightData.getAircraftStartZuluTime(aircraft));
        }
    }
}

void GpxImportPlugin::updateAircraftWaypoints(Aircraft &aircraft, const QDateTime &flightTimeUtc) noexcept
{
    Position &position = aircraft.getPosition();

    if (position.count() > 0) {
        Analytics analytics {aircraft};
        const PositionData firstPositionData = position.getFirst();
        const PositionData lastPositionData = position.getLast();
        const QDateTime endDateTimeUtc = flightTimeUtc.addMSecs(lastPositionData.timestamp);

        // Assign timestamps according to the closest flown position
        std::unordered_set<std::int64_t> timestamps;
        std::int64_t uniqueTimestamp {0};
        FlightPlan &flightPlan = aircraft.getFlightPlan();
        const std::size_t count = flightPlan.count();
        for (std::size_t i = 0; i < count; ++i) {
            Waypoint &waypoint = flightPlan[i];
            if (i == 0) {
                // First waypoint
                waypoint.localTime = flightTimeUtc.toLocalTime();
                waypoint.zuluTime = flightTimeUtc;
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
                    const QDateTime dateTimeUtc = flightTimeUtc.addMSecs(closestPositionData.timestamp);
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
