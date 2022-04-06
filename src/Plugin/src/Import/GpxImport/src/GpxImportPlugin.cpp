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

#include "../../../../../Flight/src/Analytics.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "GpxImportOptionWidget.h"
#include "GpxImportSettings.h"
#include "GpxParser.h"
#include "GpxImportPlugin.h"

class GpxImportPluginPrivate
{
public:
    GpxImportPluginPrivate()
    {}

    GpxImportSettings settings;
    QXmlStreamReader xml;    
    std::unique_ptr<GpxParser> parser;

    static inline const QString FileExtension {QStringLiteral("gpx")};
};

// PUBLIC

GpxImportPlugin::GpxImportPlugin() noexcept
    : d(std::make_unique<GpxImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("GpxImportPlugin::GpxImportPlugin: PLUGIN LOADED");
#endif
}

GpxImportPlugin::~GpxImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("GpxImportPlugin::~GpxImportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

ImportPluginBaseSettings &GpxImportPlugin::getSettings() const noexcept
{
    return d->settings;
}

QString GpxImportPlugin::getFileFilter() const noexcept
{
    return tr("GPX exchange format (*.%1)").arg(GpxImportPluginPrivate::FileExtension);
}

std::unique_ptr<QWidget> GpxImportPlugin::createOptionWidget() const noexcept
{
    return std::make_unique<GpxImportOptionWidget>(d->settings);
}

bool GpxImportPlugin::readFile(QFile &file) noexcept
{
    d->xml.setDevice(&file);
    parseGPX();

    bool ok = !d->xml.hasError();
#ifdef DEBUG
    if (!ok) {
        qDebug("GpxImportPlugin::import: XML error: %s", qPrintable(d->xml.errorString()));
    }
#endif
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
        title = tr("GPX import");
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

// PROTECTED SLOTS

void GpxImportPlugin::onRestoreDefaultSettings() noexcept
{
    d->settings.restoreDefaults();
}

// PRIVATE

void GpxImportPlugin::parseGPX() noexcept
{
    d->parser = std::make_unique<GpxParser>(d->xml, d->settings);
    d->parser->parse();
    updateWaypoints();
}

void GpxImportPlugin::updateWaypoints() noexcept
{
    Flight &flight = Logbook::getInstance().getCurrentFlight();
    const Aircraft &aircraft = flight.getUserAircraft();
    Position &position = aircraft.getPosition();

    if (position.count() > 0) {
        Analytics analytics(aircraft);
        const PositionData firstPositionData = position.getFirst();
        const PositionData lastPositionData = position.getLast();
        const QDateTime startDateTimeUtc = d->parser->getFirstDateTimeUtc();
        const QDateTime endDateTimeUtc = startDateTimeUtc.addMSecs(lastPositionData.timestamp);

        // Assign timestamps according to the closest flown position
        std::unordered_set<std::int64_t> timestamps;
        std::int64_t uniqueTimestamp;
        FlightPlan &flightPlan = aircraft.getFlightPlan();
        const int count = flightPlan.count();
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
                while (timestamps.find(uniqueTimestamp) != timestamps.end()) {
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
                    while (timestamps.find(uniqueTimestamp) != timestamps.end()) {
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
