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
#include <tuple>
#include <vector>
#include <unordered_set>

#include <QStringBuilder>
#include <QIODevice>
#include <QFlags>
#include <QByteArray>
#include <QList>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <QDateTime>
#include <QTimeZone>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringView>

#include "../../../../../Kernel/src/Unit.h"
#include "../../../../../Kernel/src/Settings.h"
#include "../../../../../Kernel/src/SkyMath.h"
#include "../../../../../Kernel/src/Convert.h"
#include "../../../../../Flight/src/Analytics.h"
#include "../../../../../Model/src/SimVar.h"
#include "../../../../../Model/src/Logbook.h"
#include "../../../../../Model/src/Flight.h"
#include "../../../../../Model/src/FlightCondition.h"
#include "../../../../../Model/src/Aircraft.h"
#include "../../../../../Model/src/Position.h"
#include "../../../../../Model/src/PositionData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/Engine.h"
#include "../../../../../Model/src/EngineData.h"
#include "../../../../../Model/src/PrimaryFlightControl.h"
#include "../../../../../Model/src/PrimaryFlightControlData.h"
#include "../../../../../Model/src/SecondaryFlightControl.h"
#include "../../../../../Model/src/SecondaryFlightControlData.h"
#include "../../../../../Model/src/AircraftHandle.h"
#include "../../../../../Model/src/AircraftHandleData.h"
#include "../../../../../Model/src/Light.h"
#include "../../../../../Model/src/LightData.h"
#include "../../../../../Model/src/FlightPlan.h"
#include "../../../../../Model/src/Waypoint.h"
#include "../../../../../SkyConnect/src/SkyConnectManager.h"
#include "../../../../../SkyConnect/src/SkyConnectIntf.h"
#include "IGCParser.h"
#include "IGCImportPlugin.h"



class IGCImportPluginPrivate
{
public:
    IGCImportPluginPrivate()
    {}

    IGCParser igcParser;
};

// PUBLIC

IGCImportPlugin::IGCImportPlugin() noexcept
    : d(std::make_unique<IGCImportPluginPrivate>())
{
#ifdef DEBUG
    qDebug("IGCImportPlugin::IGCImportPlugin: PLUGIN LOADED");
#endif
}

IGCImportPlugin::~IGCImportPlugin() noexcept
{
#ifdef DEBUG
    qDebug("IGCImportPlugin::~IGCImportPlugin: PLUGIN UNLOADED");
#endif
}

// PROTECTED

bool IGCImportPlugin::readFile(QFile &file) noexcept
{
    bool ok = d->igcParser.parse(file);
    if (ok) {
        // Now "upsert" the position data, taking duplicate timestamps into account
        Flight &flight = Logbook::getInstance().getCurrentFlight();
        const Aircraft &aircraft = flight.getUserAircraft();
        Position &position = aircraft.getPosition();
        for (const IGCParser::Fix &fix : d->igcParser.getFixes()) {
            PositionData positionData;
            positionData.timestamp = fix.timestamp;
            positionData.latitude = fix.latitude;
            positionData.longitude = fix.longitude;
            positionData.altitude = fix.gnssAltitude;
            position.upsertLast(std::move(positionData));
        }

        FlightPlan &flightPlan = aircraft.getFlightPlan();
        if (position.count() > 0) {
            Analytics analytics(aircraft);
            const QDateTime startDateTimeUtc = d->igcParser.getHeader().flightDateTimeUtc;
            // Typically the takeoff and landing location are repeated in the IGC task list,
            // e.g. the takeoff airport and the actual takeoff point; those points can be
            // identical. So to ensure that each waypoint gets assigned a unique timestamp
            // we increment a given timestamp for as long as it does not exist in the
            // 'timestamps' set. Also note that while the aircraft is expected to reach
            // the waypoints in order of the task list that is actually not guaranteed;
            // depending on how much fun the pilot had in the cockpit ;)
            std::unordered_set<qint64> timestamps;
            for (const IGCParser::TaskItem &item : d->igcParser.getTask().tasks) {
                Waypoint waypoint;
                waypoint.latitude = item.latitude;
                waypoint.longitude = item.longitude;
                waypoint.identifier = item.description;
                const PositionData &closestPositionData = analytics.closestPosition(waypoint.latitude, waypoint.longitude);
                double uniqueTimestamp = closestPositionData.timestamp;
                while (timestamps.find(uniqueTimestamp) != timestamps.end()) {
                    ++uniqueTimestamp;
                }
                waypoint.timestamp = uniqueTimestamp;
                timestamps.insert(uniqueTimestamp);
                waypoint.altitude = closestPositionData.altitude;
                const QDateTime dateTimeUtc = startDateTimeUtc.addMSecs(closestPositionData.timestamp);
                waypoint.localTime = dateTimeUtc.toLocalTime();
                waypoint.zuluTime = dateTimeUtc;

                flightPlan.add(std::move(waypoint));
            }
        } else {
            // No positions - use timestamps 0, 1, 2, ...
            qint64 currentWaypointTimestamp = 0;
            for (const IGCParser::TaskItem &item : d->igcParser.getTask().tasks) {
                Waypoint waypoint;
                waypoint.latitude = item.latitude;
                waypoint.longitude = item.longitude;
                waypoint.identifier = item.description;
                waypoint.timestamp = currentWaypointTimestamp;
                ++currentWaypointTimestamp;
                flightPlan.add(std::move(waypoint));
            }
        }
    }
    return ok;
}

QDateTime IGCImportPlugin::getStartDateTimeUtc() noexcept
{
    return d->igcParser.getHeader().flightDateTimeUtc;
}

void IGCImportPlugin::updateExtendedAircraftInfo(AircraftInfo &aircraftInfo) noexcept
{
    const IGCParser::Header &header = d->igcParser.getHeader();
    aircraftInfo.tailNumber = header.gliderId;
    aircraftInfo.flightNumber = header.flightNumber;
}

void IGCImportPlugin::updateFlight(const QFile &file) noexcept
{
    updateFlightInfo(file);
    updateFlightCondition();
}

// PRIVATE


void IGCImportPlugin::updateFlightInfo(const QFile &file) noexcept
{
    const IGCParser::Header &header = d->igcParser.getHeader();
    Flight &flight = Logbook::getInstance().getCurrentFlight();

    const QString title = header.gliderType;
    flight.setTitle(title);

    Unit unit;
    const QString description = tr("Glider type:") % " " % header.gliderType % "\n" %
                                tr("Pilot:") % " " % header.pilotName % "\n" %
                                tr("Co-Pilot:") % " " % header.coPilotName % "\n" %
                                tr("Flight date:") % " " % unit.formatDateTime(header.flightDateTimeUtc) % "\n\n" %
                                tr("Aircraft imported on %1 from file: %2").arg(unit.formatDateTime(QDateTime::currentDateTime()), file.fileName());
    flight.setDescription(description);
    flight.setCreationDate(QFileInfo(file).birthTime());
}

void IGCImportPlugin::updateFlightCondition() noexcept
{
    const IGCParser::Header &header = d->igcParser.getHeader();
    Flight &flight = Logbook::getInstance().getCurrentFlight();

    FlightCondition flightCondition;
    flightCondition.startLocalTime = header.flightDateTimeUtc.toLocalTime();
    flightCondition.startZuluTime = header.flightDateTimeUtc;
    flightCondition.endLocalTime = header.flightEndDateTimeUtc.toLocalTime();
    flightCondition.endZuluTime = header.flightEndDateTimeUtc;

    flight.setFlightCondition(flightCondition);
}

